#include "upnp.h"

#include <iostream>

#include <mutex>
#include <boost/array.hpp>

using namespace boost::asio;

bool upnp::OpenPort(string service_name, int port, IP_TYPE type)
{
  std::cout << "GATEWAY: " << gw << std::endl;
  std::cout << "IP: " << me << std::endl;
//  auto rpc_location = Discover();
//  std::cout << "FOUND: " << rpc_location << std::endl;

  //if (rpc_location == "")
    return false;
  return true;
}

bool upnp::ClosePort(string service_name, int port, IP_TYPE type)
{
  return false;
}
