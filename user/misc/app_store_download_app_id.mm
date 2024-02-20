/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
	char response[2048];

	int socket_, client_fd;

	struct sockaddr_in serv_addr;

	struct sockaddr_in local_address;

	socklen_t length;

	char *appID = (char*) argv[1];

	socket_ = socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(1448);

	client_fd = connect(socket_, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr));

	send(socket_, appID, strlen(appID), 0);

	/*while(1)
	{
		size_t bytes_received;

		bytes_received = recv(socket_, response, 2048, 0);

		if(bytes_received > 0)
		{
			fprintf(stdout, "%s\n", response);

			break;
		}
	}*/

	close(client_fd);

	return 0;
}