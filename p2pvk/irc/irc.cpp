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
  read_thread.wait();
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
  if (!connected)
    load_strings += std::count(ret.begin(), ret.end(), '\n');
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

bool irc::IsConnected() const
{
  return connected;
}

namespace
{
  auto Alphabet() -> const string
  {
    return
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "1234567890";
  }
}

#include <boost\random.hpp>
#include <boost\random\random_device.hpp>

string irc::GenerateValidRandomNickSuffix( int length )
{
  auto &chars = Alphabet();
  boost::random::random_device rng;
  boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);

  std::stringstream ss;
  for (int i = 0; i < length; ++i)
    ss << chars[index_dist(rng)];
  return ss.str();
}