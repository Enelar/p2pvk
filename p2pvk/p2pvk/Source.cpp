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
  upnp test(io);
  auto res = test.OpenPort("test", 30000, upnp::UDP);
  std::cout << res << std::endl;
  Log("TEST " + res ? "OK" : "FAIL");
}

void main()
{
  PrepareLogFunction(io);
  t_upnp();
  Log("Exit sequence initiated");
  std::cout << std::endl << "== INITIATE EXIT SEQUENCE." << std::endl;
}