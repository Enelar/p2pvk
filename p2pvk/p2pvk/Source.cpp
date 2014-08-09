#include <boost/asio.hpp>
#include "../irc/irc.h"

#include <iostream>

void f()
{
  boost::asio::io_service io;

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

void main()
{
  f();
}