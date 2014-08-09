#pragma once

#include <string>
#include <functional>
#include <future>
#include <mutex>

using namespace std;

#include <boost/asio.hpp>

class irc
{
  boost::asio::io_service &io;
  mutex m;
  future<void> read_thread;
  boost::asio::ip::tcp::socket socket;

  bool connected = false, reading = false;
  void UpdateConnectedStatus(const string &str);

  string last_channel;
public:
  irc(boost::asio::io_service &);
  ~irc();
  irc(const irc &) = delete;
  void Connect(string addr, int port = 6667);
  void Connect(string nick, string addr, int port = 6667);

  void Join(string channel);
  void Send(string msg);
  void Say(string msg);

  void ChannelSay(string);
  void ChannelSay(string channel,string);

  string ReadOnce();

  void OnMessage(function<void(irc &, string)>);
};