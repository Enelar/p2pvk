#pragma once

#include <string>
#include <list>

using namespace std;

#include "../irc/irc.h"
#include "../utils/semaphore.h"

class logger
{
  bool inited = false;
  irc stream;

  semaphore sem, should_quit;
  list<string> send_queue;
  future<void> send_thread;
public:
  logger(boost::asio::io_service &io, const string &channel = "p2pvk_logs");
  ~logger();

  void Log(const string &anything);
private:
  void StartWriteProject();
};

#include <functional>

extern std::function<void(const string &anything)> (Log);
void PrepareLogFunction(boost::asio::io_service &io);
