#include <boost/asio.hpp>
#include "../log/log.h"

#include <iostream>
boost::asio::io_service io;


#include "../rpc/rpc.h"

void t_f()
{
  rpc test(io);
  while (1)
  {
    io.run();
    std::this_thread::sleep_for(1ms);
  }

}

void main()
{
  //PrepareLogFunction(io);
  t_f();
  Log("Exit sequence initiated");
  std::cout << std::endl << "== INITIATE EXIT SEQUENCE." << std::endl;
}