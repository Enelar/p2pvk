#include <boost/asio.hpp>
#include "../log/log.h"

#include <iostream>
boost::asio::io_service io;


#include "../rpc/rpc.h"
#include "../utils/crt.h"

_CrtMemState s1, s2, s3;

void t_f()
{
  rpc test(io);
//  _CrtSetBreakAlloc(1300);
  for (auto i = 0; i < 200; i++)
  {
    std::this_thread::sleep_for(1s);
    io.run();
  }
  // memory allocations take place here
  _CrtMemDumpAllObjectsSince(&s1);
  _CrtMemCheckpoint(&s2);

  if (_CrtMemDifference(&s3, &s1, &s2))
  {
    _CrtMemDumpStatistics(&s3);
  }
}

void main()
{
  //PrepareLogFunction(io);
  _CrtMemCheckpoint(&s1);
  t_f();
  _CrtMemDumpAllObjectsSince(&s1);
  Log("Exit sequence initiated");
  std::cout << std::endl << "== INITIATE EXIT SEQUENCE." << std::endl;
}