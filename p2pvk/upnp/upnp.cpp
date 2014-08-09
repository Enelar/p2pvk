#include "upnp.h"

#include <iostream>

#include <mutex>
#include <boost/array.hpp>

using namespace boost::asio;

bool upnp::OpenPort(string service_name, int port, IP_TYPE type)
{
  std::cout << "GATEWAY: " << gw << std::endl;
  std::cout << "IP: " << me << std::endl;
  auto rpc_location = Discover();
  std::cout << "FOUND: " << rpc_location << std::endl;

  //const std::string UPNP_MULTICAST_ADDRESS("239.255.255.250");
  //const u_short U PNP_PORT = 1900;
  //const u_long BUFFER_SIZE = 2048;





//  boost::asio::ip::udp::socket socket(io);
//  boost::asio::ip::udp::resolver resolver(io);
//  connect(socket, resolver.resolve({ gateway, "1900" }));
  
  return false;
}

string upnp::Discover()
{
  auto GenMyEndpoint = [this](unsigned short port) -> boost::asio::ip::udp::endpoint
  {
    return{ boost::asio::ip::address::from_string(me), port };
  };

  boost::asio::ip::udp::endpoint
    multicast(boost::asio::ip::address::from_string("239.255.255.250"), 1900),
    me = GenMyEndpoint(1900);

  boost::asio::ip::udp::socket socket(io);
  while (true)
  {
    try
    {
      socket = boost::asio::ip::udp::socket(io, multicast.protocol());
      socket.bind(me);
      break;
    }
    catch (boost::system::system_error &e)
    {
      me = GenMyEndpoint(rand() & ((1 << 16) - 1));
    }
  }

  const std::string MSEARCH_REQUEST(
    //"M-SEARCH * HTTP/1.1\r\nMX: 3\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nST: ");
  "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nST:upnp:rootdevice\r\nMAN:\"ssdp:discover\"\r\nMX:3\r\n\r\n\r\n");
  socket.send_to(boost::asio::buffer(MSEARCH_REQUEST), multicast);

  auto TryReadPacket = [&]() -> string
  {
    boost::array<char, 1 << 10> a; // buffer overflow

    deadline_timer timer(io);
    timer.expires_from_now(boost::posix_time::seconds(15));

    std::size_t size = -1;
    mutex m;
    m.lock();

    timer.async_wait([&](const boost::system::error_code& error)
    {
      m.unlock();
    });
    socket.async_receive_from(boost::asio::buffer(a), me, [&](const boost::system::error_code& error, std::size_t _s)
    {
      timer.cancel();
      size = _s;
      m.unlock();
    });

    while (io.run_one())
      if (m.try_lock())
        break;
    m.unlock();
    if (size == -1)
      return{};

    a[size] = 0;
    return{ &a[0] };
  };

  string answer = TryReadPacket();

  string needle = "LOCATION: ";
  auto pos = answer.find(needle);
  if (pos == -1)
    return "";
  auto res = answer.substr(pos + needle.length());
  return res.substr(0, res.find("\r\n"));
}

bool upnp::ClosePort(string service_name, int port, IP_TYPE type)
{
  return false;
}
