#ifndef _INC_TCPROUTE_HPP_
#define _INC_TCPROUTE_HPP_

#include <winstrct.h>

#include <werrlog.h>

#define TCP_PACKET_SIZE    32768
#define UDP_PACKET_SIZE    1024  // If can't get size using ioctlsocket

/**
   Used to forward TCP connection through a socks firewall, transparent to the
   connecting application. Socks server name is specified in the environment
   variable TCPROUTE_SOCKSADDRESS, an optional port in TCPROUTE_SOCKSPORT and
   the socks version level i TCPROUTE_SOCKSVERSION.
*/
SOCKET
ConnectTCPSocksProxy(LPCSTR szServer,
		     LPCSTR szService);

/**
   Used to forward TCP connection through a socks firewall, transparent to the
   connecting application. Socks server name is specified in the environment
   variable TCPROUTE_SOCKSADDRESS, an optional port in TCPROUTE_SOCKSPORT and
   the socks version level i TCPROUTE_SOCKSVERSION.
*/
SOCKET
ConnectTCPSocksProxyWithLookup(LPCSTR szServer,
			       LPCSTR szService);

SOCKET
ConnectTCPHttpProxy(LPCSTR szServer,
		    LPCSTR szService);

SOCKET
ConnectTCPHttpProxyWithLookup(LPCSTR szServer,
			      LPCSTR szService);

#endif // _INC_TCPROUTE_HPP_
