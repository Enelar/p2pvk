#include "http_client.h"

#include <WinSock2.h>

#include <boost\format.hpp>
#include <boost\lexical_cast.hpp>

using namespace std;
using namespace boost;

#include "../utils/split.h"


namespace {

}

const size_t UPNPNATHTTPClient::MAX_BUFFER = 102400;

struct UPNPNATHTTPClient::Impl
{
  Impl()
    : http_sock(INVALID_SOCKET)
    , status(TCPUnknown)
    , host_port(0)
  {
  }

  Impl(const string &ip, unsigned short port)
    : http_sock(INVALID_SOCKET)
    , host(ip)
    , host_port(port)
    , status(TCPUnknown)
  {
  }

  ~Impl()
  {
  }

  int http_sock;
  Status status;

  string host;
  unsigned short host_port;

  // [internal]class for the connection management.
  struct Manager
  {
    Manager(UPNPNATHTTPClient *c, Impl *impl) : client(c)
    {
      client->Connect(impl->host, impl->host_port);
    }
    ~Manager()
    {
      client->Disconnect();
    }

    UPNPNATHTTPClient *client;
  };

  struct BlockSocket
  {
    BlockSocket(int s) : sock(s)
    {
      unsigned long flag(1);
      int res = ioctlsocket(sock, FIONBIO, &flag);
      if (0 != res)
      {
       // BHDebug(false, L"socket error");
        return;
      }
    }
    ~BlockSocket()
    {
      unsigned long flag(0);
      int res = ioctlsocket(sock, FIONBIO, &flag);
      if (0 != res)
      {
       // BHDebug(false, L"socket error");
        return;
      }
    }

    int sock;
  };

};



UPNPNATHTTPClient::UPNPNATHTTPClient()
  : pimpl(new Impl)
{


}


UPNPNATHTTPClient::UPNPNATHTTPClient(
  const std::string &ip
  , unsigned short port
  )
  : pimpl(new Impl(ip, port))
{


}


UPNPNATHTTPClient::~UPNPNATHTTPClient()
{

}



UPNPNATHTTPClient::Result UPNPNATHTTPClient::Connect(void)
{
  SetStatus(TCPUnknown);

  int res(0);

  pimpl->http_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (INVALID_SOCKET == pimpl->http_sock)
  {
    return SocketError;
  }

  sockaddr_in igd;
  igd.sin_family = AF_INET;
  igd.sin_port = htons(pimpl->host_port);
  igd.sin_addr.s_addr = inet_addr(pimpl->host.c_str());

  {
    //Impl::BlockSocket _block( pimpl->http_sock );
    res = connect(
      pimpl->http_sock, (struct sockaddr*)&igd, sizeof(sockaddr_in));
    if (0 == res)
    {
      pimpl->status = TCPConnected;
      return Succeeded;
    }
  }

  //aconsole("failed");
  return Failed;
}


UPNPNATHTTPClient::Result UPNPNATHTTPClient::Connect(
  const std::string &ip, unsigned short port
  )
{
 // aconsole(str(format("connecting as http to %s:%u.....") % ip%port));
  int res(0);
  pimpl->http_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (INVALID_SOCKET == pimpl->http_sock)
  {
    return SocketError;
  }

  sockaddr_in igd;
  igd.sin_family = AF_INET;
  igd.sin_port = htons(port);
  igd.sin_addr.s_addr = inet_addr(ip.c_str());

  // connect with the timeout
  {
    //Impl::BlockSocket _block( pimpl->http_sock );
    res = connect(
      pimpl->http_sock, (struct sockaddr*)&igd, sizeof(sockaddr_in)
      );
    if (0 == res)
    {
      pimpl->status = TCPConnected;
      pimpl->host = ip;
      pimpl->host_port = port;
   //   aconsole("connected.");
      return Succeeded;
    }
  }

 // aconsole("failed.");
  return Failed;
}


UPNPNATHTTPClient::Result UPNPNATHTTPClient::Disconnect(void)
{
  if (INVALID_SOCKET != pimpl->http_sock)
  {
    int res(0);
    res = shutdown(pimpl->http_sock, SD_SEND);
    if (0 != res)
    {
      pimpl->status = TCPDisconnectionIncomplete;
      return SocketError;
    }
    res = closesocket(pimpl->http_sock);
    if (0 != res)
    {
      pimpl->status = TCPDisconnectionIncomplete;
      return SocketError;
    }
  }

  return Succeeded;
}


UPNPNATHTTPClient::Result UPNPNATHTTPClient::SendRequest(const std::string &req)
{
  timeval timeout;
  timeout.tv_sec = 1;
  int request_size = static_cast<int>(req.size());
  int total(0);
  while (true)
  {
    fd_set writable;
    FD_ZERO(&writable);
    FD_SET(pimpl->http_sock, &writable);
    int error = select(0, NULL, &writable, NULL, &timeout);
    if (SOCKET_ERROR != error && 0 < error)
    {
      if (FD_ISSET(pimpl->http_sock, &writable))
      {
        int sended = send(pimpl->http_sock, req.c_str(), static_cast<int>(req.size()), 0);
        if (SOCKET_ERROR == sended)
        {
          return SocketError;
        }
        total += sended;
        if (total >= request_size)
        {
          break;
        }
      }
    }
    else if (0 == error)
    {
      return Timeout;
    }
  }//while

  return Succeeded;
}


std::string UPNPNATHTTPClient::ReceiveResponce(void)
{
  std::string res;
  char buff[MAX_BUFFER] = { 0 };
  int received(0);

  timeval timeout = { 0 };
  timeout.tv_sec = 10;

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(pimpl->http_sock, &rfds);
  int err = select(pimpl->http_sock, &rfds, NULL, NULL, &timeout);
  if (0 == err)
  {
    return string("");
  }

  if (FD_ISSET(pimpl->http_sock, &rfds))
  {
    while (received = recv(pimpl->http_sock, buff, sizeof(buff), 0) > 0)
    {
      res += buff;
      memset(buff, 0, sizeof(buff));
    }
  }

  return res;
}


UPNPNATHTTPClient::SoapResult UPNPNATHTTPClient::GetUPNPServiceDescription(
  std::string &desc
  , const std::string &desc_url
  )
{
  Impl::Manager connection_raii(this, pimpl.get());
  if (TCPConnected != pimpl->status)
  {
    return SoapNoConnection;
  }

  string request = boost::str(
    boost::format("GET %s HTTP/1.1\r\nHost: %s:%d\r\n\r\n")
    % desc_url.c_str()
    % pimpl->host.c_str()
    % pimpl->host_port
    );
  Result res = SendRequest(request);
  if (Succeeded != res)
  {
    return SoapFailed;
  }

  desc = ReceiveResponce();
  // extracting only the part of the xml.
  if (!desc.empty())
  {
    size_t xml_head = desc.find("<?xml");
    if (string::npos != xml_head)
    {
      desc = desc.substr(xml_head, desc.length());
      return SoapSucceeded;
    }
  }

  return SoapFailed;
}


//////////////////////////////////////////////////////////////////////////
///// UPnP Requests
//////////////////////////////////////////////////////////////////////////
UPNPNATHTTPClient::SoapResult UPNPNATHTTPClient::GetDefaultConnectionService(
  const std::string &control_url
  , const std::string &service_type
  )
{
  if (UPNPSERVICE_LAYER3FORWARDING1 == service_type)
  {
    return SoapServiceTypeMismatch;
  }
  Impl::Manager connection_raii(this, pimpl.get());
  if (TCPConnected != pimpl->status)
  {
    return SoapNoConnection;
  }

  string soap_args = boost::str(
    format(AA_GET_DEFAULTCONNECTIONSERVICE)
    );
  string soap_body = boost::str(
    format(SOAP_ENVELOPE)
    % AN_GET_DEFAULTCONNECTIONSERVICE
    % service_type.c_str()
    % soap_args.c_str()
    % AN_GET_DEFAULTCONNECTIONSERVICE
    );
  string soap_request = boost::str(
    format(HTTP_POST_HEADER_TEMPLATE)
    % control_url.c_str()
    % pimpl->host.c_str()
    % pimpl->host_port
    % service_type.c_str()
    % AN_GET_DEFAULTCONNECTIONSERVICE
    % soap_body.size()
    );

  soap_request += soap_body;

  Result res = SendRequest(soap_request);
  if (Failed == res)
  {
//    BHDebug(false, L"A SOAP request sending failed.");
    return SoapFailed;
  }

//  dout(http_debug, soap_request);
  string soap_responce = ReceiveResponce();
//  dout(http_debug, soap_responce);

  return SoapSucceeded;
}


UPNPNATHTTPClient::SoapResult UPNPNATHTTPClient::GetWANIPAddress(
  std::string &wanip
  , const std::string &control_url
  , const std::string &service_type
  )
{
  if (
    UPNPSERVICE_WANIPCONNECTION1 != service_type
    && UPNPSERVICE_WANPPPCONNECTION1 != service_type
    )
  {
    return SoapServiceTypeMismatch;
  }

  Impl::Manager connection_raii(this, pimpl.get());
  if (TCPConnected != pimpl->status)
  {
    return SoapNoConnection;
  }

  string soap_args = boost::str(
    format(AA_GET_WANIPADDRESS)
    );
  string soap_body = boost::str(
    format(SOAP_ENVELOPE)
    % AN_GET_WANIPADDRESS
    % service_type.c_str()
    % soap_args.c_str()
    % AN_GET_WANIPADDRESS
    );
  string soap_request = boost::str(
    format(HTTP_POST_HEADER_TEMPLATE)
    % control_url.c_str()
    % pimpl->host.c_str()
    % pimpl->host_port
    % service_type.c_str()
    % AN_GET_WANIPADDRESS
    % soap_body.size()
    );

  soap_request += soap_body;

  Result res = SendRequest(soap_request);
  if (Failed == res)
  {
//    BHDebug(false, L"A SOAP request sending failed.");
    return SoapFailed;
  }

//  dout(http_debug, soap_request);
  string soap_responce = ReceiveResponce();
  //dout(http_debug, soap_responce);

  string tt(HTTP_200);
  if (soap_responce.find(HTTP_200) != string::npos)
  {
    string xml = soap_responce.substr(soap_responce.find("<?xml"), soap_responce.length());
    auto parts = parser::Split(xml, "NewExternalIPAddress>", false, false);
    if (!parts.size())
      return SoapParseError;
    else if (parts.size() != 3)
      return SoapIllegalResponce;
    wanip = parser::Split(parts[1], "</", true, false)[0];
    return SoapSucceeded;
  }
  Log(soap_request);
  Log(soap_responce);
  return SoapFailed;
}


UPNPNATHTTPClient::SoapResult UPNPNATHTTPClient::AddPortMapping(
  const std::string &desc
  //, const std::string &nat_ip
  , unsigned short external_port
  , unsigned short internal_port
  , const std::string &protocol
  , const std::string &igd_internal_ip
  , const std::string &control_url
  , const std::string &service_type
  )
{
  if (
    UPNPSERVICE_WANIPCONNECTION1 != service_type
    && UPNPSERVICE_WANPPPCONNECTION1 != service_type
    )
  {
    return SoapServiceTypeMismatch;
  }

  Impl::Manager connection_raii(this, pimpl.get());
  if (TCPConnected != pimpl->status)
  {
    return SoapNoConnection;
  }

  string soap_args = boost::str(
    format(AA_ADD_PORTMAPPING)
    % external_port
    % protocol.c_str()
    % internal_port
    % igd_internal_ip // pimpl->host.c_str()
    % desc.c_str()
    );
  string soap_body = boost::str(
    format(SOAP_ENVELOPE)
    % AN_ADD_PORTMAPPING
    % service_type.c_str()
    % soap_args.c_str()
    % AN_ADD_PORTMAPPING
    );
  string soap_request = boost::str(
    format(HTTP_POST_HEADER_TEMPLATE)
    % control_url.c_str()
    % pimpl->host.c_str()
    % pimpl->host_port
    % service_type.c_str()
    % AN_ADD_PORTMAPPING
    % soap_body.size()
    );

  soap_request += soap_body;

  Result res = SendRequest(soap_request);
  if (Failed == res)
  {
//    BHDebug(false, L"A SOAP request sending failed.");
    return SoapFailed;
  }

//  dout(http_debug, soap_request);
  string soap_responce = ReceiveResponce();
//  dout(http_debug, soap_responce);
  if (soap_responce.find(HTTP_200) != string::npos)
  {
    return SoapSucceeded;
  }
  Log(soap_request);
  Log(soap_responce);

  return SoapFailed;
}


UPNPNATHTTPClient::SoapResult UPNPNATHTTPClient::DeletePortMapping(
  unsigned short external_port
  , const std::string &protocol
  , const std::string &control_url
  , const std::string &service_type
  )
{
  if (
    UPNPSERVICE_WANIPCONNECTION1 != service_type
    && UPNPSERVICE_WANPPPCONNECTION1 != service_type
    )
  {
    return SoapServiceTypeMismatch;
  }

  Impl::Manager connection_raii(this, pimpl.get());
  if (TCPConnected != pimpl->status)
  {
    return SoapNoConnection;
  }

  string soap_args = boost::str(
    format(AA_DELETE_PORTMAPPING)
    % external_port
    % protocol.c_str()
    );
  string soap_body = boost::str(
    format(SOAP_ENVELOPE)
    % AN_DELETE_PORTMAPPING
    % service_type.c_str()
    % soap_args.c_str()
    % AN_DELETE_PORTMAPPING
    );
  string soap_request = boost::str(
    format(HTTP_POST_HEADER_TEMPLATE)
    % control_url.c_str()
    % pimpl->host.c_str()
    % pimpl->host_port
    % service_type.c_str()
    % AN_DELETE_PORTMAPPING
    % soap_body.size()
    );

  soap_request += soap_body;

  Result res = SendRequest(soap_request);
  if (Succeeded != res)
  {
//    BHDebug(false, L"A SOAP request sending failed.");
    return SoapFailed;
  }

//  dout(http_debug, soap_request);
  string soap_responce = ReceiveResponce();
//  dout(http_debug, soap_responce);

  if (soap_responce.find(HTTP_200) != string::npos)
  {
    return SoapSucceeded;
  }

  return SoapFailed;
}


//////////////////////////////////////////////////////////////////////////
// WANCommonInterfaceConfig:1 services
//////////////////////////////////////////////////////////////////////////
UPNPNATHTTPClient::SoapResult UPNPNATHTTPClient::GetCommonLinkProperties(
  const std::string &service_type
  , const std::string &control_url
  )
{
  if (UPNPSERVICE_WANCOMMONINTERFACECONFIG1 != service_type)
  {
    return SoapServiceTypeMismatch;
  }

  Impl::Manager connection_raii(this, pimpl.get());

  string soap_args = boost::str(
    format(AA_GET_COMMONLINKPROPERTIES)
    );
  string soap_body = boost::str(
    format(SOAP_ENVELOPE)
    % AN_GET_COMMONLINKPROPERTIES
    % service_type.c_str()
    % soap_args.c_str()
    % AN_GET_COMMONLINKPROPERTIES
    );
  string soap_request = boost::str(
    format(HTTP_POST_HEADER_TEMPLATE)
    % control_url.c_str()
    % pimpl->host.c_str()
    % pimpl->host_port
    % service_type.c_str()
    % AN_GET_COMMONLINKPROPERTIES
    % soap_body.size()
    );

  soap_request += soap_body;

  Result res = SendRequest(soap_request);
  if (Succeeded != res)
  {
    //BHDebug(false, L"A SOAP request sending failed.");
    return SoapFailed;
  }

//  dout(http_debug, soap_request);
  string soap_responce = ReceiveResponce();
  //dout(http_debug, soap_responce);

  if (soap_responce.find(HTTP_200) != string::npos)
  {
    return SoapSucceeded;
  }

  return SoapFailed;
}


UPNPNATHTTPClient::SoapResult UPNPNATHTTPClient::GetWANAccessProvider(
  std::string &ip
  , const std::string &service_type
  , const std::string &control_url
  )
{
  if (UPNPSERVICE_WANCOMMONINTERFACECONFIG1 != service_type)
  {
    return SoapServiceTypeMismatch;
  }

  Impl::Manager connection_raii(this, pimpl.get());

  string soap_args = boost::str(
    format(AA_GET_WANACCESSPROVIDER)
    );
  string soap_body = boost::str(
    format(SOAP_ENVELOPE)
    % AN_GET_WANACCESSPROVIDER
    % service_type.c_str()
    % soap_args.c_str()
    % AN_GET_WANACCESSPROVIDER
    );
  string soap_request = boost::str(
    format(HTTP_POST_HEADER_TEMPLATE)
    % control_url.c_str()
    % pimpl->host.c_str()
    % pimpl->host_port
    % service_type.c_str()
    % AN_GET_WANACCESSPROVIDER
    % soap_body.size()
    );

  soap_request += soap_body;

  Result res = SendRequest(soap_request);
  if (Succeeded != res)
  {
//    BHDebug(false, L"A SOAP request sending failed.");
    return SoapFailed;
  }

//  dout(http_debug, soap_request);
  string soap_responce = ReceiveResponce();
//  dout(http_debug, soap_responce);

  if (soap_responce.find(HTTP_200) != string::npos)
  {
    return SoapSucceeded;
  }

  return SoapFailed;
}


UPNPNATHTTPClient::SoapResult UPNPNATHTTPClient::GetActiveConnection(
  unsigned short connection_id //in
  , const std::string &service_type
  , const std::string &control_url
  )
{
  if (UPNPSERVICE_WANCOMMONINTERFACECONFIG1 != service_type)
  {
    return SoapServiceTypeMismatch;
  }

  Impl::Manager connection_raii(this, pimpl.get());

  string soap_args = boost::str(
    format(AA_GET_COMMONLINKPROPERTIES)
    % connection_id
    );
  string soap_body = boost::str(
    format(SOAP_ENVELOPE)
    % AN_GET_COMMONLINKPROPERTIES
    % service_type.c_str()
    % soap_args.c_str()
    % AN_GET_COMMONLINKPROPERTIES
    );
  string soap_request = boost::str(
    format(HTTP_POST_HEADER_TEMPLATE)
    % control_url.c_str()
    % pimpl->host.c_str()
    % pimpl->host_port
    % service_type.c_str()
    % AN_GET_COMMONLINKPROPERTIES
    % soap_body.size()
    );

  soap_request += soap_body;

  Result res = SendRequest(soap_request);
  if (Succeeded != res)
  {
//    BHDebug(false, L"A SOAP request sending failed.");
    return SoapFailed;
  }

//  dout(http_debug, soap_request);
  string soap_responce = ReceiveResponce();
  //dout(http_debug, soap_responce);

  if (soap_responce.find(HTTP_200) != string::npos)
  {
    return SoapSucceeded;
  }

  return SoapFailed;
}



void UPNPNATHTTPClient::SetStatus(Status status)
{
  pimpl->status = status;
}


