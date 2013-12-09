#ifndef UDP_DATAGRAM_H_
#define UDP_DATAGRAM_H_

using namespace EPOS;

class UDP_Datagram {

public:

	typedef unsigned char* IP_Address;

	UDP_Datagram(){}

	UDP_Datagram(const IP_Address & source, const IP_Address & dest,
			unsigned short udp_length, unsigned int size,
			unsigned char source_port, unsigned char destination_port)
	{
		DATAGRAM _datagram;
		_datagram._header->_source_port = source_port;
		_datagram._header->_destination_port = destination_port;
		_datagram._header->_length = CPU::htons(udp_length);
		_datagram._header->_checksum = 0;

		_src_address = source;
		_dst_address = dest;
		_udp_length = udp_length;
		_size = size;

		_datagram._data = new char[size]; //size = tamanho dos dados
	}

	/* Aloca memoria para o datagrama
	 * com o tamanho de acordo com o MTU da rede
	 * */
	void malloc_datagram(unsigned int mtu_size)
	{
		datagram = (char*) malloc(mtu_size * sizeof(char));
	}

	/* Libera memoria do datagrama */
	void free_datagram()
	{
		free(datagram);
	}

	/* Prepara o pacote e os dados */
	void prepare_datagram(unsigned int mtu_size, char* data)
	{
		malloc_datagram(mtu_size);
		set_data(data);
		set_checksum( get_length(), get_src_address(), get_dest_address(), get_padding(), get_buffer());
	}

	/* Coloca os dados no datagrama */
	void set_data(char* data)
	{
		memcpy(_datagram._data, data, _size);
	}

	/* Calculo do checksum */
	void set_checksum(unsigned short int length_udp, unsigned short int * src_addr, unsigned short int * dest_addr, bool padding, unsigned short int * buffer)
	{
		unsigned short prot_udp = 17;
		unsigned short padd = 0;
		unsigned short w_16;
		unsigned long sum;

		//Se o tamanho dos dados for ímpar, deve-se acrescentar um octeto contendo zero.
		if (padding&1 == 1){
			padd=1;
			buffer[length_udp] = 0;
		}

		sum = 0; //soma começa em zero

		//forma grupos de 16 bits ;junta os grupo de 8 bits consecutivos
		for (int i = 0; i < length_udp + padd; i = i+2){
			w_16 =((buffer[i]<<8)&0xFF00)+(buffer[i+1]&0xFF);
			sum = sum + (unsigned long)w_16;
		}

		//adiciona as informações do pseudo cabeçalho que contém os endereços fonte e destino
		for (int i = 0; i < 4; i = i+2){
			w_16 =((src_addr[i]<<8)&0xFF00)+(src_addr[i+1]&0xFF);
			sum = sum + w_16;
		}

		for (int i = 0; i < 4; i = i+2){
			w_16 =((dest_addr[i]<<8)&0xFF00)+(dest_addr[i+1]&0xFF);
			sum = sum + w_16;
		}

		//adiciona o protocolo e o tamanho
		sum = sum + prot_udp + length_udp;

		//mantem os últimos 16 bits e adiciona os carries
		while (sum>>16)
			sum = (sum & 0xFFFF)+(sum >> 16);

		// Complementa de 1
		sum = ~sum;

		_datagram._header->_checksum = ((unsigned short) sum);
	}

	//Retorna o cálculo do checksum
	unsigned short get_checksum()
	{
		return _datagram._header->_checksum;
	}

	/* Retorna datagrama completo */
	char* get_datagram()
	{
		// COPIA HEADER
		memcpy(datagram, &_datagram._header, 8);
		// COPIA DADOS
		memcpy(datagram + 8, _datagram._data, _size);
		return datagram;
	}

	//Retorna apenas os dados do datagrama
	char* get_data()
	{
		// COPIA DADOS
		memcpy(datagram, _datagram._data, _size);
		return datagram;
	}

	unsigned short get_length()
	{
		return _datagram._header->_length;
	}

	unsigned short int * get_src_address()
	{
		unsigned short src_address[_src_address];
		for(int i = 0; i < _src_address; i ++)
		{
			src_address[i] = _src_address[i];
		}
		return src_address;
	}

	unsigned short int * get_dest_address()
	{
		unsigned short dst_address[_dst_address];
		for(int i = 0; i < _dst_address; i ++)
		{
			dst_address[i] = _dst_address[i];
		}
		return dst_address;
	}

	bool get_padding()
	{
		bool padding = false;
		if ( ((_size/8)%2) != 0)//se o número de octetos da parte de dados for ímpar
			padding = true;
		return padding;
	}

	unsigned short int * get_buffer()
	{
		unsigned short buff[_datagram._header->_length];
		for(int i = 0; i < _datagram._header->_length; i ++)
		{
			buff[i] = _datagram._header->_length[i];
		}
		return buff;
	}

private: //informações do pseudoheader para uso no checksum

	IP_Address _src_address;
	IP_Address _dst_address;
	unsigned short _udp_length;
	unsigned int _size; //guarda o tamanho dos dados > sem cabeçalho

public: //datagrama com cabeçalho e dados

	char* datagram;

	struct DATAGRAM
	{
		struct HEADER
		{
			unsigned short _source_port :16;
			unsigned short _destination_port :16;
			unsigned short _checksum:16;
			unsigned short _length :16; //Comprimento do datagrama (Cabeçalho UDP + Dados)
		} *_header;
		char* _data;
	} _datagram;
}__attribute__((packed, may_alias));

#endif /* UDP_DATAGRAM_H_ */

