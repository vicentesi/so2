/*
 * ip_datagram_receiver.h
 *
 *  Created on: Nov 24, 2013
 *      Author: vicente
 */

#ifndef IP_DATAGRAM_RECEIVER_H_
#define IP_DATAGRAM_RECEIVER_H_

// TEMPO MAXIMO DE ESPERA
#define WAIT_TIME 1000 // chronometro eh em microssegundos = 1 millis

#include "chronometer.h"
#include "ip_datagram.h"

using namespace EPOS;

class IP_Datagram_Receiver {

public:

	// Enumeracao para status dos frames
	enum StatusFrameReceiver
	{
		INCOMPLETO, COMPLETO, TEMPO_ESGOTADO
	};

	IP_Datagram_Receiver()
	{
		_somaTotal = 0;
		_chronometer.start();
		_tempoInicio = _chronometer.read();
		_chegouUltimoFrame = false;
		_datagramaCompleto = false;
	}

	~IP_Datagram_Receiver()
	{
		_chronometer.stop();
	}

	StatusFrameReceiver addFrame(IP_Datagram * frame)
	{
		_tempoFinal = _chronometer.read();

		if (_tempoFinal - _tempoInicio > WAIT_TIME)
			return TEMPO_ESGOTADO;
		else
			_tempoInicio = WAIT_TIME;

		_somaTotal = _somaTotal - frame->_datagram._header->_total_length + 20; // tira 20 do cabecalho

		Simple_List<IP_Datagram>::Element* element =
				new Simple_List<IP_Datagram>::Element(frame);
		_frames.insert(element);

		// LAST FRAGMENT
		if (frame->_datagram._header->_flags == false)
			chegouUltimoFrame(frame);

		if (_chegouUltimoFrame) {
			if (_somaTotal == 0) {
				_datagramaCompleto = true;
				return COMPLETO;
			}
		}
		return INCOMPLETO;
	}

	char* getDatagramaDataCompleto()
	{
		if (!_datagramaCompleto)
			return 0;
		return montarDatagramaCompleto();
	}

private:

	// METODO QUE VERIFICA SE CHEGOU O ULTIMO FRAGMENTO
	void chegouUltimoFrame(IP_Datagram * frame)
	{
		_tamanhoTotal = (frame->_datagram._header->_frag_offset * 8) // numero se sequencia (offset)
				+ frame->_datagram._header->_total_length - 20;
		_somaTotal += _tamanhoTotal;
		_chegouUltimoFrame = true;
	}

	char* montarDatagramaCompleto()
	{
		char* datagramaTotalData = new char[_tamanhoTotal];

		for (Simple_List<IP_Datagram>::Iterator i = _frames.begin();
				i != _frames.end(); i++)
		{
			Simple_List<IP_Datagram>::Element* datagrama =
					_frames.remove_head();

			// REVER ESSE TRECHO DE CODIGO, VER COMO MEMSET E MEMCOPY!!!
			/*System.arraycopy(datagrama._datagram._data, 0, datagramaTotalData,
			 datagrama._datagram._header._frag_offset * 8,
			 datagrama._datagram._header._total_length - 20);*/

		}

		return datagramaTotalData;
	}

private:

	OStream _cout;

	int _somaTotal;
	int _tamanhoTotal;
	long _tempoInicio;
	long _tempoFinal;
	bool _chegouUltimoFrame;
	bool _datagramaCompleto;

	Chronometer _chronometer;

	Simple_List<IP_Datagram> _frames;
};

#endif /* IP_DATAGRAM_RECEIVER_H_ */
