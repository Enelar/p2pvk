#pragma once

#include <boost\asio.hpp>

class ssdp
{
  static const std::string ssdp_multicast;
  static const unsigned short ssdp_port;

  boost::asio::io_service &io;
  std::string ip;

  boost::asio::ip::udp::socket socket;
  boost::asio::ip::udp::endpoint multicast;
  boost::asio::ip::udp::endpoint BindTo(unsigned short port);
  boost::asio::ip::udp::endpoint OpenPort() noexcept;
public:
  ssdp(boost::asio::io_service &_io, std::string local_addr)
    : io(_io), ip(local_addr), socket(io), multicast(boost::asio::ip::address::from_string(ssdp_multicast), ssdp_port)
  {}
  std::string Discover();
};