/*
 * buffer.h
 *
 *  Created on: Dec 8, 2013
 *      Author: vicente
 */

#ifndef BUFFER_H_
#define BUFFER_H_

using namespace EPOS;

class Buffer {

public:

	Buffer(int espaco_maximo) {
		vazio = new Semaphore(espaco_maximo);
		cheio = new Semaphore(0);
		buffer = new Semaphore(1);
		_espaco_maximo = espaco_maximo;
	}

	void produz(char* elemento) {
		vazio->p();
		buffer->p();

		Simple_List<char*>::Element * e = new Simple_List<char*>::Element(
				elemento);
		lista.insert_tail(e);

		buffer->v();
		cheio->v();
	}

	char* consome() {
		cheio->p();
		buffer->p();

		char* aux = lista.remove_head()->object();

		buffer->v();
		vazio->v();

		return aux;
	}

	char* get_head(){
		buffer->p();
		char* aux = lista.head()->object();
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

	Simple_List<char*> lista;

};

#endif /* BUFFER_H_ */
