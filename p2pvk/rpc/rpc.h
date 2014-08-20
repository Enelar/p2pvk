#pragma once

#include <boost\asio.hpp>
#include <string>

using namespace boost::asio;

#include "rpc_instance.h"
#include "../utils/semaphore.h"

#include <future>

struct rpc : boost::noncopyable
{
  semaphore exit;
  io_service &io;
  ip::tcp::acceptor local_socket, extern_socket;

  std::map<std::string, rpc_instance> locals, externs;
  std::future<void> acceptor;
public:
  rpc(boost::asio::io_service &io, int local_port = 30238, int extern_port = 30240);
  ~rpc();
private:
  ip::tcp::acceptor OpenLocalPort(int port) const;
  ip::tcp::acceptor OpenGlobalPort(int port, const std::string &name = "p2pvk_dev") const;

  void HandleNewConnection(boost::asio::ip::tcp::socket &s, bool is_local);
};