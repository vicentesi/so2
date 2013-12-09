/*
 * arp_datagram.h
 *
 *  Created on: Nov 12, 2013
 *      Author: vicente
 */

#ifndef ARP_DATAGRAM_H_
#define ARP_DATAGRAM_H_

using namespace EPOS;

class ARP_Datagram {

public:

	typedef unsigned char* IP_Address;
	typedef NIC_Common::Address<6> MAC_Address;
	typedef unsigned short Protocol;

	/* Construtor do FRAME Eth ARP */
	ARP_Datagram(const IP_Address & ip_src, const IP_Address & ip_dst,
			const MAC_Address & mac_src, const Protocol & protocol) {

		ARP_PACKET _frame;

		_arp_packet._hw_type = 1;
		_arp_packet._protocol_type = protocol;
		_arp_packet._hw_addr_length = 6;
		_arp_packet._proto_addr_length = 4;
		_arp_packet._opcode = 0;
		_arp_packet._src_hw_address = mac_src;
		_arp_packet._src_proto_address = ip_src;
		_arp_packet._dst_hw_address = 0;
		_arp_packet._dst_ip_address = ip_dst;

		frame = new char[28];
	}

	// Aloca memoria para o frame
	void malloc_frame(unsigned int frame_size) {
		frame = (char*) malloc(frame_size * sizeof(char));
	}

	/* Libera memoria do frame */
	void free_frame() {
		free(frame);
	}

	void set_opcode(unsigned short op) {
		/*
		 * 1 = request
		 * 2 = reply
		 * */
		_arp_packet._opcode = op;
	}

	/* Prepara o pacote */
	void prepare_frame(char* mac_dst)
	{
		malloc_frame(28);
		set_mac_dst(mac_dst);
	}

	/* Coloca os dados no datagrama */
	void set_mac_dst(char* mac_dst)
	{
//		memcpy(_arp_packet._dst_hw_address, mac_dst, 48);
	}

	/* Retorna datagram completo (header+dados)*/
	char* get_frame()
	{
		memcpy(frame, &_arp_packet, 28);
		return frame;
	}

private:

	char* frame;

	/* *********** DATAGRAM *********** */

	struct ARP_PACKET {
		unsigned short _hw_type :16;
		unsigned short _protocol_type :16;
		unsigned char _hw_addr_length :8;
		unsigned char _proto_addr_length :8;
		unsigned short _opcode :16;
		MAC_Address _src_hw_address;
		IP_Address _src_proto_address;
		MAC_Address _dst_hw_address;
		IP_Address _dst_ip_address;
	} _arp_packet;

};

#endif /* ARP_DATAGRAM_H_ */
