#pragma once

#include <string>
#include <memory>

// very small http client for the UPnP-NAT
class UPNPNATHTTPClient
{
public:
  enum Result
  {
    Succeeded,
    Failed,
    HTTPOK,
    HTTPNG,
    Timeout,
    SocketError
  };

  enum Status
  {
    TCPConnected,
    TCPDisconnected,
    TCPDisconnectionIncomplete,
    TCPUnknown
  };

  enum SoapResult
  {
    SoapFailed
    , SoapSucceeded
    , SoapNoConnection
    , SoapParseError
    , SoapIllegalResponce
    , SoapServiceTypeMismatch
  };

  static const size_t MAX_BUFFER;

public:
  UPNPNATHTTPClient();
  UPNPNATHTTPClient(
    const std::string &ip, unsigned short port
    );

  ~UPNPNATHTTPClient();

private:
  Result Connect(void);
public:
  Result Connect(
    const std::string &ip, unsigned short port);
  Result Disconnect(void);

  Result SendRequest(const std::string &req);
  std::string ReceiveResponce(void);

  // UPnP Requests
public:
  SoapResult GetUPNPServiceDescription(
    std::string &desc, const std::string &desc_url
    );

  //////////////////////////////////////////////////////////////////////////
  // Layer3Forwarding
  //////////////////////////////////////////////////////////////////////////
  SoapResult GetDefaultConnectionService(
    const std::string &control_url
    , const std::string &service_type
    );

  //////////////////////////////////////////////////////////////////////////
  // WANIPConnection services
  //////////////////////////////////////////////////////////////////////////
  SoapResult GetWANIPAddress(
    std::string &wanip
    , const std::string &control_url
    , const std::string &service_type
    );

  SoapResult AddPortMapping(
    const std::string &desc
    //, const std::string &nat_ip
    , unsigned short external_port
    , unsigned short internal_port
    , const std::string &protocol
    , const std::string &igd_internal_ip
    , const std::string &control_url
    , const std::string &service_type
    );

  SoapResult DeletePortMapping(
    unsigned short external_port
    , const std::string &protocol
    , const std::string &control_url
    , const std::string &service_type
    );

  //////////////////////////////////////////////////////////////////////////
  // WANCommonInterafaceConfig services
  //////////////////////////////////////////////////////////////////////////
  SoapResult GetCommonLinkProperties(
    const std::string &service_type
    , const std::string &control_url
    ); //incomplete arg

  SoapResult GetWANAccessProvider(
    std::string &ip //out
    , const std::string &service_type
    , const std::string &control_url
    );

  SoapResult GetActiveConnection(
    unsigned short connection_id //in
    , const std::string &service_type
    , const std::string &control_url
    );

public:
  void SetStatus(Status status);

private:
  struct Impl;
  std::shared_ptr<Impl> pimpl;
};


#define HTTP_VERSION "1.1"

#define HTTP_GET_HEADER_TEMPLATE\
	""//todo

#define HTTP_POST_HEADER_TEMPLATE\
	"POST %s HTTP"\
	HTTP_VERSION\
	"\r\n"\
	"HOST: %s:%u\r\n"\
	"SOAPAction:\"%s#%s\"\r\n"\
	"CONTENT-TYPE: text/xml ; charset=\"utf-8\"\r\n"\
	"Content-Length: %d\r\n\r\n"

#define HTTP_200\
	HTTP_VERSION\
	" 200 OK"

#define SOAP_ENVELOPE\
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"\
	"<s:Envelope xmlns:s="\
	"\"http://schemas.xmlsoap.org/soap/envelope/\" "\
	"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"\
	"<s:Body>\r\n"\
	"<u:%s xmlns:u=\"%s\">\r\n%s"\
	"</u:%s>\r\n"\
	"</s:Body>\r\n"\
	"</s:Envelope>\r\n"

// is this the old format ?
//#define SOAP_ENVELOPE\
//	"<?xml version=\"1.0\"?>\r\n"\
//	"<SOAP-ENV:Envelope xmlns:SOAP-ENV="\
//	"\"http://schemas.xmlsoap.org/soap/envelope/\" "\
//	"SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"\
//	"<SOAP-ENV:Body>"\
//	"<m:%s xmlns:m=\"%s\">%s"\
//	"</u:%s>"\
//	"</SOAP-ENV:Body>"\
//	"</SOAP-ENV:Envelope>"



//////////////////////////////////////////////////////////////////////////
// Layer3Forwarding service's templates
// 
// AN:Action Name
// AA:Action Args
//
//#define AN_GET_COMMONLINKPROPERTIES "GetActiveConnection"
//#define AA_GET_COMMONLINKPROPERTIES\
//"<ArgName></ArgName>"
//
//////////////////////////////////////////////////////////////////////////
#define AN_SET_DEFAULTCONNECTIONSERVICE "SetDefaultConnectionService"
#define AA_SET_DEFAULTCONNECTIONSERVICE\
	"<NewDefaultConnectionService>%s</NewDefaultConnectionService>"

#define AN_GET_DEFAULTCONNECTIONSERVICE "GetDefaultConnectionService"
#define AA_GET_DEFAULTCONNECTIONSERVICE\
	"<NewDefaultConnectionService></NewDefaultConnectionService>"



//////////////////////////////////////////////////////////////////////////
// WANIPConnection or WANPPPConnection service's templates
// 
// AN:Action Name
// AA:Action Args
//
//#define AN_GET_COMMONLINKPROPERTIES "GetActiveConnection"
//#define AA_GET_COMMONLINKPROPERTIES\
//"<ArgName></ArgName>"
//
//////////////////////////////////////////////////////////////////////////
#define PORTMAPPING_LEASE_DURATION "63072000"

#define AN_GET_WANIPADDRESS "GetExternalIPAddress"
#define AA_GET_WANIPADDRESS\
	"<NewExternalIPAddress></NewExternalIPAddress>"


#define AN_ADD_PORTMAPPING "AddPortMapping"
#define AA_ADD_PORTMAPPING\
	"<NewRemoteHost></NewRemoteHost>"\
	"<NewExternalPort>%u</NewExternalPort>"\
	"<NewProtocol>%s</NewProtocol>"\
	"<NewInternalPort>%u</NewInternalPort>"\
	"<NewInternalClient>%s</NewInternalClient>"\
	"<NewEnabled>1</NewEnabled>"\
	"<NewPortMappingDescription>%s</NewPortMappingDescription>"\
	"<NewLeaseDuration>"\
	PORTMAPPING_LEASE_DURATION\
	"</NewLeaseDuration>"


#define AN_DELETE_PORTMAPPING "DeletePortMapping"
#define AA_DELETE_PORTMAPPING\
	"<NewExternalPort>%u</NewExternalPort>"\
	"<NewProtocol>%s</NewProtocol>"\
	"<NewRemoteHost></NewRemoteHost>"


//////////////////////////////////////////////////////////////////////////
// WANCommonInterfaceConfig service's templates
//
// AN:Action Name
// AA:Action Args
//
//#define AN_GET_COMMONLINKPROPERTIES "GetActiveConnection"
//#define AA_GET_COMMONLINKPROPERTIES\
//"<ArgName></ArgName>"
//
//////////////////////////////////////////////////////////////////////////
#define AN_GET_COMMONLINKPROPERTIES "GetCommonLinkProperties"
#define AA_GET_COMMONLINKPROPERTIES\
	"<NewWANAccessType></NewWANAccessType>"\
	"<NewLayer1UpstreamMaxBitRate></NewLayer1UpstreamMaxBitRate>"\
	"<NewLayer1DownstreamMaxBitRate></NewLayer1DownstreamMaxBitRate>"\
	"<NewPhysicalLinkStatus></NewPhysicalLinkStatus>"

#define AN_GET_WANACCESSPROVIDER "GetWANAccessProvider"
#define AA_GET_WANACCESSPROVIDER\
	"<NewWANAccessProvider></NewWANAccessProvider>"

#define AN_GET_COMMONLINKPROPERTIES "GetActiveConnection"
#define AA_GET_COMMONLINKPROPERTIES\
	"<NewActiveConnectionIndex>%u</NewActiveConnectionIndex>"\
	"<NewActiveConnDeviceContainer></NewActiveConnDeviceContainer>"\
	"<NewActiveConnectionServiceID></NewActiveConnectionServiceID>"

static const std::string UPNPSERVICE_LAYER3FORWARDING1 = "urn:schemas-upnp-org:service:Layer3Forwarding:1";
static const std::string UPNPSERVICE_WANCOMMONINTERFACECONFIG1 = "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";
static const std::string UPNPSERVICE_WANPPPCONNECTION1 = "urn:schemas-upnp-org:service:WANPPPConnection:1";
static const std::string UPNPSERVICE_WANIPCONNECTION1 = "urn:schemas-upnp-org:service:WANIPConnection:1";
