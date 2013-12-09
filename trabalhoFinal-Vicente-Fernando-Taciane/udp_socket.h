#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_

#include "socket.h"
#include "ip.h"
#include "udp_datagram.h"
#include "utility/ostream.h"
#include "utility/list.h"
#include "buffer.h"

using namespace EPOS;

class UDP_Socket: public Socket {

public:

	typedef unsigned char* IP_Address;

	UDP_Socket(IP * ip, const IP_Address & source, const IP_Address & dest,
			unsigned int udp_length, unsigned int size,
			unsigned int source_port, unsigned int destination_port) {
		_tx_app_udp = new Buffer(5000);
		_rx_udp_app = new Buffer(5000);
		_size = size;
		_porta_dst = destination_port;
		_porta_src = source_port;
		_ip_dest = dest;
		_udp_datagrama = new UDP_Datagram(source, dest, udp_length, size,
				source_port, destination_port);
	}

	//CRIAR DATAGRAMA UDP e enviar através da camada IP
	void send(char* data) {
		_udp_datagrama.prepare_datagram(_size, data);
		_ipV4->send(_ip_dest, _udp_datagrama.get_datagram(), _size);
		_udp_datagrama.free_datagram();
	}

	//Checar checksum e passar apenas os dados para aplicação
	void receive_ipdata(char* data) {

		UDP_Datagram datagram = getUDPdatagram(data);

		// Se os checksums forem iguais, o pacote foi transmitido corretamente
		// e pode ser entregue à aplicação
		if (checksum(datagram.get_length(), datagram.get_src_address(),
				datagram.get_dest_address(), datagram.get_padding(),
				datagram.get_buffer()) == datagram.get_checksum()) {
			// Buffer recebe os dados
			_rx_udp_app->produz(datagram.get_data());
		}
	}

	// Aplicacao acessa os dados (sem header)
	char* receive() {
		return _rx_udp_app->consome();
	}

	unsigned char* get_ip_destino() {
		return _ip_dest;
	}

	unsigned int get_porta_destino() {
		return _porta_dst;
	}

	unsigned int get_porta_origem() {
		return _porta_src;
	}

private:

	UDP_Datagram* getUDPdatagram(char* data) {
		return new UDP_Datagram();
	}

	unsigned short checksum(unsigned short int length_udp,
			unsigned short int * src_addr, unsigned short int * dest_addr,
			bool padding, unsigned short int * buffer) {
		unsigned short prot_udp = 17;
		unsigned short padd = 0;
		unsigned short w_16;
		unsigned long sum;

		//Se o tamanho dos dados for ímpar, deve-se acrescentar um octeto contendo zero.
		if (padding & 1 == 1) {
			padd = 1;
			buffer[length_udp] = 0;
		}

		sum = 0; //soma começa em zero

		//forma grupos de 16 bits ;junta os grupo de 8 bits consecutivos
		for (int i = 0; i < length_udp + padd; i = i + 2) {
			w_16 = ((buffer[i] << 8) & 0xFF00) + (buffer[i + 1] & 0xFF); //<<<<<<<<<<
			sum = sum + (unsigned long) w_16;
		}

		//adiciona as informações do pseudo cabeçalho que contém os endereços fonte e destino
		for (int i = 0; i < 4; i = i + 2) {
			w_16 = ((src_addr[i] << 8) & 0xFF00) + (src_addr[i + 1] & 0xFF);
			sum = sum + w_16;
		}

		for (int i = 0; i < 4; i = i + 2) {
			w_16 = ((dest_addr[i] << 8) & 0xFF00) + (dest_addr[i + 1] & 0xFF);
			sum = sum + w_16;
		}

		//adiciona o protocolo e o tamanho
		sum = sum + prot_udp + length_udp;

		//mantem os últimos 16 bits e adiciona os carries
		while (sum >> 16)
			sum = (sum & 0xFFFF) + (sum >> 16);

		// Complementa de 1
		sum = ~sum;

		return ((unsigned short) sum);
	}

private:

	UDP_Datagram _udp_datagrama;
	OStream _cout;
	unsigned int _size;
	IP * _ipV4;
	Buffer* _tx_app_udp;
	Buffer* _rx_udp_app;

	IP_Address _ip_dest;
	unsigned int _porta_dst;
	unsigned int _porta_src;
};

#endif /* UDP_SOCKET_H_ */

