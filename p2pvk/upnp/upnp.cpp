#include "upnp.h"

#include <iostream>

#include <mutex>
#include <boost/array.hpp>
#include "../utils/split.h"

using namespace boost::asio;

#include "ssdp.h"
#include "../log/log.h"

bool upnp::OpenPort(string service_name, int port, IP_TYPE type)
{
  Log("GATEWAY: " + gw);
  Log("IP: " + me);
  ssdp pp(io, me);
  auto rpc_location = pp.Discover();
  Log("FOUND: " + rpc_location);
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
