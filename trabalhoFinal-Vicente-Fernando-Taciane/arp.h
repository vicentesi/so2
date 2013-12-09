/*
 * arp.h
 *
 *  Created on: Nov 7, 2013
 *      Author: vicente
 */

#ifndef ARP_H_
#define ARP_H_

#include "nic.h"
#include "utility/ostream.h"

#define LENGTH 10

using namespace EPOS;

class ARP {

public:

	typedef NIC_Common::Address<6> MAC_addr;
	typedef unsigned char* IP_Address;

	// ESTRUTURA QUE DEFINE ELEMENTO DA TABELA ARP
	typedef struct Table_Element {
		MAC_addr _mac;
		IP_Address _ip;
	} element;

	ARP() {
		_table_index = 0;

		/* Notas de aula 25-11, entrada estatica - bootp - todos os bits em 1 DHCP */

		// fake MAC
		char fake_dhcpMAC[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
		// fake IP
		unsigned char fake_dhcpIP[4] = { 255, 255, 255, 255 };

		MAC_addr fake_dhcpMAC_epos(fake_dhcpMAC[0], fake_dhcpMAC[1],
				fake_dhcpMAC[2], fake_dhcpMAC[3], fake_dhcpMAC[4],
				fake_dhcpMAC[5]);

		// fake for DHCP all bits -> 1
		insert_element(fake_dhcpMAC_epos, fake_dhcpIP);

		/* *********************************************************************** */

		char mac_gateway[6] = { 86, 52, 18, 0, 84, 82 };
		unsigned char ip_gateway[4] = { 192, 168, 0, 1 };

		MAC_addr dst_mac(mac_gateway[0], mac_gateway[1], mac_gateway[2],
				mac_gateway[3], mac_gateway[4], mac_gateway[5]);

		insert_element(dst_mac, ip_gateway);
	}

	// INSERE ELEMENTO NA TABELA <MAC,IP>
	void insert_element(MAC_addr mac_addr, IP_Address ip_addr) {

		/* procura na tabela a existencia de elemento com este ip dest*/
		bool exists = search(ip_addr);

		/* se nao existir, adiciona na tabela */
		if (!exists) {
			element n_elem;
			n_elem._ip = ip_addr;
			n_elem._mac = mac_addr;

			_arp_table[_table_index] = n_elem;

			_table_index += 1;
		}
	}

	// BUSCA ELEMENTO DA TABELA (STRUCT), BUSCA COM IP
	// SE EXISTIR COLOCA NA VARIAVEL <_arp_element>
	bool search(IP_Address ip) {
		bool exists = false;

		/* verifica se ja existe entrada com este ip dest */
		for (int i = 0; i < _table_index; ++i) {
			if ((_arp_table[i]._ip[0] == ip[0])
					&& (_arp_table[i]._ip[1] == ip[1])
					&& (_arp_table[i]._ip[2] == ip[2])
					&& (_arp_table[i]._ip[3] == ip[3])) {
				_arp_element = _arp_table[i];
				exists = true;
			}
		}
		return exists;
	}

	element get_element() {
		return _arp_element;
	}

private:

	OStream _cout;
	int _table_index;

	element _arp_element;

	// TAMANHO DA TABELA DEFINIDO EM 10
	element _arp_table[LENGTH];

};

#endif /* ARP_H_ */
