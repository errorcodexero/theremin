#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include <string>
#include <cstdint>
#include <vector>
#include "boost/asio.hpp"

class UDP_receiver {
	private:
	std::string received;
	std::vector<uint8_t> m_buffer;
	boost::asio::io_service &m_service;
	boost::asio::ip::udp::endpoint m_endpoint;
	boost::asio::ip::udp::socket m_socket;
	
	void handle_receive(const boost::system::error_code&, std::size_t);
	void start_receive();

	public:
	void receive();
	int get();

	UDP_receiver(boost::asio::io_service&,int,size_t);	
};

#endif
