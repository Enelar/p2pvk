#include "irc.h"

#include <boost/lexical_cast.hpp>
using boost::asio::buffer;
using namespace boost;
using namespace boost::asio;

irc::irc(io_service &_io)
  : io(_io), socket(io)
{
}

irc::~irc()
{
  std::unique_lock<std::mutex> lk(m);
  read_thread.get();
}

void irc::Connect(string addr, int port)
{
  using ip::tcp;
  socket = tcp::socket(io);
  tcp::resolver resolver(io);
  connect(socket, resolver.resolve({ addr, boost::lexical_cast<string>(port) }));
}

#include <iostream>
#include <future>

void irc::Connect(string nick, string addr, int port)
{
  Connect(addr, port);

  // Some magic from Tyler Allen
  // http://www.the-tech-tutorial.com/simple-c-irc-bot-template/
  ReadOnce();
  ReadOnce();
  ReadOnce();
  Say("NICK " + nick);
  Say("USER guest tolmoon tolsun :Mr.Noname");
}

string irc::ReadOnce()
{
  asio::streambuf b;
  read_until(socket, b, '\n');

  std::stringstream ss;
  ss << &b;
  auto ret = ss.str();
  UpdateConnectedStatus(ret);
  return ret;
}

void irc::UpdateConnectedStatus(const string &str)
{
  if (str.find("End of /MOTD command.") != -1)
    connected = true;
}

void irc::Join(string channel)
{
  last_channel = "#" + channel;
  Say("JOIN " + last_channel);
}

void irc::Send(string msg)
{
  boost::asio::write(socket, buffer(msg, msg.length()));
}

void irc::Say(string msg)
{
  Send(msg + "\r\n");
}

void irc::ChannelSay(string msg)
{
  ChannelSay(last_channel, msg);
}

void irc::ChannelSay(string channel, string msg)
{
  Say("PRIVMSG " + channel + " :" + msg);
}


void irc::OnMessage(function<void(irc &, string)> f)
{
  assert(!reading);
  reading = true;

  auto ReadingFunctor = [this, f]()
  {
    while (true)
    {
      string readed = ReadOnce();
      f(*this, readed);
      if (!m.try_lock())
        break;
      m.unlock();
    }
  };

  read_thread = async(std::launch::async, ReadingFunctor);
  read_thread.wait_for(std::chrono::microseconds(1));
}