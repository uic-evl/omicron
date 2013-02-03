#ifdef  _WIN32
	#include <winsock.h>
	#include <stdio.h>
	#include <stdlib.h>
#else
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif	

typedef struct {
	float x;
	float y;
	float z;
	float yaw;
	float pitch;
	float roll;
}tracker_frame;

#ifdef  _WIN32

u_long lookupAddress(const char* host)
{
    u_long nRemoteAddr = inet_addr(host);
    if (nRemoteAddr == INADDR_NONE) {
        // pcHost isn't a dotted IP, so resolve it through DNS
        hostent* pHE = gethostbyname(host);
        if (pHE == 0) {
            return INADDR_NONE;
        }
        nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
    }

    return nRemoteAddr;
}

/* server side*/
SOCKET setupSocketServer(unsigned short portnum)
{ 
	SOCKET sock;							/* Socket descriptor of server */
	struct sockaddr_in server;			/* Information about the server */
 	char hostName[256];				/* Name of the server */

	WSADATA w;							/* Used to open windows connection */
	/* Open windows connection */
	if (WSAStartup(0x0101, &w) != 0)
	{
		printf("Could not open Windows connection.\n");
		exit(0);
	}

	/* Open a datagram socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("Could not create socket.\n");
		WSACleanup();
		exit(0);
	}

	/* Clear out server struct */
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	server.sin_family = AF_INET;
	server.sin_port = htons(portnum);

	/* Get host name of this computer */
	gethostname(hostName, sizeof(hostName));
	server.sin_addr.s_addr=lookupAddress(hostName); 

	/* Bind address to socket */
	if (bind(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
	{
		printf("Could not bind name to socket.\n");
		closesocket(sock);
		WSACleanup();
		exit(0);
	}

	printf("Server running on %s@ %d\n",inet_ntoa(server.sin_addr),portnum);
	printf("Press CTRL + C to quit\n");
	return sock;
}

/* client side*/
SOCKET setupSocketClient(char * config,struct sockaddr_in *server)
{ 
	SOCKET sock;							/* Socket descriptor of server */
	char serverName[256];
	int  portnum;
	FILE * pServerConfig;

	pServerConfig = fopen(config,"r");
	if (pServerConfig != NULL)
	{
		fscanf(pServerConfig,"Server=%s\nPort=%d",serverName,&portnum);
	}
	else
	{
		printf("Unable to open client config file\n");
		exit(0);
	}


	WSADATA w;							/* Used to open windows connection */
	/* Open windows connection */
	if (WSAStartup(0x0101, &w) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(0);
	}

	/* Open a datagram socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		exit(0);
	}

	/* Clear out server struct */
	memset(server, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	server->sin_family = AF_INET;
	server->sin_port = htons(portnum);
	/* Set server address */
    server->sin_addr.s_addr=lookupAddress(serverName); 
	printf("Press a key to quit\n");

	return sock;
}

void finishSocket(SOCKET *sock)
{
	closesocket(*sock);
	WSACleanup();
}

int sendData(unsigned int sock,const tracker_frame * sendBuf, int bufLength, const sockaddr* dest )
{
	int sendBytes;
	if ((sendBytes = sendto(sock, (const char *)sendBuf, bufLength , 0, dest, sizeof(struct sockaddr_in))) < 0)
	{
		printf("Error transmitting data.\n");
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	return sendBytes;
}
float floatSwap( float f )
{
  union
  {
    float f;
    unsigned char b[4];
  } dat1, dat2;

  dat1.f = f;
  dat2.b[0] = dat1.b[3];
  dat2.b[1] = dat1.b[2];
  dat2.b[2] = dat1.b[1];
  dat2.b[3] = dat1.b[0];
  return dat2.f;
}

int recvData(unsigned int sock,tracker_frame * recvBuf, int recvLength, sockaddr* src, int isLittleEndian)
{
	int recvBytes;
	int addrLength = (int)sizeof(struct sockaddr_in);
	tracker_frame temp;
	if (!isLittleEndian)
	{
		if ((recvBytes = recvfrom(sock, (char *)&temp, recvLength, 0, src, &addrLength)) < 0)
		{
			printf( "Error receiving data.\n");
			closesocket(sock);
			WSACleanup();
			return -1;
		}
		recvBuf->x = floatSwap(temp.x);
		recvBuf->y = floatSwap(temp.y);
		recvBuf->z = floatSwap(temp.z);
		recvBuf->yaw = floatSwap(temp.yaw);
		recvBuf->pitch = floatSwap(temp.pitch);
		recvBuf->roll = floatSwap(temp.roll);
		
	}
	else
	{
		if ((recvBytes = recvfrom(sock, (char *)recvBuf, recvLength, 0, src, &addrLength)) < 0)
		{
			printf( "Error receiving data.\n");
			closesocket(sock);
			WSACleanup();
			return -1;
		}
	}
	return recvBytes;
}

#else

void lookupAddress(const char * host, struct sockaddr_in * server)
{
 	char hostName[256];				/* Name of the server */
	struct hostent* s_hostent;	

	/* Get host name of this computer */
	if (host == NULL)
	{
		if (gethostname(hostName,sizeof(hostName)) < 0)
		{
			printf("Cannot gethostname\n");		
			exit(0);
		}	
		s_hostent = gethostbyname(hostName);
		
	}
	else
		s_hostent = gethostbyname(host);
	
	if (s_hostent != NULL) { //got name of server
		memcpy(&server->sin_addr, s_hostent->h_addr, s_hostent->h_length);
		printf("Server is running on %s...\n",inet_ntoa(server->sin_addr));
	}
	else
	{
		printf("Cannot gethostbyname\n");			
		exit(0);
	}	
}

/* server side*/
int setupSocketServer(unsigned short portnum)
{ 
	int sock;							/* Socket descriptor of server */
	struct sockaddr_in server;			/* Information about the server */

	/* Open a datagram socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		printf("Could not create socket.\n");
		exit(0);
	}

	/* Clear out server struct */
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	server.sin_family = AF_INET;
	server.sin_port = htons(portnum);
	lookupAddress(NULL,&server);

	/* Bind address to socket */
	if (bind(sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0)
	{
		printf("Could not bind name to socket.\n");
		close(sock);
		exit(0);
	}

	/* Print out server information */
	printf("Server running on %s@ %d\n",inet_ntoa(server.sin_addr),portnum);
	printf("Press CTRL + C to quit\n");
	return sock;
}

void finishSocket(int sock)
{
	close(sock);
}

int sendData(int sock,const tracker_frame * sendBuf, int bufLength, const struct sockaddr* dest )
{
	int sendBytes;
	if ((sendBytes = sendto(sock, (const char *)sendBuf, bufLength , 0, dest, sizeof(struct sockaddr_in))) < 0)
	{
		printf("Error transmitting data.\n");
		close(sock);
		return -1;
	}
	return sendBytes;
}
float floatSwap( float f )
{
  union
  {
    float f;
    unsigned char b[4];
  } dat1, dat2;

  dat1.f = f;
  dat2.b[0] = dat1.b[3];
  dat2.b[1] = dat1.b[2];
  dat2.b[2] = dat1.b[1];
  dat2.b[3] = dat1.b[0];
  return dat2.f;
}

int recvData(int sock,tracker_frame * recvBuf, int recvLength, struct sockaddr* src, int isLittleEndian)
{
	int recvBytes;
	socklen_t addrLength = sizeof(struct sockaddr_in);
	tracker_frame temp;
	if (!isLittleEndian)
	{
		if ((recvBytes = recvfrom(sock, &temp, recvLength, 0, src, &addrLength)) < 0)
		{
			printf( "Error receiving data.\n");
			close(sock);
			return -1;
		}
		recvBuf->x = floatSwap(temp.x);
		recvBuf->y = floatSwap(temp.y);
		recvBuf->z = floatSwap(temp.z);
		recvBuf->yaw = floatSwap(temp.yaw);
		recvBuf->pitch = floatSwap(temp.pitch);
		recvBuf->roll = floatSwap(temp.roll);

	}
	else
	{
		if ((recvBytes = recvfrom(sock, recvBuf, recvLength, 0, src, &addrLength)) < 0)
		{
			printf( "Error receiving data.\n");
			close(sock);
			return -1;
		}
	}
	return recvBytes;
}
#endif

