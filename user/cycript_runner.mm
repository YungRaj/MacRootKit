#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h> 

#include <dlfcn.h>

#include <mach/mach.h> 
#include <mach/exc.h>

#include <arpa/inet.h>

#include <ifaddrs.h>

#include <Foundation/Foundation.h>

NSString* get_ip_address()
{
	NSString *ip_address;

	char *ip;

	struct ifaddrs *ifaddrs;
	struct ifaddrs *ifaddr;

	struct in_addr inaddr;

	ifaddrs = NULL;
	ip_address = NULL;

	if(!getifaddrs(&ifaddrs))
	{
		for(ifaddr = ifaddrs; ifaddr; ifaddr = ifaddr->ifa_next)
		{
			if(ifaddr->ifa_addr->sa_family == 2)
			{
				if(strcmp(ifaddr->ifa_name, "en0") == 0)
				{
					inaddr.s_addr = *(uint32_t*) &ifaddr->ifa_addr->sa_data;

					ip = inet_ntoa(inaddr);

					ip_address = [NSString stringWithUTF8String:ip];
				}
			}
		}
	}

	free(ifaddrs);

	return ip_address;
}

void run_cycript_server()
{
	mach_vm_address_t CYListenServer;

	NSString *ip_address;

	ip_address = get_ip_address();

	CYListenServer = reinterpret_cast<mach_vm_address_t>(dlsym((void*) 0xFFFFFFFFFFFFFFFE, "CYListenServer"));

	if(CYListenServer)
	{
		typedef void (*_CYListenServer_) (int port);
		void (*_CYListenServer) (int port);

		_CYListenServer = reinterpret_cast<_CYListenServer_>(CYListenServer);

		_CYListenServer(1337);

		NSLog(@"CYListenServer = %lld called!\n", CYListenServer);
	}

	NSLog(@"Running cycript server on port 1337 at IP Address %@", ip_address);
}

extern "C"
{
	__attribute__((constructor))
	static void initializer()
	{
		printf("[%s] initializer()\n", __FILE__);

		run_cycript_server();
	}

	__attribute__ ((destructor))
	static void finalizer()
	{
		printf("[%s] finalizer()\n", __FILE__);
	}
}