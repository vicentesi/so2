/*
 * transporte.h
 *
 *  Created on: Dec 3, 2013
 *      Author: vicente
 */

#ifndef TRANSPORTE_H_
#define TRANSPORTE_H_

#include "thread.h"
#include "socket_TCP.h"
#include "udp_socket.h"
#include "ip.h"
#include <utility/hash.h>
#include "segmento_tcp.h"
#include "socket.h"

using namespace EPOS;

class Transporte {

public:

	typedef unsigned char* IP_Address;

	enum TipoSocket {
		TCP, UDP
	};

	Transporte(IP * ip) {
		_th_receiver = new Thread(&_ip_receiver());
	}

	Socket getSocket(IP_Address ipDestino, int portaRemetente, int portaDestino,
			TipoSocket tipo) {

		Socket* socket;
		if (!portaEstaSendoUsada(portaRemetente)) {

			if (tipo == TipoSocket::TCP) {
				socket = new Socket_TCP(ipDestino, portaRemetente, portaDestino,
						_ipV4);
			} else if (tipo == TipoSocket::UDP) {
				socket = new UDP_Socket(ipDestino, portaRemetente, portaDestino,
						_ipV4);
			}

			Simple_List<Socket>::Element * e = new Simple_List<Socket>::Element(
					socket);
			sockets_ativos.insert_tail(e);

			return socket;
		}

		return 0;
	}

private:

	bool portaEstaSendoUsada(int porta) {
		for (Simple_List<Socket>::Iterator i = sockets_ativos.begin();
				i != sockets_ativos.end(); i++) {
			if (sockets_ativos.head()->object()->get_porta_origem() == porta)
				return true;
		}
		return false;
	}

	int _ip_receiver(void) {
		while (true) {
			char* ipdata = _ipV4->receive();
			int porta = getPortaDestino(ipdata);

			// Pega um socket ativo da lista conforme a porta
			Socket* socket = getSocketAtivo(porta);
			// Chama um metodo que produz no buffer rx_ip_tcp ou rx_ip_udp
			socket->receive_ipdata(ipdata);
		}
		return 0;
	}

private:

	Socket* getSocketAtivo(int porta) {
		Socket* retorno;
		for (Simple_List<Socket>::Iterator i = sockets_ativos.begin();
				i != sockets_ativos.end(); i++) {
			if (sockets_ativos.head()->object()->get_porta_origem() == porta)
				return sockets_ativos.head()->object();
		}
		return 0;
	}

	int getPortaDestino(char* data) {
		return 0;
	}

	OStream _cout;
	IP * _ipV4;
	Thread * _th_receiver;

	Simple_List<Socket> sockets_ativos;

};

#endif /* TRANSPORTE_H_ */
