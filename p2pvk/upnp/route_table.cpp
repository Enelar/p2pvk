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
#include <iostream>

vector<string> route_table::LocalAddrToGW(string remote)
{
  PIP_ADAPTER_INFO adapters = nullptr;
  DWORD size = 0;
  assert(GetAdaptersInfo(adapters, &size) == ERROR_BUFFER_OVERFLOW);
  adapters = (PIP_ADAPTER_INFO)HeapAlloc(GetProcessHeap(), 0, size);
  assert(adapters);
  assert(GetAdaptersInfo(adapters, &size) == NO_ERROR);

  auto adapter = adapters;
  auto pAdapter = adapter;
  while (pAdapter) {
    printf("\tComboIndex: \t%d\n", pAdapter->ComboIndex);
    printf("\tAdapter Name: \t%s\n", pAdapter->AdapterName);
    printf("\tAdapter Desc: \t%s\n", pAdapter->Description);
    printf("\tAdapter Addr: \t");
    for (auto i = 0; i < pAdapter->AddressLength; i++) {
      if (i == (pAdapter->AddressLength - 1))
        printf("%.2X\n", (int)pAdapter->Address[i]);
      else
        printf("%.2X-", (int)pAdapter->Address[i]);
    }
    printf("\tIndex: \t%d\n", pAdapter->Index);
    printf("\tType: \t");
    switch (pAdapter->Type) {
    case MIB_IF_TYPE_OTHER:
      printf("Other\n");
      break;
    case MIB_IF_TYPE_ETHERNET:
      printf("Ethernet\n");
      break;
    case MIB_IF_TYPE_TOKENRING:
      printf("Token Ring\n");
      break;
    case MIB_IF_TYPE_FDDI:
      printf("FDDI\n");
      break;
    case MIB_IF_TYPE_PPP:
      printf("PPP\n");
      break;
    case MIB_IF_TYPE_LOOPBACK:
      printf("Lookback\n");
      break;
    case MIB_IF_TYPE_SLIP:
      printf("Slip\n");
      break;
    default:
      printf("Unknown type %ld\n", pAdapter->Type);
      break;
    }

    printf("\tIP Address: \t%s\n",
      pAdapter->IpAddressList.IpAddress.String);
    printf("\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);

    printf("\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
    printf("\t***\n");

    if (pAdapter->DhcpEnabled) {
      printf("\tDHCP Enabled: Yes\n");
      printf("\t  DHCP Server: \t%s\n",
        pAdapter->DhcpServer.IpAddress.String);

      printf("\t  Lease Obtained: ");
      /* Display local time */
    }
    else
      printf("\tDHCP Enabled: No\n");

    if (pAdapter->HaveWins) {
      printf("\tHave Wins: Yes\n");
      printf("\t  Primary Wins Server:    %s\n",
        pAdapter->PrimaryWinsServer.IpAddress.String);
      printf("\t  Secondary Wins Server:  %s\n",
        pAdapter->SecondaryWinsServer.IpAddress.String);
    }
    else
      printf("\tHave Wins: No\n");
    pAdapter = pAdapter->Next;
    printf("\n");
  }

  while (adapter)
  {

    auto *gateway = &adapter->GatewayList;
    std::cout << "ADAPTER: " << (int)adapter << adapter->AdapterName << std::endl;
    while (gateway)
    {
      std::cout << "GW " << (int)gateway << " ";
      std::cout << "((" << remote << ")) ((" << string(gateway->IpAddress.String) << "))" << std::endl;
      if (remote == gateway->IpAddress.String)
        goto up;
     // break; // nasty workaround: using only first gateway
      gateway = gateway->Next;
    }
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
  HeapFree(GetProcessHeap(), 0, adapters);
  return ret;
}