#include "log.h"
#include "../utils/split.h"

std::function<void(const string &anything)> (::Log);

void logger::Log(const string &anything)
{
  auto parts = parser::Split(anything, '\n', true, false);
  for (auto p : parts)
    stream.ChannelSay(p);
}

#include <iostream>

logger::logger(boost::asio::io_service &io, const string &channel)
  : stream(io)
{
  stream.OnMessage([](irc &, string a){ std::cout << a << std::endl; });
  stream.Connect("p2p_" + irc::GenerateValidRandomNickSuffix(), "irc.freenode.org");
  stream.Join(channel);
}

void PrepareLogFunction(boost::asio::io_service &_io)
{
  boost::asio::io_service &io = _io;
  //logger op(io);

  Log = [&io](const string &anything)
  {
    static logger op(io);
    op.Log(anything);
  };
}

logger::~logger()
{
  auto lk = should_quit.Lock();
  send_thread.get();
}