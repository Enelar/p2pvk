#include "upnp.h"

#include <iostream>

#include <mutex>
#include <boost/array.hpp>
#include "../utils/split.h"

using namespace boost::asio;

#include "ssdp.h"
#include "../log/log.h"

#include <boost\lexical_cast.hpp>

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
  ExtractServices(res);
  auto wanip = GetWanIp();
  Log("Determined WAN: " + wanip);
  Log("Open port " + boost::lexical_cast<string>(port) + " for " + service_name);
  return SoapOpenPort(service_name, port, port, type);
}

bool upnp::ClosePort(string service_name, int port, IP_TYPE type)
{
  return false;
}
