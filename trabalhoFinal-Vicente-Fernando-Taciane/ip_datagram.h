#include "nic.h"

#ifndef IP_DATAGRAM_H_
#define IP_DATAGRAM_H_

using namespace EPOS;

class IP_Datagram {

public:

	typedef unsigned char* IP_Address;
	typedef unsigned short Protocol;

	/* Construtor do Datagrama */
	IP_Datagram(){}

	/* Construtor do Datagrama */
	IP_Datagram(const IP_Address & source, const IP_Address & dest,
			const Protocol & protocol, unsigned int size)
	{
		DATAGRAM _datagram;

		_datagram._header->_version = VER;
		_datagram._header->_ihl = IHL;
		_datagram._header->_tos = TOS;
		_datagram._header->_total_length = CPU::htons(size);
		_datagram._header->_identification = CPU::htons(0);
		_datagram._header->_flags = 0;
		_datagram._header->_frag_offset = 0;
		_datagram._header->_ttl = TTL;
		_datagram._header->_protocol = protocol;
		_datagram._header->_header_checksum = 0;
		_datagram._header->_src_address = source;
		_datagram._header->_dst_address = dest;
	}

	/* Prepara o pacote, define a flag, o id, offset e os dados */
	void prepare_datagram(unsigned int mtu_size, unsigned short flag, unsigned short pcktID,
			unsigned int offset, char* data)
	{
		set_flags(flag);
		set_id(pcktID);
		set_frag_offset(offset);
		set_data(data);
		set_checksum();
	}

	/* Define o identificador do pacote no header*/
	void set_id(unsigned short id)
	{
		_datagram._header->_identification = CPU::htons(id);
	}

	/* Define as flags de fragmentacao no header */
	void set_flags(unsigned short flag)
	{
		unsigned short x = CPU::htons(flag << 13 | get_frag_offset());
		_datagram._header->_frag_offset = x & 0x1fff;
		_datagram._header->_flags = x >> 13;
	}

	/* Define o offset de fragmentacao no header */
	void set_frag_offset(unsigned short offset)
	{
		unsigned short x = CPU::htons(get_flags() << 13 | offset);
		_datagram._header->_identification = x & 0x1fff;
		_datagram._header->_identification = x >> 13;
	}

	/* Coloca os dados no datagrama */
	void set_data(char* data)
	{
		_datagram._data = new char[1480];
		memcpy(_datagram._data, data, 1480);
	}

	/* Calculo do checksum */
	void set_checksum()
	{
		_datagram._header->_header_checksum = CRC::crc16(datagram, 10);
	}

	/* Retorna as flags de fragmentacao do header */
	unsigned short get_flags()
	{
		return CPU::ntohs(
				_datagram._header->_identification << 13
						| _datagram._header->_identification) >> 13;
	}

	/* Retorna o offset de fragmentacao do header */
	unsigned short get_frag_offset() const
	{
		return CPU::ntohs(
				_datagram._header->_flags << 13 | _datagram._header->_frag_offset)
				& 0x1fff;
	}

	/* Retorna datagram completo (header+dados)*/
	char* get_datagram()
	{
		// COPIA HEADER
		memcpy(datagram, _datagram._header, 20);
		// COPIA DADOS
		memcpy(datagram + 20, _datagram._data, 1480);
		// RETORNA DATAGRAMA COMPLETO
		return datagram;
	}

	unsigned char get_protocol(){
		return _datagram._header->_protocol;
	}

//private:
public:

	char datagram[1500];

	static const unsigned char VER = 4;
	static const unsigned char IHL = 4;
	static const unsigned char TOS = 0;
	static const unsigned char TTL = 64;

	/* *********** DATAGRAM *********** */

	struct DATAGRAM
	{
		struct HEADER
		{
			unsigned char _version :4;
			unsigned char _ihl :4;
			unsigned char _tos:8;
			unsigned short _total_length:16;
			unsigned short _identification:16;
			unsigned short _flags :3;
			unsigned short _frag_offset :13;
			unsigned char _ttl:8;
			unsigned char _protocol:8;
			unsigned short _header_checksum:16;
			IP_Address _src_address;
			IP_Address _dst_address;
		} *_header;
		char* _data;
	} _datagram;

}__attribute__((packed, may_alias));

#endif /* IP_DATAGRAM_H_ */

