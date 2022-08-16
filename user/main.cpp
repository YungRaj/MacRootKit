#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#include <sys/sysctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h> 

#include <mach/mach.h> 
#include <mach/exc.h>

#include <pthread.h>
#include <ptrauth.h>

#include <IOKit/IOKitLib.h>

#include "Kernel.hpp"
#include "UserMachO.hpp"
#include "Task.hpp"
#include "Dyld.hpp"
#include "PAC.hpp"

#include <arm64/PatchFinder_arm64.hpp>

using namespace std;

#define STACK_SIZE ((1024 * 1024) * 512)


#define ROP_ret "\xff\x0f\x5f\xd6"
#define ALIGNSIZE 8
#define align64(x) ( ((x) + ALIGNSIZE - 1) & ~(ALIGNSIZE - 1) )

Kernel *kernel;

Task *task;

MachO *libDyld;
MachO *libSystemPthread;

mach_vm_address_t dlopen;
mach_vm_address_t pthread_create_from_mach_thread;
mach_vm_address_t gadget_address;

vm_address_t remote_stack;

thread_t thread;
pthread_t pthread;

arm_thread_state64_t state = {0};

mach_msg_type_number_t stateCount = ARM_THREAD_STATE64_COUNT;

static uint64_t ropcall(mach_vm_address_t function, char *argMap, uint64_t *arg1, uint64_t *arg2, uint64_t *arg3, uint64_t *arg4)
{
	kern_return_t kret;

	state.__opaque_pc = (void*) ptrauth_sign_unauthenticated((void*) function, ptrauth_key_process_independent_code, ptrauth_string_discriminator("pc"));

	thread_convert_thread_state(thread, THREAD_CONVERT_THREAD_STATE_FROM_SELF, ARM_THREAD_STATE64, reinterpret_cast<thread_state_t>(&state), stateCount, reinterpret_cast<thread_state_t>(&state), &stateCount);

	state.__opaque_lr = (void*) gadget_address;
	state.__opaque_sp = (void*) (uint64_t) ((remote_stack + STACK_SIZE) - (STACK_SIZE / 4));
	state.__opaque_fp = (void*) state.__opaque_sp;

	char *local_fake_stack = (char *)malloc((size_t) STACK_SIZE);

	char *argp = (char *)argMap;

	char *stack_ptr = local_fake_stack;

	uint64_t paramLen = 0;

	for(int param = 0; param <= 4; param++)
	{
		if(!(*argp))
			break;

		switch(*argp)
		{
			case 's':
				;

				int num_digits;

				char tmp_buf[6];

				argp++;

				num_digits = 0;

				while(*argp >= '0' && *argp <= '9')
				{
					if(++num_digits == 6)
					{
						fprintf(stderr, "String too long, param=%d\n", param);

						return 0;
					}

					tmp_buf[num_digits-1] = *(argp++);
				}

				tmp_buf[num_digits] = 0;

				paramLen = strtoull(tmp_buf, NULL, 10);

				uint64_t *argPtr;

				if(param==0)
					argPtr = arg1;
				if(param==1)
					argPtr = arg2;
				if(param==2)
					argPtr = arg3;
				if(param==3)
					argPtr = arg4;

				memcpy(stack_ptr, argPtr, paramLen);

				state.__x[param] = (uint64_t)remote_stack + (stack_ptr - local_fake_stack);
				stack_ptr += 16;
				stack_ptr += paramLen;
				stack_ptr = (char *)align64((uint64_t)stack_ptr);

				break;

			case 'u':
				;

				state.__x[param] = (param==0) ? (uint64_t)arg1: (param==1) ? (uint64_t)arg2 : (param==2) ? (uint64_t)arg3 : (uint64_t)arg4;
				
				argp++;
				
				break;

			default:
				;

				fprintf(stderr, "Unknown argument type: '%c'\n", *argp);
				
				return 0;
		}
	}

	kret = vm_write(task->getTaskPort(), remote_stack, (vm_address_t)local_fake_stack, STACK_SIZE);
	
	free(local_fake_stack);

	if(kret != KERN_SUCCESS)
	{
		fprintf(stderr, "Unable to copy fake stack to target process! %s\n", mach_error_string(kret));

		return 0;
	}

	printf("Calling function at %p...\n", (void *)function);

	thread_set_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, ARM_THREAD_STATE64_COUNT);

	thread_resume(thread);

	while(1)
	{
		usleep(250000);

		thread_get_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, &stateCount);

		if(state.__opaque_pc == (void*)gadget_address)
		{
			printf("Returned from function!\n");

			thread_suspend(thread);

			break;
		}
	}

	return (uint64_t)state.__x[0];
}

static void* find_gadget(const char *gadget, int gadget_len)
{
	kern_return_t kr;

	vm_size_t size = 0;

	size = 65536;

	char *buf = (char*) malloc(size);

	char *orig_buf = buf;

	if(!buf)
	{
		fprintf(stderr, "Error allocating memory!\n");

		return NULL;
	}

	kr = vm_read_overwrite(task->getTaskPort(), dlopen, size, (vm_address_t)buf, &size);

	if(kr != KERN_SUCCESS)
	{
		fprintf(stderr, "Could not read RX pages!\n");

		free(orig_buf);

		return NULL;
	}

	while(buf < orig_buf + size)
	{
		char *ptr = (char *) memmem((const void *)buf, (size_t)size - (size_t)(buf - orig_buf), (const void *)gadget, (size_t)gadget_len);
	
		if(ptr)
		{
			vm_size_t offset = (vm_size_t) (ptr - orig_buf);

			vm_address_t gadget_addr_real = dlopen + offset;
			
			if(((uint64_t)gadget_addr_real % 8) == 0)
			{
				free(orig_buf);

				return (void *)gadget_addr_real;
			}
			else
			{
				buf = ptr + gadget_len;
			}
		}
	}

	free(orig_buf);

	return NULL;
}

int main()
{
	kern_return_t kr;

	kernel = new Kernel();

	printf("Kernel base = 0x%llx slide = 0x%llx\n", kernel->getBase(), kernel->getSlide());

	task = new Task(kernel, "Twitter");

	printf("PID 1382 task = 0x%llx proc = 0x%llx\n", task->getTask(), task->getProc());

	libDyld = task->getDyld()->cacheDumpImage("libdyld.dylib");

	dlopen = libDyld->getSymbolAddressByName("_dlopen") + libDyld->getAslrSlide();

	libSystemPthread = task->getDyld()->cacheDumpImage("libsystem_pthread.dylib");

	pthread_create_from_mach_thread = libSystemPthread->getSymbolAddressByName("_pthread_create_from_mach_thread") + libSystemPthread->getAslrSlide();

	printf("dlopen = 0x%llx\n", dlopen);

	printf("pthread_create_from_mach_thread = 0x%llx\n", pthread_create_from_mach_thread);

	if((kr = thread_create(task->getTaskPort(), &thread)) != KERN_SUCCESS)
	{
		fprintf(stderr, "Could not create new thread in task!\n");

		return -1;
	}

	gadget_address = reinterpret_cast<mach_vm_address_t>(find_gadget(ROP_ret, 4));

	if(!gadget_address)
	{
		fprintf(stderr, "Failed to find gadget address!\n");

		return 0;
	}

	fprintf(stdout, "Found gadget at 0x%llx\n", gadget_address);

	kr = vm_allocate(task->getTaskPort(), &remote_stack, STACK_SIZE, VM_FLAGS_ANYWHERE);

	if (kr != KERN_SUCCESS)
	{
		printf("Unable to allocate memory! %s\n", mach_error_string(kr));
		
		return -1;
	}

	kr = vm_protect(task->getTaskPort(), remote_stack, STACK_SIZE, FALSE, VM_PROT_READ| VM_PROT_WRITE);
	
	if(kr != KERN_SUCCESS)
	{
		printf("Unable to protect memory! %s\n", mach_error_string(kr));
		
		return -1;
	}

	mach_vm_address_t pthread;

	kr = vm_allocate(task->getTaskPort(), (vm_address_t*) &pthread, 16192, VM_FLAGS_ANYWHERE);

	if (kr != KERN_SUCCESS)
	{
		printf("Unable to allocate memory! %s\n", mach_error_string(kr));
		
		return -1;
	}

	kr = vm_protect(task->getTaskPort(), pthread, 16192, FALSE, VM_PROT_READ| VM_PROT_WRITE);
	
	if(kr != KERN_SUCCESS)
	{
		printf("Unable to protect memory! %s\n", mach_error_string(kr));
		
		return -1;
	}

	printf("Pthread arguments = 0x%llx\n", pthread);

	ropcall(pthread_create_from_mach_thread, (char *)"u", (uint64_t*) pthread, NULL, (uint64_t*) pthread + 256, NULL );

	// dlopen("libcycript.dylib", RTLD_NOW | RTLD_GLOBAL, 0, 0);

	thread_get_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, &stateCount);

	thread_suspend(thread);

	thread_terminate(thread);

	vm_deallocate(task->getTaskPort(), remote_stack, STACK_SIZE);

	delete task;

	delete kernel;

	return 0;
}