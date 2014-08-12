#include <boost/asio.hpp>
#include "../log/log.h"

#include <iostream>
boost::asio::io_service io;


void t_irc()
{

  irc test(io);
  test.Connect("Enetest434", "irc.freenode.org");

  test.OnMessage([]( irc &, const string &a )
  {
    std::cout << a;
  });
  test.Join("p2pvk_bots");

  while (1)
  {
    char aa[1024];
    std::cin.getline(aa, 1000);
    test.ChannelSay(aa);
  }
}

#include "../upnp/route_table.h"
#include "../upnp/http_client.h"
#include "../upnp/upnp.h"
#include "../upnp/ssdp.h"

void t_upnp()
{
  route_table t;
  auto gw = t.Gateway();
  std::cout << gw << std::endl;

  auto ips = t.LocalAddrToGW(gw);

  for (auto local_ip : ips)
  {
    upnp pp(io, local_ip, gw);
    if (pp.OpenPort("test", 30000, upnp::UDP))
      std::cout << "TEST OK" << std::endl;
    else
      std::cout << "TEST FAILED" << std::endl;
  }
  Log("Exit sequence initiated");
  std::cout << std::endl << "== INITIATE EXIT SEQUENCE." << std::endl;
}

void main()
{
  PrepareLogFunction(io);
  t_upnp();
}