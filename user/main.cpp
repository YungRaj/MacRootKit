#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#include <sys/sysctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h> 

#include <mach/mach.h> 
#include <mach/exc.h>

#include <mach/mach.h>

#include <iostream>

#include <IOKit/IOKitLib.h>

#include "Kernel.hpp"
#include "UserMachO.hpp"
#include "Task.hpp"
#include "Dyld.hpp"

using namespace std;

int main()
{
	kern_return_t kr;

	vm_address_t remoteStack;

	thread_t thread;

	arm_thread_state64_t state = {0};

	mach_msg_type_number_t stateCount = ARM_THREAD_STATE64_COUNT;

	Kernel *kernel = new Kernel();

	printf("Kernel base = 0x%llx slide = 0x%llx\n", kernel->getBase(), kernel->getSlide());

	Task *task = new Task(kernel, "Twitter");

	printf("PID 1382 task = 0x%llx proc = 0x%llx\n", task->getTask(), task->getProc());

	MachO *macho = task->getDyld()->cacheDumpImage("libdyld.dylib");

	mach_vm_address_t dlopen = macho->getSymbolAddressByName("_dlopen") + macho->getAslrSlide();

	printf("dlopen = 0x%llx\n", dlopen);

	if((kr = thread_create(task->getTaskPort(), &thread)) != KERN_SUCCESS)
	{
		fprintf(stderr, "Could not create new thread in task!\n");

		return -1;
	}

	thread_set_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, ARM_THREAD_STATE64_COUNT);

	kr = vm_allocate(task->getTaskPort(), &remoteStack, 16192, VM_FLAGS_ANYWHERE);

	if (kr != KERN_SUCCESS)
	{
		printf("Unable allocate memory!: %s\n", mach_error_string(kr));
		
		return -1;
	}

	kr = vm_protect(task->getTaskPort(), remoteStack, 16192, FALSE, VM_PROT_WRITE | VM_PROT_READ);
	
	if(kr != KERN_SUCCESS)
	{
		printf("Unable protect memory!: %s\n", mach_error_string(kr));
		
		return -1;
	}

	// dlopen("libcycript.dylib", RTLD_NOW | RTLD_GLOBAL, 0, 0);

	thread_get_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, &stateCount);

	thread_suspend(thread);

	thread_terminate(thread);

	vm_deallocate(task->getTaskPort(), remoteStack, 16192);

	delete task;

	delete kernel;

	return 0;
}