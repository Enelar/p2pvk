#include "upnp.h"

#include <iostream>

#include <mutex>
#include <boost/array.hpp>
#include "../utils/split.h"

using namespace boost::asio;

#include "ssdp.h"
#include "../log/log.h"

#include <boost\lexical_cast.hpp>

#include "route_table.h"

upnp::upnp(boost::asio::io_service &_io)
  : io(_io)
{
}

bool upnp::OpenPort(string service_name, int port, IP_TYPE type)
{
  route_table t;
  auto gw = t.Gateway();
  std::cout << gw << std::endl;

  auto ips = t.LocalAddrToGW(gw);

  for (auto me : ips)
  {
    ssdp pp(io, me);
    auto rpc_location = pp.Discover();
    if (rpc_location == "")
      continue;

    auto res = SoapInitRequest(rpc_location);
    ExtractServices(res);
    auto wanip = GetWanIp();
    Log("Determined WAN: " + wanip);
    Log("Open port " + boost::lexical_cast<string>(port) + " for " + service_name);
    if (SoapOpenPort(service_name, port, port, me, type))
      return true;
  }

  return false;
}

bool upnp::ClosePort(string service_name, int port, IP_TYPE type)
{
  return false;
}
