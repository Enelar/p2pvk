// Based on best article ever
// http://www.codeproject.com/Articles/35853/Portmappings-on-UPnP-NAT-s-using-C-winsock-xerces

#pragma once

#include <string>
#include <map>

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
  map<string, string> soap_services;
  struct
  {
    string addr, port, location;
  } gems;

  string SoapInitRequest( string ssdp_result_location );
  string SoapPost(string addr, int port, string location, string message);
  string SoapGet(string addr, int port, string location);

  void ExtractServices(const string &);

  string GetWanIp();
  bool SoapOpenPort(const string &service_name, int externport, int localport, IP_TYPE proto);

  string ActiveWanKey();
};