#pragma once

#include <string>
#include <functional>

using namespace std;

#include <boost/asio.hpp>

class irc
{
  boost::asio::io_service &io;
  boost::asio::ip::tcp::iostream stream;
  boost::asio::ip::tcp::socket socket;

  bool connected = false;
  void UpdateConnectedStatus(const string &str);
public:
  irc(boost::asio::io_service &);
  void Connect(string addr, int port = 6667);
  void Connect(string nick, string addr, int port = 6667);

  void Join(string channel);
  void Send(string msg);
  void Say(string msg);

  string ReadOnce();

  void OnMessage(function<void(irc &, string)>);
};