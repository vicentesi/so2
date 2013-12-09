/*
 * socket_TCP.h
 *
 *  Created on: Dec 3, 2013
 *      Author: vicente
 */

#ifndef SOCKET_TCP_H_
#define SOCKET_TCP_H_

#include <semaphore.h>
#include "chronometer.h"
#include "socket.h"
#include "fila_sending.h"

using namespace EPOS;

class Socket_TCP: Socket {

	typedef unsigned char* IP_Address;

	static enum Status_conexao {
		NAO_CONECTADO, CONECTANDO, CONECTADO, FINALIZANDO
	};

public:

	Socket_TCP(IP_Address ip_destino, int portaSrc, int portaDst, IP * ipv4) {
		_ipdst = ip_destino;
		_portaSrc = portaSrc;
		_portaDst = portaDst;
		_ipV4 = ipv4;
		_status_conexao = NAO_CONECTADO;
		_janela_receptor = 0;

		_th_sender = new Thread(&_sender());
		_th_receiver = new Thread(&_receiver());

		_rx_ip_tcp = new Buffer(1);
		_tx_app_tcp = new Buffer(5000);
		_rx_tcp_app = new Buffer(5000);

		_fila_sending = new Fila_Sending(_tempo_espera, _ipV4,
				_tamanho_max_janela, ip_destino);

		_esperar_conexao = new Semaphore(0);
	}

	void send(char* msg) {
		int numero_segmento = (sizeof(msg) / _tamanho_max_segmento);
		bool tem_resto = false;

		for (int i = 0; i < numero_segmento; i++) {
			int indice_inicio = i * _tamanho_max_segmento;
			int tam = indice_inicio + _tamanho_max_segmento;
			char* copy = new char[_tamanho_max_segmento];

			for (int j = 0; j < _tamanho_max_segmento; j++) {
				copy[j] = msg[indice_inicio + j];
			}
			_tx_app_tcp->produz(copy);
		}
	}

	char* receive() {
		return _rx_tcp_app->consome();
	}

	void receive_ipdata(char* data) {
		_rx_ip_tcp->produz(data);
	}

	unsigned char* get_ip_destino() {
		return _ipdst;
	}

	int get_porta_destino() {
		return _portaDst;
	}

	int get_porta_origem() {
		return _portaSrc;
	}

private:

	int _sender() {
		while (true) {

			if (_status_conexao == NAO_CONECTADO) {
				Segmento_TCP syn = Segmento_TCP::get_segmento_syn();
				_status_conexao = CONECTANDO;
				_ipV4->send(syn.get_segmento_TCP_em_formato_array_char(),
						_ipdst);

				_esperar_conexao->p();
				_esperar_conexao->v();

			} else if (_status_conexao == CONECTADO) {
				Segmento_TCP segSending = new Segmento_TCP(
						_tx_app_tcp->consome());
				segSending.calcula_checksum();
				_fila_sending->insert_tail(segSending);
			}
		}
		return 0;
	}

	int _receiver() {
		while (true) {

			Segmento_TCP segmento = new Segmento_TCP(_rx_ip_tcp->consome());

			if (segmento.eh_segmento_syn()
					&& _status_conexao == NAO_CONECTADO) {
				Segmento_TCP* seg = Segmento_TCP::get_segmento_synack();
				_status_conexao = CONECTANDO;
				_numero_sequencia_esperado = 0;
				_ipV4->send(seg->get_segmento_TCP_em_formato_array_char(),
						_ipdst);
			} else if (segmento.eh_segmento_synack()
					&& _status_conexao == CONECTANDO) {
				Segmento_TCP* ack = Segmento_TCP::get_segmento_ack();
				_status_conexao = CONECTADO;
				_rx_tcp_app = new Buffer(5000);
				_numero_sequencia_esperado = 0;
				_ipV4->send(ack->get_segmento_TCP_em_formato_array_char(),
						_ipdst);
				_esperar_conexao->v();
			} else if (segmento.eh_segmento_ack()
					&& _status_conexao == CONECTANDO) {
				_status_conexao = CONECTADO;
				_rx_tcp_app = new Buffer(5000);
				_esperar_conexao->v();
			} else if (segmento.eh_segmento_data()
					&& _status_conexao == CONECTADO) {
				validar_receive(segmento);
			} else if (segmento.eh_segmento_ack()
					&& _status_conexao == CONECTADO) {
				_fila_sending->ack_receive(segmento);

				if (_tx_app_tcp->esta_vazio() && _fila_sending->esta_vazio()) {
					_status_conexao == FINALIZANDO;
					Segmento_TCP* fin = Segmento_TCP::get_segmento_fin();
					_ipV4->send(fin->get_segmento_TCP_em_formato_array_char(),
							_ipdst);
				}
			} else if (segmento.eh_segmento_fin()
					&& _status_conexao == CONECTADO) {
				_status_conexao == FINALIZANDO;
				Segmento_TCP* ack = Segmento_TCP::get_segmento_ack();
				_ipV4->send(ack->get_segmento_TCP_em_formato_array_char(),
						_ipdst);
				Segmento_TCP* fin = Segmento_TCP::get_segmento_fin();
				_ipV4->send(fin->get_segmento_TCP_em_formato_array_char(),
						_ipdst);
			} else if (segmento.eh_segmento_ack()
					&& _status_conexao == FINALIZANDO) {
				_status_conexao = NAO_CONECTADO;
				_esperar_conexao->p();
			}
		}
		return 0;
	}

	// Armazena em uma estrutura todos os segmentos que chegaram adiantados
	// Quando chegar o segmento esperado, entao move o segmento para o buffer
	// de aplicacao, assim como todos os segmentos adjacentes a ele.
	void validar_receive(Segmento_TCP segmento) {
		if (!segmento.compara_checksum())
			return;

		int numero_sequencia = segmento._numero_sequencia;

		if (numero_sequencia == _numero_sequencia_esperado) {
			_rx_tcp_app->produz(segmento._data);
			_numero_sequencia_esperado = numero_sequencia
					+ sizeof(segmento._data);
			bool parar = false;

			while (!parar) {
				Segmento_TCP* seg = fila_segmentos_adiantados.head();

				if (seg->_numero_sequencia == _numero_sequencia_esperado) {
					fila_segmentos_adiantados.remove_head();
					_rx_tcp_app->produz(seg->_data);
					_numero_sequencia_esperado = seg->_numero_sequencia
							+ sizeof(seg->_data);
				} else {
					parar = true;
				}
			}

			Segmento_TCP* seg = Segmento_TCP::get_segmento_ack();
			seg->_numero_reconhecimento = _numero_sequencia_esperado;
			seg->_janela_recepcao = _rx_tcp_app->tamanho();
			_ipV4->send(seg->get_segmento_TCP_em_formato_array_char(), _ipdst);

		} else {
			colocar_segmento_adiantado_na_lista(segmento);
		}
	}

	void colocar_segmento_adiantado_na_lista(Segmento_TCP segmento);

private:

	IP * _ipV4;
	Status_conexao _status_conexao;

	Simple_List<Segmento_TCP> fila_segmentos_adiantados;
	Fila_Sending* _fila_sending;

	int _numero_sequencia_esperado;

	const int _tamanho_max_segmento = 9000;
	const int _tempo_espera = 10000;
	const int _tamanho_max_janela = 100000;

	long _janela_receptor;

	IP_Address _ipdst;
	int _portaSrc;
	int _portaDst;

	Semaphore* _esperar_conexao;

	Buffer* _rx_ip_tcp;
	Buffer* _rx_tcp_app;
	Buffer* _tx_app_tcp;

	Thread * _th_sender;
	Thread * _th_receiver;
};

#endif /* SOCKET_TCP_H_ */
