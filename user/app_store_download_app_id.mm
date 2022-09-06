#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>

#include <sys/sysctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>
#include <sys/un.h>

int main(int argc, const char * argv[])
{
	int socket_, client_fd;

	struct sockaddr_in serv_addr;

	struct sockaddr_in local_address;

	socklen_t length;

	char *appID = (char*) argv[1];

	socket_ = socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;
	// serv_addr.sin_addr.s_addr = inet_addr("169.254.21.35");
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(1448);

	client_fd = connect(socket_, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr));

	send(socket_, appID, strlen(appID), 0);

	close(client_fd);

	return 0;
}