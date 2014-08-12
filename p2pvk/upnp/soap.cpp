#include "upnp.h"
#include <boost\lexical_cast.hpp>
#include "semaphore.h"
#include "split.h"
#include "http_client.h"

const std::string UPNPSERVICE_WANIPCONNECTION1 = "urn:schemas-upnp-org:service:WANIPConnection:1";

string upnp::SoapInitRequest(string ssdp_result_location)
{  //"http://192.168.29.1:56854/rootDesc.xml"
  struct
  {
    string addr, port, location;
  } gems;

  {
    // http: // other
    auto parts = parser::Split(ssdp_result_location, "//", false, false);
    assert(parts.size() == 2);
    // addr : port/location
    auto addr = parser::Split(parts[1], ":", false, false);
    if (addr.size() == 1)
    { // no port provided, use default 80
      gems.port = 80;
      auto f = parser::Split(addr[0], "/", false, true);
      gems.addr = f[0];
      gems.location = f[1];
    }
    else
    {
      gems.addr = addr[0];
      auto f = parser::Split(addr[1], "/", false, true);
      gems.port = f[0];
      gems.location = f[1];
    }
  }

  auto res = SoapGet(gems.addr, boost::lexical_cast<int>(gems.port), gems.location);
  auto parts = parser::Split(res, UPNPSERVICE_WANIPCONNECTION1, false, false);
  UPNPNATHTTPClient client(gems.addr, boost::lexical_cast<int>(gems.port));
  string a[3] = {"", "/WANIPCn.xml", UPNPSERVICE_WANIPCONNECTION1};
  client.GetWANIPAddress(a[0], a[1], a[2]);

  return res;
}

string upnp::SoapPost(string addr, int port, string location, string message)
{
  boost::asio::ip::tcp::socket socket(io);
  boost::asio::ip::tcp::resolver resolver(io);
  connect(socket, resolver.resolve({ addr, boost::lexical_cast<string>(port) }));

  {
    semaphore m;

    vector<string> params =
    {
      "POST " + location + " HTTP/1.1",
      "Host: " + addr,
      "Content-Type: application/soap+xml; charset=utf-8",
      "Content-Length: " + boost::lexical_cast<string>(message.length()),
      "SOAPAction: \"http://www.w3.org/2003/05/soap-envelope\"",
      ""
    };
    stringstream ss;
    for (auto &t : params)
      ss << t << "\r\n";
    ss << message;
    string buf = ss.str();

    boost::asio::async_write(socket, boost::asio::buffer(buf), [&m](const boost::system::error_code& error, std::size_t _s)
    {
      m.TurnOff();
    });

    while (m.Status())
      io.run();
  }

  array<char, 1 << 10> buf; // buffer overflow

  {
    semaphore m;

    boost::asio::async_read(socket, boost::asio::buffer(buf), [&m](const boost::system::error_code& error, std::size_t _s)
    {
      m.TurnOff();
    });

    while (m.Status())
      io.run();
  }

  return{ &buf[0] };
}

string upnp::SoapGet(string addr, int port, string location)
{
  boost::asio::ip::tcp::socket socket(io);
  boost::asio::ip::tcp::resolver resolver(io);
  connect(socket, resolver.resolve({ addr, boost::lexical_cast<string>(port) }));

  {
    semaphore m;

    vector<string> params =
    {
      "GET " + location + " HTTP/1.1",
      "Host: " + addr,
      "Content-Type: application/soap+xml; charset=utf-8",
      ""
    };
    stringstream ss;
    for (auto &t : params)
      ss << t << "\r\n";
    string buf = ss.str();

    boost::asio::async_write(socket, boost::asio::buffer(buf), [&m](const boost::system::error_code& error, std::size_t _s)
    {
      m.TurnOff();
    });

    while (m.Status())
      io.run();
  }

  vector<char> buf(10 << 20); // 10mbytes buffer

  {
    semaphore m;

    boost::asio::async_read(socket, boost::asio::buffer(buf), [&m](const boost::system::error_code& error, std::size_t _s)
    {
      m.TurnOff();
    });

    while (m.Status())
      io.run();
  }

  return{ &buf[0] };
}