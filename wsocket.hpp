#ifndef _INC_WSOCKET_HPP_
#define _INC_WSOCKET_HPP_

#include <stdlib.h>
#include <wbase.h>

SOCKET ConnectTCP(LPCWSTR szServer, LPCWSTR szService);
SOCKET ConnectTCP(LPCSTR szServer, LPCSTR szService);
SOCKET ConnectTCP(LPCWSTR szServer, u_short usPort);
SOCKET ConnectTCP(LPCSTR szServer, u_short usPort);
SOCKET ConnectTCP(u_long ulAddress, u_short usPort);

class WSocket : public WKernelObject
{
public:
	WSocket()
	{
	}

	explicit WSocket(SOCKET hSocket) : WKernelObject((HANDLE)hSocket)
	{
	}

	explicit WSocket(HANDLE hSocket) : WKernelObject(hSocket)
	{
	}

	~WSocket()
    {
        Close();
    }

    SOCKET Socket()
    {
        return (SOCKET)hObject;
    }

    bool Close()
    {
        if (!IsValidHandle())
            return true;

        if (shutdown(Socket(), 0x01) == 0)
        {
            shutdown(Socket(), 0x00);
            closesocket(Socket());
            hObject = INVALID_HANDLE_VALUE;
            return true;
        }
        else
            return WKernelObject::Close();
    }

    bool Connect(LPCSTR szServer, LPCSTR szService, LPCSTR szProtocol = NULL)
        const
    {
        if (szServer == NULL)
            return false;

        if (szService == NULL)
            return false;

        u_short usPort = htons((u_short)strtoul(szService, NULL, 0));
        if (usPort == 0)
        {
            // Get port name for service
            servent *service = getservbyname(szService, szProtocol);
            if (!service)
                return false;

            usPort = service->s_port;
        }

        return Connect(szServer, usPort);
    }

    bool Connect(LPCSTR szServer, u_short usPort)
        const
    {
        if (szServer == NULL)
            return false;

        if (usPort == 0)
            return false;

        // Get server address
        u_long haddr = inet_addr(szServer);

        // Wasn't IP? Lookup host.
        if (haddr == INADDR_NONE)
        {
            hostent *hent = gethostbyname(szServer);
            if (!hent)
                return false;

            haddr = *(u_long*)hent->h_addr;
        }

        return Connect(haddr, usPort);
    }

    bool Connect(u_long ulAddress, u_short usPort)
        const
    {
        // Is socket handle ok?
        if (!IsValidHandle())
            return false;

        sockaddr_in addr = { 0 };
        addr.sin_family = AF_INET;
        addr.sin_port = usPort;
        addr.sin_addr.s_addr = ulAddress;

        if (connect((SOCKET)hObject, (sockaddr*)&addr, sizeof addr)
            == SOCKET_ERROR)
            return false;

        return true;
    }

    bool Listen(u_long ulAddress = INADDR_ANY, u_short usPort = 0,
        int backlog = 0)
        const
    {
        // Is socket handle ok?
        if (!IsValidHandle())
            return false;

        sockaddr_in addr = { 0 };
        addr.sin_family = AF_INET;
        addr.sin_port = usPort;
        addr.sin_addr.s_addr = ulAddress;

        if (bind((SOCKET)hObject, (sockaddr*)&addr, sizeof addr) == SOCKET_ERROR)
            return false;

        if (listen((SOCKET)hObject, backlog) == SOCKET_ERROR)
            return false;

        return true;
    }

    u_short GetLocalPort()
        const
    {
        sockaddr_in sin;
        int sinsize = sizeof sin;
        if (getsockname((SOCKET)hObject, (sockaddr*)&sin, &sinsize)
            == SOCKET_ERROR)
            return 0;

        return ntohs(sin.sin_port);
    }

    u_short GetPeerPort()
        const
    {
        sockaddr_in sin;
        int sinsize = sizeof sin;
        if (getpeername((SOCKET)hObject, (sockaddr*)&sin, &sinsize)
            == SOCKET_ERROR)
            return 0;

        return ntohs(sin.sin_port);
    }

    u_long GetLocalAddr()
        const
    {
        sockaddr_in sin;
        int sinsize = sizeof sin;
        if (getsockname((SOCKET)hObject, (sockaddr*)&sin, &sinsize)
            == SOCKET_ERROR)
            return 0;

        return sin.sin_addr.s_addr;
    }

    char *GetLocalAddrString()
        const
    {
        sockaddr_in sin;
        int sinsize = sizeof sin;
        if (getsockname((SOCKET)hObject, (sockaddr*)&sin, &sinsize)
            == SOCKET_ERROR)
            return 0;

        return inet_ntoa(sin.sin_addr);
    }

    u_long GetPeerAddr()
        const
    {
        sockaddr_in sin;
        int sinsize = sizeof sin;
        if (getpeername((SOCKET)hObject, (sockaddr*)&sin, &sinsize)
            == SOCKET_ERROR)
            return 0;

        return sin.sin_addr.s_addr;
    }

    char *GetPeerAddrString()
        const
    {
        sockaddr_in sin;
        int sinsize = sizeof sin;
        if (getpeername((SOCKET)hObject, (sockaddr*)&sin, &sinsize)
            == SOCKET_ERROR)
            return 0;

        return inet_ntoa(sin.sin_addr);
    }

    int Shutdown(int how = 2)
    {
        return shutdown((SOCKET)hObject, how);
    }
};

class WSocketTCP : public WSocket
{
public:
    WSocketTCP() : WSocket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
    {
    }

    WSocketTCP(SOCKET sock) : WSocket(sock)
    {
    }

    bool ReCreate()
	{
		Close();
		hObject = (HANDLE)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		return IsValidHandle();
	}

    bool Connect(LPCSTR szServer, LPCSTR szService)
        const
    {
        return WSocket::Connect(szServer, szService, "tcp");
    }

    int SetNoDelay(bool no_delay)
        const
    {
        return setsockopt((SOCKET)hObject, IPPROTO_TCP, TCP_NODELAY,
            (const char*)&no_delay, sizeof no_delay);
    }
};

class WSocketTCPAccept : public WSocketTCP
{
	static SOCKET Accept(SOCKET s, sockaddr_in *addr, int *addrlen)
	{
		*addrlen = sizeof(sockaddr_in);
		return accept(s, (sockaddr*)addr, addrlen);
	}

public:
	sockaddr_in addr;
	int addr_len;

    explicit WSocketTCPAccept(WSocketTCP &s) : WSocketTCP(Accept((SOCKET)s.Handle(), &addr, &addr_len))
    {
    }

    explicit WSocketTCPAccept(SOCKET s) : WSocketTCP(Accept(s, &addr, &addr_len))
    {
    }
};

class WSocketUDP : public WSocket
{
public:
    WSocketUDP() : WSocket(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
    {
    }

    bool ReCreate()
    {
        Close();
        hObject = (HANDLE)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        return IsValidHandle();
    }
};

#ifdef _WINSOCK2API_
class WSocketSyncTCP : public WSocket
{
public:
    WSocketSyncTCP(LPWSAPROTOCOL_INFO lpProtocolInfo = NULL,
        GROUP g = 0,
        DWORD dwFlags = 0)
        : WSocket(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
        lpProtocolInfo, g, dwFlags))
    {
    }

    bool ReCreate(LPWSAPROTOCOL_INFO lpProtocolInfo = NULL,
        GROUP g = 0,
        DWORD dwFlags = 0)
    {
        Close();
        hObject = (HANDLE)WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
            lpProtocolInfo, g, dwFlags);
        return IsValidHandle();
    }
};

class WSocketSyncUDP : public WSocket
{
public:
    WSocketSyncUDP(LPWSAPROTOCOL_INFO lpProtocolInfo = NULL,
        GROUP g = 0,
        DWORD dwFlags = 0)
        : WSocket(WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP,
        lpProtocolInfo, g, dwFlags))
    {
    }

    bool ReCreate(LPWSAPROTOCOL_INFO lpProtocolInfo = NULL,
        GROUP g = 0,
        DWORD dwFlags = 0)
    {
        Close();
        hObject = (HANDLE)WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP,
            lpProtocolInfo, g, dwFlags);
        return IsValidHandle();
    }
};
#endif // _WINSOCK2API_

#endif // _INC_WSOCKET_HPP_
