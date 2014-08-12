#include "log.h"

std::function<void(const string &anything)> (::Log);

void logger::Log(const string &anything)
{
  stream.ChannelSay(anything);
}

#include <iostream>

logger::logger(boost::asio::io_service &io, const string &channel)
  : stream(io)
{
  stream.OnMessage([](irc &, string a){ std::cout << a << std::endl; });
  stream.Connect("p2p_" + irc::GenerateValidRandomNickSuffix(), "irc.freenode.org");
  stream.Join(channel);
  Log("Logger connected");
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