#include <iostream>
#include "boost/bind.hpp"
#include "udp_receiver.h"

UDP_receiver::UDP_receiver(boost::asio::io_service &service, int port, size_t size):m_service(service),m_endpoint(boost::asio::ip::address_v4::any(), port),m_socket(service) {
	boost::system::error_code ec;

	m_buffer.resize(size);
	m_socket.open(m_endpoint.protocol());
	m_socket.set_option(boost::asio::socket_base::reuse_address(true));
	m_socket.bind(m_endpoint, ec);
	m_socket.set_option(boost::asio::socket_base::reuse_address(true));

	start_receive();
}

void UDP_receiver::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
	std::cout<<"recieved\n";
	if(!error) {
		std::cout<<std::string((const char *)&m_buffer[0], bytes_transferred)<<"\n";
		start_receive();
	} else {
		std::cout<<"error\n";
	}
}

void UDP_receiver::start_receive() {
	m_socket.async_receive_from(boost::asio::buffer(m_buffer), m_endpoint, boost::bind(&UDP_receiver::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

std::string UDP_receiver::receive() {
	size_t count = m_socket.receive_from(boost::asio::buffer(m_buffer), m_endpoint);
	std::string msg = std::string((const char *)&m_buffer[0], count);
	std::cout<<msg<<"\n";
	return msg;
	//return std::stod(msg);
}
