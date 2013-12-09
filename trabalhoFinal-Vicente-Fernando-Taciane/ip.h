#ifndef __ip_h
#define __ip_h

#include "ethernet.h"
#include "mach/pc/config.h"
#include "nic.h"
#include "utility/ostream.h"
#include "ip_datagram.h"
#include "arp.h"
#include "ip_router.h"
#include "arp_datagram.h"
#include "ip_datagram_receiver.h"
#include "utility/hash.h"
#include <semaphore.h>

#define HEADER_SIZE 20
#define MTU_SIZE 1480
#define PROTOCOL_IP 0x0800
#define PROTOCOL_ARP 0x0806
#define MF_FLAG 1
#define LF_FLAG 0

using namespace EPOS;

class IP {

public:

	NIC nic;

	typedef NIC_Common::Address<6> MAC_Address;
	typedef unsigned char* IP_Address;
	typedef unsigned short Protocol;

	IP() {

		_offset = 0;
		_packetID = 0;

		unsigned char self_ip[4] = { 192, 168, 0, 2 };
		unsigned char self_mask[4] = { 255, 255, 255, 0 };
		unsigned char self_gtway[4] = { 192, 168, 0, 1 };

		_my_IP = self_ip;
		_my_mask = self_mask;
		_my_gateway = self_gtway;
		_my_mac = nic.address();

		_th_sender = new Thread(&_sender());
		_th_receiver = new Thread(&_receiver());

	}

	void send(char* data, IP_Address ip) {
		Datagram_IP* elemento = new Datagram_IP();
		elemento->datagram = data;
		elemento->ip = ip;

		_tx_transp_ip->produz(elemento);
	}

	char* receive() {
		return _rx_ip_transp->consome();
	}

private:

	// pacote protocolo IP 0x0800
	void ipv4_receive(IP_Datagram* frame) {
		if (estaContidoNoConjuntoPerdidos(
				frame->_datagram._header->_identification))
			return;

		if (!estaContidoNoBufferDatagramReceive(
				frame->_datagram._header->_identification)) {
			IP_Datagram_Receiver aux = new IP_Datagram_Receiver();
			Simple_List<IP_Datagram_Receiver>::Element * e = new Simple_List<
					IP_Datagram_Receiver>::Element(aux);
			bufferDatagramasReceive.insert_tail(e);
		}

		IP_Datagram_Receiver datagrama_receive = getIpDatagramReceive(
				frame->_datagram._header->_identification);
		IP_Datagram_Receiver::StatusFrameReceiver status =
				datagrama_receive.addFrame(frame);

		if (status == IP_Datagram_Receiver::TEMPO_ESGOTADO) {
			Simple_List<unsigned short>::Element * e = new Simple_List<
					unsigned short>::Element(
					frame->_datagram._header->_identification);
			conjuntoDatagramasPerdidos.insert_tail(e);
		} else if (status == IP_Datagram_Receiver::COMPLETO) {
			char* datagramaCompleto =
					datagrama_receive.getDatagramaDataCompleto();
			_rx_ip_transp->produz(datagramaCompleto);
		}
	}

	// ARP REQUEST
	// pacote protocolo ARP 0x0806
	void arp_request(IP_Datagram* frame) {
		if (frame->_datagram._header->_dst_address == _my_IP) {
			ARP_Datagram* arp_req = reinterpret_cast<ARP_Datagram>(frame);

			MAC_Address mac_src = arp_req->_arp_packet._src_hw_address;
			arp_req->_arp_packet._dst_hw_address = _my_mac;

			// MANDA UNICAST PARA O REMETENTE COM SELF MAC INSERIDO NO PACOTE
			nic.send(mac_src, 0x0807, arp_req->frame, 28);
		}
	}

	// ARP REPLY
	// pacote protocolo ARP 0x0807
	void arp_reply(IP_Datagram* frame) {
		ARP_Datagram* arp_req = reinterpret_cast<ARP_Datagram>(frame);

		// INSERE O REGISTRO NA TABELA ARP COM O MAC RECEBIDO NO REPLY
		_arp_table.insert_element(arp_req->_arp_packet._dst_hw_address,
				arp_req->_arp_packet._dst_ip_address);

	}

	int _sender() {
		while (true) {

			Datagram_IP datagrama_ip = _tx_transp_ip->consome();

			_offset = 0;

			// CONSULTA TABELA DE ROTEAMENTO
			IP_Address ip_gateway = _router.resolve(datagrama_ip.ip, _my_IP,
					_my_mask, _my_gateway);

			// CONSULTA TABELA ARP PROCURANDO MAC DEST
			MAC_Address dst_mac;
			if (_arp_table.search(ip_gateway)) {
				dst_mac = _arp_table.get_element()._mac;
			} else {
				// Criar o datagrama ARP
				ARP_Datagram _arp_frame(_my_IP, ip_gateway, nic.address(),
				PROTOCOL_ARP);

				_frame_arp = _arp_frame.get_frame();

				// Envia um ARP REQUEST
				nic.send(NIC::BROADCAST, PROTOCOL_ARP, _frame_arp, nic.mtu());

				// Verifica se o mac de dest ja esta na tabela
				// atraves do ARP REPLY do receiver
				for (int i = 0; i < 5; i++) {
					if (_arp_table.search(ip_gateway))
						dst_mac = _arp_table.get_element()._mac;
				}
			}

			// FRAGMENTACAO E ENVIO
			if (nic.mtu() > (nic.mtu() - HEADER_SIZE)) {
				// CALCULO DO NUMERO DE FRAGMENTOS A SER ENVIADOS
				int num_frag = (int) (nic.mtu() / MTU_SIZE);
				if (nic.mtu() % (nic.mtu() - HEADER_SIZE) > 0)
					num_frag += 1;

				// envio de fragmentos
				for (int i = 0; i < num_frag - 1; i++) {
					/* PREPARA DATAGRAMA */
					// CRIACAO DO DATAGRAMA
					IP_Datagram * header = new IP_Datagram(_my_IP, dst,
					PROTOCOL_IP, nic.mtu());
					header->prepare_datagram(nic.mtu(), MF_FLAG, _packetID,
							_offset, data);
					_datagram_ip = header->get_datagram();

					/* ENVIA PACOTE (HEADER + DATA) */
					nic.send(dst_mac, PROTOCOL_IP, _datagram_ip, nic.mtu());

					/* INCREMENTA OFFSET PARA PROXIMO FRAGMENTO
					 * LIBERA MEMORIA DESTE FRAGMENTO*/
					_offset += 1500;
					free(header);
				}

				// envio do ultimo fragmento
				// CRIACAO DO DATAGRAMA
				IP_Datagram * header = new IP_Datagram(_my_IP, dst, PROTOCOL_IP,
						nic.mtu());
				header->prepare_datagram(nic.mtu(), LF_FLAG, _packetID, _offset,
						data);
				_datagram_ip = header->get_datagram();

				/* ENVIA PACOTE (HEADER + DATA) */
				nic.send(dst_mac, PROTOCOL_IP, _datagram_ip, nic.mtu());

				/* LIBERA MEMORIA DESTE FRAGMENTO*/
				free(header);

			} else {
				/* PREPARA DATAGRAMA */
				// CRIACAO DO DATAGRAMA
				IP_Datagram * header = new IP_Datagram(_my_IP, dst, PROTOCOL_IP,
						nic.mtu());
				header->prepare_datagram(nic.mtu(), LF_FLAG, _packetID, _offset,
						data);
				_datagram_ip = header->get_datagram();

				/* ENVIA PACOTE (HEADER + DATA) */
				nic.send(dst_mac, PROTOCOL_IP, _datagram_ip, nic.mtu());

				/* LIBERA MEMORIA DESTE FRAGMENTO*/
				free(header);
			}

			_packetID++;
		}
		return 0;
	}

	void _receiver() {
		while (true) {
			nic.receive(&src, &prot, data, nic.mtu());
			char* charFrame = data;
			IP_Datagram* frame = getIpDatagrama(charFrame);

			// IP
			if (frame->get_protocol() == 0x0800) {
				ipv4_receive(frame);
			} else if (frame->get_protocol() == 0x0806) { // ARP REQUEST
				arp_request(frame);
			} else if (frame->get_protocol() == 0x0807) { // ARP REPLY
				arp_reply(frame);
			}
		}
	}

	bool estaContidoNoConjuntoPerdidos(unsigned short id) {
		return false;
	}

	bool estaContidoNoBufferDatagramReceive(unsigned short id) {
		return false;
	}

	IP_Datagram_Receiver getIpDatagramReceive(unsigned short id) {
		return 0;
	}

	IP_Datagram_Receiver removeIpDatagramReceive(unsigned short id) {
		return 0;
	}

	IP_Datagram* getIpDatagrama(char* data) {
		return new IP_Datagram();
	}

private:

	OStream _cout;

	NIC::Address src, dst;
	NIC::Protocol prot;
	char data[nic.mtu()];
	static unsigned char dst_addr[4] = { 192, 168, 0, 1 };

	ARP _arp_table;
	IP_Router _router;

	IP_Address _my_IP;
	IP_Address _my_mask;
	IP_Address _my_gateway;
	MAC_Address _my_mac;

	unsigned int _offset;
	unsigned short _packetID;

	char* _datagram_ip;
	char* _frame_arp;

	Simple_List<IP_Datagram_Receiver> bufferDatagramasReceive;
	Simple_List<unsigned short> conjuntoDatagramasPerdidos;

	Buffer* _rx_ip_transp;
	Buffer_IPv4* _tx_transp_ip;

	Thread * _th_sender;
	Thread * _th_receiver;

	IP_Datagram _datagrama;
}
;

//__END_SYS

#endif

