/*
 * buffer_ipv4.h
 *
 *  Created on: Dec 8, 2013
 *      Author: vicente
 */

#ifndef BUFFER_IPV4_H_
#define BUFFER_IPV4_H_

#include "semaphore.h"

using namespace EPOS;

class Buffer_IPv4 {

public:

	Buffer_IPv4(int espaco_maximo) {
		vazio = new Semaphore(espaco_maximo);
		cheio = new Semaphore(0);
		buffer = new Semaphore(1);
		_espaco_maximo = espaco_maximo;
	}

	void produz(Datagram_IP* elemento) {
		vazio->p();
		buffer->p();

		Simple_List<Datagram_IP*>::Element * e = new Simple_List<Datagram_IP*>::Element(
				elemento);
		lista.insert_tail(e);

		buffer->v();
		cheio->v();
	}

	Datagram_IP* consome() {
		cheio->p();
		buffer->p();

		Datagram_IP* aux = lista.remove_head()->object();

		buffer->v();
		vazio->v();

		return aux;
	}

	Datagram_IP* get_head(){
		buffer->p();
		Datagram_IP* aux = lista.head()->object();
		buffer->v();
		return aux;
	}

	bool esta_vazio() {
		return lista.empty();
	}

	int tamanho() {
		return _espaco_maximo - lista.size();
	}

private:

	Semaphore* vazio;
	Semaphore* cheio;
	Semaphore* buffer;
	int _espaco_maximo;

	Simple_List<Datagram_IP*> lista;

};

#endif /* BUFFER_IPV4_H_ */
