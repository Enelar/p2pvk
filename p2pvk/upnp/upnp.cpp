#include "upnp.h"

#include <iostream>

#include <mutex>
#include <boost/array.hpp>
#include "split.h"

using namespace boost::asio;

#include "ssdp.h"

bool upnp::OpenPort(string service_name, int port, IP_TYPE type)
{
  std::cout << "GATEWAY: " << gw << std::endl;
  std::cout << "IP: " << me << std::endl;
  ssdp pp(io, me);
  auto rpc_location = pp.Discover();
  if (rpc_location == "")
    return false;

  auto res = SoapInitRequest(rpc_location);
  std::cout << res << std::endl;
  return true;
}

bool upnp::ClosePort(string service_name, int port, IP_TYPE type)
{
  return false;
}
