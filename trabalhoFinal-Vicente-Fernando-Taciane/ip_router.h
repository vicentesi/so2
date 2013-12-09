/*
 * ip_router.h
 *
 *  Created on: Nov 7, 2013
 *      Author: vicente
 */

#ifndef IP_ROUTER_H_
#define IP_ROUTER_H_

#include "arp.h"
#include "nic.h"
#include "utility/ostream.h"

#define LENGTH 10

using namespace EPOS;

class IP_Router {

public:

	typedef NIC_Common::Address<6> MAC_Address;
	typedef unsigned char* IP_Address;

	typedef struct Route_Element {
		IP_Address _destination;
		IP_Address _netmask;
		IP_Address _gateway;
		Route_Element next_hop;
	} element;

	IP_Router() {
		_table_index = 0;

		/* Notas de aula 25-11, entrada estatica - bootp - todos os bits em 1 DHCP */

		// fake MAC
		unsigned char fake_dhcpIP[4] = { 255, 255, 255, 255 };
		unsigned char fake_dhcpMASK[4] = { 255, 255, 255, 255 };
		unsigned char fake_dhcpGTWay[4] = { 255, 255, 255, 255 };

		Route_Element* fake_dhcp_elem;
		fake_dhcp_elem->_destination = fake_dhcpIP;
		fake_dhcp_elem->_gateway = fake_dhcpMASK;
		fake_dhcp_elem->_netmask = fake_dhcpGTWay;
		insert_route(fake_dhcp_elem);

		/* *********************************************************************** */

		unsigned char ip_dst[4] = { 192, 168, 0, 3 };
		unsigned char mask_dst[4] = { 255, 255, 255, 0 };
		unsigned char gtway_dst[4] = { 192, 168, 0, 1 };

		Route_Element* elem_dest;
		elem_dest->_destination = ip_dst;
		elem_dest->_gateway = mask_dst;
		elem_dest->_netmask = gtway_dst;

		// Insere esta entrada estatica do endereco de destino na tabela de roteamento
		insert_route(elem_dest);
	}

	// PROCURA NA TABELA DE ROTAS PELO ENDERECO PASSADO CASO
	// EXISTA UMA ROTA PARA ESTE DESTINO RETORNA GATEWAY.
	IP_Address resolve(IP_Address dest_ip, IP_Address src_ip,
			IP_Address src_mask, IP_Address src_gtway) {

		/* Aproveita e insere na tabela de roteamento uma entrada com as
		 * informacoes do requisitante (source)*/
		insert_route(src_ip, src_mask, src_gtway);

		// Aplica mascara para verificar se estao na mesma rede
		IP_Address _my_netw = apply_mask(src_ip, src_mask);
		IP_Address _dst_netw = apply_mask(dest_ip, src_mask);

		/* Se estao na mesma rede, adiciona uma entrada na tabela com o gateway
		 * do source para o dest e retorna diretatemente o gateway do source
		 * */
		if (_my_netw == _dst_netw) {
			insert_route(dest_ip, src_mask, src_gtway);
			return src_gtway;
		}

		int index = 0;
		for (int i = 0; i < LENGTH; i++) {
			if ((_routing_table[i]->_destination[0] == dest_ip[0])
					&& (_routing_table[i]->_destination[1] == dest_ip[1])
					&& (_routing_table[i]->_destination[2] == dest_ip[2])
					&& (_routing_table[i]->_destination[3] == dest_ip[3])) {
				index = i;
			}
		}

		return _routing_table[index]->_gateway;
	}

	// INSERE UMA ROTA NA TABELA PASSANDO UMA STRUCT
	void insert_route(element* elem) {
		_routing_table[_table_index] = elem;
		_table_index += 1;
	}

	// INSERE UMA ROTA NA TABELA PASSANDO PARAMETROS (IP,MAC,GTWAY)
	void insert_route(IP_Address dest, IP_Address mask, IP_Address gtway) {
		Route_Element* elemento;
		elemento->_destination = dest;
		elemento->_netmask = mask;
		elemento->_gateway = gtway;

		insert_route(elemento);
	}

	/* APLICA A MASCARA SOBRE O IP DE DESTINO PASSADO
	 * PARA VERIFICAR SE PERTENCEM A MESMA REDE, retorna a rede */
	IP_Address apply_mask(IP_Address dest, IP_Address mask) {
		IP_Address subnet;
		int i = 0;
		while (i < 4) {
			subnet[i] = dest[i] & mask[i];
			i += 1;
		}

		return subnet;
	}

private:

	OStream _cout;
	int _table_index;

	ARP _arp_table;
	element* _routing_table[LENGTH];

};

#endif /* IP_ROUTER_H_ */
