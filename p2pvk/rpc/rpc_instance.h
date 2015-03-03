#pragma once

#include <boost\asio.hpp>

using namespace boost::asio;

struct rpc_instance
{
  io_service &io;
  ip::tcp::socket socket;
  const bool is_local;
  bool start;

  rpc_instance(io_service &_io, bool _is_local) : io(_io), socket(io), is_local(_is_local)
  {}
  ~rpc_instance()
  {
  }

  rpc_instance(rpc_instance &&);

  void Start()
  {
    start = true;
  }

  operator ip::tcp::socket &()
  {
    return socket;
  }

  void StartReading();
  void LocalReader(const boost::system::error_code& error, size_t bytesT);
  void ExternReader(const boost::system::error_code& error, size_t bytesT);
};