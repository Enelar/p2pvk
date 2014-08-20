#include "rpc.h"
#include "../upnp/upnp.h"
#include <iostream>
#include <boost\lexical_cast.hpp>

#include "../log/log.h"


using namespace boost::asio::ip;

#include <memory>
#include "../utils/crt.h"

rpc::~rpc()
{
  exit.TurnOff();
  while (acceptor.wait_for(1ms) != future_status::ready)
    io.run();
}

rpc::rpc(boost::asio::io_service &_io, int local_port, int extern_port)
  : io(_io), local_socket(_io), extern_socket(_io)
{
  extern_socket = OpenGlobalPort(extern_port);
  local_socket = OpenLocalPort(local_port);

  auto Pooler = [this](ip::tcp::acceptor *accept, const bool is_extern, std::function<void(std::unique_ptr<rpc_instance> &)> OnNew )
  {
    semaphore ready;

    std::unique_ptr<rpc_instance> new_connection;
    auto AcceptCallback = [&, this](const boost::system::error_code& error)
    {

    };

    while (true)
    {
      new_connection = std::make_unique<rpc_instance>(io, !is_extern);
      auto binded_callback = std::bind(&rpc::AcceptCallback, shared_from_this(), std::ref(ready), is_extern, std::ref(new_connection), std::placeholders::_1);
      accept->async_accept(new_connection->socket, binded_callback);
    loop:
      std::this_thread::sleep_for(1ms);
      if (!exit.Status())
        break;
      if (!ready.Status())
        goto loop;

      ready.TurnOn();
    }
    accept->close();
    ready.Lock();
  };

  acceptor = std::async(std::launch::async,
    [this, Pooler]()
  {
    using namespace std;
#define THREAD_DEBUG
    auto local = std::async(std::launch::async, Pooler, &local_socket, false, [this](std::unique_ptr<rpc_instance> &con)
    {
      //locals.insert({ "NEW", move(*con.release()) });
    });
#ifndef THREAD_DEBUG
    auto external = std::async(std::launch::async, Pooler, &extern_socket, true, [this](std::unique_ptr<rpc_instance> &con)
    {
      externs.insert({"NEW", move(*con.release())});
    });

    external.wait();
#endif
    local.wait();
    Log("Pooler finished");
  });
  acceptor.wait_for(1ms);
}

void rpc::AcceptCallback(semaphore &ready, bool is_extern, std::unique_ptr<rpc_instance> &, const boost::system::error_code& error)
{
  if (error)
  {
    ready.TurnOff();
    return;
  }
  //OnNew(new_connection);
  ready.TurnOff();
}


ip::tcp::acceptor rpc::OpenLocalPort(int port) const
{
  tcp::endpoint local(address::from_string("127.0.0.1"), port);
  tcp::acceptor ret(io, local);
  return ret;
}

ip::tcp::acceptor rpc::OpenGlobalPort(int port, const string &name) const
{
  upnp helper(io);
  if (!helper.OpenPort(name, port, upnp::TCP))
    std::cout << "Open port failed" << std::endl;

  tcp::acceptor ret(io, tcp::endpoint(tcp::v4(), port));
  return ret;
}