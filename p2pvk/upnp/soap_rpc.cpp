#include "upnp.h"

#include "http_client.h"

#include "../utils/split.h"
#include <boost\lexical_cast.hpp>

string upnp::ActiveWanKey()
{
  if (soap_services[UPNPSERVICE_WANIPCONNECTION1] != "")
    return UPNPSERVICE_WANIPCONNECTION1;
  if (soap_services[UPNPSERVICE_WANPPPCONNECTION1] != "")
    return UPNPSERVICE_WANPPPCONNECTION1;
  Log("ERROR: No active WAN connection discovered");
  exit(1);
  return "";
}

string upnp::GetWanIp()
{
  auto service = soap_services[ActiveWanKey()];
  UPNPNATHTTPClient client(gems.addr, boost::lexical_cast<int>(gems.port));
  string ret;
  if (!client.GetWANIPAddress(ret, service, ActiveWanKey()))
    return "";
  return ret;
}

bool upnp::SoapOpenPort(const string &service_name, int externport, int localport, string me, IP_TYPE proto )
{
  auto service = soap_services[ActiveWanKey()];
  string a[10] = {""};
  UPNPNATHTTPClient client(gems.addr, boost::lexical_cast<int>(gems.port));
  auto res = client.AddPortMapping(service_name, externport, localport, proto == TCP ? "TCP" : "UDP", me, service, ActiveWanKey());
  return res == UPNPNATHTTPClient::SoapSucceeded;
}