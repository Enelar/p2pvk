#pragma once

#include <string>

using namespace std;

#include "../irc/irc.h"

class logger
{
  bool inited = false;
  irc stream;
public:
  logger(boost::asio::io_service &io, const string &channel = "p2pvk_logs");

  void Log(const string &anything);
};

#include <functional>

extern std::function<void(const string &anything)> (Log);
void PrepareLogFunction(boost::asio::io_service &io);
