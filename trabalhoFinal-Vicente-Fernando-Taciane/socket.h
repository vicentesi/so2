/*
 * socket.h
 *
 *  Created on: Dec 8, 2013
 *      Author: vicente
 */

#ifndef SOCKET_H_
#define SOCKET_H_

class Socket {

public:
	virtual void send(char* data) = 0;
	virtual char* receive() = 0;
	virtual void receive_ipdata(char* data) = 0;

	virtual unsigned char* get_ip_destino() = 0;
	virtual int get_porta_destino() = 0;
	virtual int get_porta_origem() = 0;
};

#endif /* SOCKET_H_ */
