/*
 * segmento_tcp.h
 *
 *  Created on: Dec 4, 2013
 *      Author: vicente
 */

#ifndef SEGMENTO_TCP_H_
#define SEGMENTO_TCP_H_

using namespace EPOS;

class Segmento_TCP {

public:

	Segmento_TCP();

	Segmento_TCP(char* ip_data){

	}

	static Segmento_TCP get_segmento_syn();
	static Segmento_TCP get_segmento_synack();
	static Segmento_TCP get_segmento_ack();
	static Segmento_TCP get_segmento_fin();

	bool eh_segmento_syn();
	bool eh_segmento_synack();
	bool eh_segmento_ack();
	bool eh_segmento_fin();
	bool eh_segmento_data();


	void calcula_checksum();
	bool compara_checksum();

	char* get_segmento_TCP_em_formato_array_char();

	int _porta_src;
	int _porta_dst;
	int _numero_sequencia;
	int _numero_reconhecimento;
	int _tamanho_cabecalho;
	int _nao_usado;

	bool _urg;
	bool _ack;
	bool _psh;
	bool _rst;
	bool _syn;
	bool _fin;

	int _janela_recepcao;
	char* _checksum;
	int _ptr_urgencia;
	char* _data;

	long _valor_do_cronometro;


};

#endif /* SEGMENTO_TCP_H_ */
