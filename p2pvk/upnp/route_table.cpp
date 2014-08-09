#include "route_table.h"

#include <WinSock2.h>
#include <Iphlpapi.h>

#pragma comment(lib, "Iphlpapi.lib")

vector<route_rule> route_table::Fetch()
{
  DWORD lowest_metric = ~0;
  int pgw = -1;

  DWORD dwSize = 0;
  GetIpForwardTable(NULL, &dwSize, false);
  PMIB_IPFORWARDTABLE pForwardInfo =(MIB_IPFORWARDTABLE *)calloc(dwSize, 1);
  if (GetIpForwardTable(pForwardInfo, &dwSize, false) != NO_ERROR)
    return{};
  vector<route_rule> ret;
  for (unsigned long i = 0; i < pForwardInfo->dwNumEntries; i++)
  {
    MIB_IPFORWARDROW *row = &pForwardInfo->table[i];
    if (!row)
    {
      ret.push_back({});
      continue;
    }

    route_rule route_entry;
    route_entry.destination.assign(inet_ntoa(*(struct in_addr *)&row->dwForwardDest));
    route_entry.netmask.assign(inet_ntoa(*(struct in_addr *)&row->dwForwardMask));
    route_entry.gateway.assign(inet_ntoa(*(struct in_addr *)&row->dwForwardNextHop));
    route_entry.index = row->dwForwardIfIndex;
    route_entry.type = row->dwForwardType;
    route_entry.metric = row->dwForwardMetric1;

    if (!row->dwForwardDest && !row->dwForwardMask)
    {
      route_entry.is_default_gateway = true;
      if (lowest_metric > route_entry.metric)
      {
        lowest_metric = route_entry.metric;
        pgw = i;
      }
    }
    ret.push_back(route_entry);
  }
  if (pgw >= 0)
    ret[pgw].is_primary_gateway = true;
  free(pForwardInfo);
  return ret;
}

string route_table::Gateway()
{
  for (auto rule : Fetch())
    if (rule.is_primary_gateway)
      return rule.gateway;
  return{};
}

#include <assert.h>

vector<string> route_table::LocalAddrToGW(string remote)
{
  PIP_ADAPTER_INFO adapters = nullptr;
  DWORD size = 0;
  assert(GetAdaptersInfo(adapters, &size) == ERROR_BUFFER_OVERFLOW);
  adapters = (PIP_ADAPTER_INFO)malloc(size);
  assert(adapters);
  assert(GetAdaptersInfo(adapters, &size) == NO_ERROR);

  auto adapter = adapters;
  while (adapter)
  {
    auto *gateway = &adapter->GatewayList;
    while (gateway)
      if (gateway->IpAddress.String == remote)
        goto up;
      else
        gateway = gateway->Next;
    adapter = adapter->Next;
  }

up:
  vector<string> ret;
  auto *ip = &adapter->IpAddressList;
  while (ip)
  {
    ret.push_back(ip->IpAddress.String);
    ip = ip->Next;
  }
  free(adapters);
  return ret;
}