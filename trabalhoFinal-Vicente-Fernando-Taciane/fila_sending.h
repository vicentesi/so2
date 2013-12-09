/*
 * fila_sending.h
 *
 *  Created on: Dec 4, 2013
 *      Author: vicente
 */

#ifndef FILA_SENDING_H_
#define FILA_SENDING_H_

#include <utility/hash.h>

using namespace EPOS;

// Estrutura que contem todos os segmentos que foram enviados mas que ainda nao receberam o ack
// A quantidade de elementos contidos ao mesmo tempo nesta fila eh menor ou igual
// ao tamanho da janela do receptor
class Fila_Sending {

public:

	Fila_Sending(int tempoMaximo, IP* ipv4, int janelaMaxima,
			IP_Address ipdst) {
		_semph_fila_sending = new Semaphore(1);
		_semph_timeout_ctrl = new Semaphore(0);
		_semph_inserir_na_lista = new Semaphore(janelaMaxima);

		_tempo_maximo_espera = tempoMaximo;
		_janela_maxima = janelaMaxima;
		_ipv4 = ipv4;
		_ip_dst = ipdst;

		_th_timeout = new Thread(&_timeout_ctrl());

		_chronometer.start();
	}

	~Fila_Sending() {
		_chronometer.stop();
	}

	void ack_receive(Segmento_TCP ack) {
		_semph_fila_sending->p();

		if(ack._numero_reconhecimento > _ultimo_nro_reconhecimento)
			_valor_janela_recepcao_destino = ack._janela_recepcao;

		for (Simple_List<Segmento_TCP>::Iterator i =
				_lista_ordenada_cronometro->begin();
				i != _lista_ordenada_cronometro->end(); i++) {
			Simple_List<Segmento_TCP>::Element* segmento =
					_lista_ordenada_cronometro->head()->object();
			Segmento_TCP* seg = _lista_ordenada_cronometro->head()->object();
			if (seg->_numero_sequencia < ack._numero_reconhecimento)
				_lista_ordenada_cronometro->remove(segmento);
		}

		if (_valor_janela_recepcao_destino - _lista_ordenada_cronometro->size()
				> 0)
			_semph_inserir_na_lista->v();

		_semph_fila_sending->v();

	}

	void insert_tail(Segmento_TCP segmento) {
		_semph_inserir_na_lista->p();
		_semph_fila_sending->p();

		segmento._valor_do_cronometro = _chronometer.read()
				+ _tempo_maximo_espera;
		_ipv4->send(segmento.get_segmento_TCP_em_formato_array_char(), _ip_dst);
		Simple_List<Segmento_TCP>::Element* e = segmento;
		_lista_ordenada_cronometro->insert_tail(e);
		_valor_janela_recepcao_destino--;

		_semph_timeout_ctrl->v();
		_semph_fila_sending->v();
	}

	void clear() {
		_semph_fila_sending->p();
		_lista_ordenada_cronometro = new Simple_List<Segmento_TCP>();
		_semph_fila_sending->v();
	}

	bool esta_vazio() {
		_semph_fila_sending->p();
		bool aux = _lista_ordenada_cronometro->empty();
		_semph_fila_sending->v();
		return aux;
	}

private:

	void _timeout_ctrl() {
		while (true) {
			_semph_timeout_ctrl->p();
			_semph_fila_sending->p();

			long tempo_espera = -1;
			long timeout_atual = _chronometer.read();

			Segmento_TCP seg = _lista_ordenada_cronometro->head()->object();
			tempo_espera = timeout_atual - seg._valor_do_cronometro;

			while (timeout_atual > seg._valor_do_cronometro) {
				_lista_ordenada_cronometro->remove_head();
				seg._valor_do_cronometro = timeout_atual + _tempo_maximo_espera;
				Simple_List<Segmento_TCP>::Element* e = seg;
				_lista_ordenada_cronometro->insert_tail(e);
				_ipv4->send(seg.get_segmento_TCP_em_formato_array_char(),
						_ip_dst);
				seg = _lista_ordenada_cronometro->head()->object();
				tempo_espera = timeout_atual - seg._valor_do_cronometro;
			}

			_semph_fila_sending->v();
			_semph_timeout_ctrl->v();

			if (tempo_espera > 0){
				Function_Handler handler_a(&func_a);
				Alarm alarm_a(tempo_espera, &handler_a, iterations);
			}
		}
	}

	void func_a(){

	}

	Thread* _th_timeout;

	Semaphore* _semph_fila_sending;
	Semaphore* _semph_timeout_ctrl;
	Semaphore* _semph_inserir_na_lista;

	Chronometer _chronometer;
	Simple_List<Segmento_TCP>* _lista_ordenada_cronometro;
	IP* _ipv4;

	int _ultimo_nro_reconhecimento;
	int _valor_janela_recepcao_destino;
	int _tempo_maximo_espera;
	int _janela_maxima;

	IP_Address _ip_dst;
};

#endif /* FILA_SENDING_H_ */
