#include "log.h"
#include "../utils/split.h"

std::function<void(const string &anything)> (::Log);

void logger::Log(const string &anything)
{
  auto parts = parser::Split(anything, '\n', true, false);
  auto lk = sem.Lock();
  for (auto p : parts)
    send_queue.push_back(p);
}

#include <iostream>

logger::logger(boost::asio::io_service &io, const string &channel)
  : stream(io)
{
  stream.OnMessage([](irc &, string a){ std::cout << a << std::endl; });
  stream.Connect("p2p_" + irc::GenerateValidRandomNickSuffix(), "irc.freenode.org");
  stream.Join(channel);
  StartWriteProject();
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

void logger::StartWriteProject()
{
  sem.TurnOff();
  should_quit.TurnOff();

  auto WritingFunctor = [this]()
  {
    string task;
    while (true)
    {
      std::this_thread::sleep_for(1s);
      {
        auto lk = sem.Lock();
        if (!send_queue.size())
          continue;
        task = send_queue.front();
        send_queue.pop_front();
      }
      stream.ChannelSay(task);
      if (should_quit.Status())
        break;
    }
  };

  send_thread = async(std::launch::async, WritingFunctor);
  send_thread.wait_for(1ms);
}