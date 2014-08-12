#include "ssdp.h"
#include <vector>
#include <boost/array.hpp>
#include <mutex>

using namespace std;

const std::string ssdp::ssdp_multicast = "239.255.255.250";
const unsigned short ssdp::ssdp_port = 1900;

boost::asio::ip::udp::endpoint ssdp::BindTo(unsigned short port)
{
  boost::asio::ip::udp::endpoint me = { boost::asio::ip::address::from_string(ip), port };
  socket = boost::asio::ip::udp::socket(io, multicast.protocol());
  socket.bind(me);
  return me;
}

boost::asio::ip::udp::endpoint ssdp::OpenPort() noexcept
{
  int port = ssdp_port;
  do
  {
    try
    {
      return BindTo(port);
    }
    catch (boost::system::system_error &e)
    {
      port = rand() & ((1 << 16) - 1);
    }
  } while (true);
}

std::string ssdp::TryReadPacket(boost::asio::ip::udp::endpoint me, boost::posix_time::time_duration duration)
{
  boost::array<char, 1 << 10> a; // buffer overflow

  boost::asio::deadline_timer timer(io);
  timer.expires_from_now(duration);

  std::size_t size = -1;
  mutex m;
  m.lock();

  timer.async_wait([&](const boost::system::error_code& error)
  {
    if (error)
      return;
    m.unlock();
  });

  socket.async_receive_from(boost::asio::buffer(a), me, [&](const boost::system::error_code& error, std::size_t _s)
  {
    size = _s;
    m.unlock();
  });

  while (io.run_one())
    if (m.try_lock())
      break;
  timer.cancel();
  m.unlock();
  if (size == -1)
    return{};

  a[size] = 0;
  return{ &a[0] };
}

std::string ssdp::ExtractLocation(const string &answer)
{
  string needle = "LOCATION: ";
  auto pos = answer.find(needle);
  if (pos == -1)
    return "";
  auto res = answer.substr(pos + needle.length());
  return res.substr(0, res.find("\r\n"));
}

string ssdp::Discover()
{
  boost::asio::ip::udp::endpoint me = OpenPort();

  const auto MSEARCH_REQUEST = []()
  {
    vector<string> request =
    {
      "M-SEARCH * HTTP/1.1",
      "HOST: 239.255.255.250:1900",
      "ST:upnp:rootdevice",
      "MAN:\"ssdp:discover\"",
      "MX:3",
      "",
      ""
    };
    stringstream ss;
    for (auto &t : request)
      ss << t << "\r\n";
    return ss.str();
  }();

  socket.send_to(boost::asio::buffer(MSEARCH_REQUEST), multicast);

  string answer = TryReadPacket(me, boost::posix_time::seconds(5));
  return ExtractLocation(answer);
}
