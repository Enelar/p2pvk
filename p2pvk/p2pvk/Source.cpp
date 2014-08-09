#include <boost/asio.hpp>
#include "../irc/irc.h"

void main()
{
  boost::asio::io_service io;
  boost::asio::ip::tcp::iostream("aa", "bb");
  irc test(io);
  test.Connect("Enetest434", "irc.freenode.org");
  test.Join("p2pvk_bots");
  while (1)
  {
  }
  test.Say("PRIVMSG #p2pvk_bots Hello");
}