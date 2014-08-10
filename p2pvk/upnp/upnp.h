// Based on best article ever
// http://www.codeproject.com/Articles/35853/Portmappings-on-UPnP-NAT-s-using-C-winsock-xerces

#pragma once

#include <string>

using namespace std;

#include <boost/asio.hpp>

struct upnp
{
  boost::asio::io_service &io;
  string me, gw;

  upnp(boost::asio::io_service &_io, string my_ip, string gw_ip) : io(_io), me(my_ip), gw(gw_ip)
  {}

  enum IP_TYPE
  {
    TCP,
    UDP
  };

  bool OpenPort(string service_name, int port, IP_TYPE type);
  bool ClosePort(string service_name, int port, IP_TYPE type);
private:
};