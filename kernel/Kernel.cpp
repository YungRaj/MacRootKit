#include "Kernel.hpp"

#include "Log.hpp"

#include "IOKernelRootKitService.hpp"
#include "MacRootKit.hpp"

#include "KernelMachO.hpp"

extern "C"
{
	#include "kern.h"
}

using namespace XNU;

off_t Kernel::tempExecutableMemoryOffset = 0;

uint8_t Kernel::tempExecutableMemory[tempExecutableMemorySize] __attribute__((section("__TEXT,__text")));

Kernel::Kernel(mach_port_t kernel_task_port)
{
	this->kernel_task_port = kernel_task_port;
	this->base = Kernel::findKernelBase();
	this->disassembler = new Disassembler(this);
	this->kernelWriteLock = IOSimpleLockAlloc();
}

Kernel::Kernel(mach_vm_address_t cache, mach_vm_address_t base, off_t slide)
{
	this->macho = new KernelMachO(this);

	this->macho->initWithBase(base, slide);

	// this->getKernelObjects();

	this->disassembler = new Disassembler(this);
}

Kernel::Kernel(mach_vm_address_t base, off_t slide)
{
	this->macho = new KernelMachO(this);

	this->macho->initWithBase(base, slide);

	// this->getKernelObjects();

	this->disassembler = new Disassembler(this);

	/*
	this->createKernelTaskPort();

	this->kernelWriteLock = IOSimpleLockAlloc();

	this->base = base;

	set_kernel_map(this->getKernelMap());

	set_vm_functions(this->getSymbolAddressByName("_vm_read_overwrite"),
					 this->getSymbolAddressByName("_vm_write"),
					 this->getSymbolAddressByName("_vm_protect"),
					 this->getSymbolAddressByName("_vm_remap"),
					 this->getSymbolAddressByName("_vm_allocate"),
					 this->getSymbolAddressByName("_vm_deallocate"),
					 this->getSymbolAddressByName("_vm_map_copyin"),
					 this->getSymbolAddressByName("_vm_map_copy_overwrite")
	);

	set_phys_functions(this->getSymbolAddressByName("_pmap_find_phys"),
					   this->getSymbolAddressByName("_ml_phys_read_double_64"),
					   this->getSymbolAddressByName("_ml_phys_read_word_64"),
					   this->getSymbolAddressByName("_ml_phys_read_half_64"),
					   this->getSymbolAddressByName("_ml_phys_read_byte_64"),
					   this->getSymbolAddressByName("_ml_phys_write_double_64"),
					   this->getSymbolAddressByName("_ml_phys_write_word_64"),
					   this->getSymbolAddressByName("_ml_phys_write_half_64"),
					   this->getSymbolAddressByName("_ml_phys_write_byte_64")
	);
	*/
}

Kernel::~Kernel()
{
}

/*

KernelCache slide: 0x000000000bd54000\n
KernelCache base:  0xfffffe0012d58000\n
Kernel slide:      0x000000000c580000\n
Kernel text base:  0xfffffe0013584000\n
Kernel text exec slide: 0x000000000c668000\n
Kernel text exec base:  0xfffffe001366c000

*/

mach_vm_address_t Kernel::findKernelCache()
{
	static mach_vm_address_t kernel_cache = 0;

	if(kernel_cache)
		return kernel_cache;

	mach_vm_address_t near = 0xfffffe0000000000 | *reinterpret_cast<mach_vm_address_t*>(IOLog);

	size_t kaslr_align = 0x4000;

	near &= ~(kaslr_align - 1);

	while(true)
	{
		struct mach_header_64 *mh = reinterpret_cast<struct mach_header_64*>(near);

		if(mh->magic == MH_MAGIC_64)
		{
			if(mh->filetype == 0xC && mh->flags == 0 && mh->reserved == 0)
			{
				break;
			}
		}

		near -= kaslr_align;
	}

	kernel_cache = near;

	return kernel_cache;
}

mach_vm_address_t Kernel::findKernelCollection()
{
	static mach_vm_address_t kernel_collection = 0;

	if(kernel_collection)
		return kernel_collection;

	mach_vm_address_t near = reinterpret_cast<mach_vm_address_t>(IOLog);

	size_t kaslr_align = 0x100000;

	near &= ~(kaslr_align - 1);

	while(true)
	{
		struct mach_header_64 *mh = reinterpret_cast<struct mach_header_64*>(near);

		if(mh->magic == MH_MAGIC_64)
		{
			if(mh->filetype == 0xC && mh->flags == 0 && mh->reserved == 0)
			{
				break;
			}
		}

		near -= kaslr_align;
	}

	kernel_collection = near;

	return kernel_collection;
}

mach_vm_address_t Kernel::findKernelBase()
{
	static mach_vm_address_t kernel_base = 0;

	if(kernel_base)
		return kernel_base;

	mach_vm_address_t kc;

#ifdef __arm64__

	kc = Kernel::findKernelCache();

	struct mach_header_64 *mh = reinterpret_cast<struct mach_header_64*>(kc);

	uint8_t *q = reinterpret_cast<uint8_t*>(kc) + sizeof(struct mach_header_64);

	for(uint32_t i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>(q);

		if(load_command->cmd == LC_FILESET_ENTRY)
		{
			struct fileset_entry_command *fileset_entry_command = reinterpret_cast<struct fileset_entry_command*>(load_command);

			char *entry_id = reinterpret_cast<char*>(fileset_entry_command) + fileset_entry_command->entry_id;

			if(strcmp(entry_id, "com.apple.kernel") == 0)
			{
				kernel_base = 0xfffffe0000000000 | fileset_entry_command->vmaddr;

				break;
			}
		}

		q += load_command->cmdsize;
	}


#endif

#ifdef __x86_64__

	kc = Kernel::findKernelCollection();

	struct mach_header_64 *mh = reinterpret_cast<struct mach_header_64*>(kc);

	uint8_t *q = reinterpret_cast<uint8_t*>(kc) + sizeof(struct mach_header_64);

	for(uint32_t i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>(q);

		if(load_command->cmd == LC_SEGMENT_64)
		{
			struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

			if(strncmp(segment_command->segname, "__PRELINK_TEXT", strlen("__PRELINK_TEXT")) == 0)
			{
				kernel_base = segment_command->vmaddr;

				break;
			}
		}

		q += load_command->cmdsize;
	}

#endif

	return kernel_base;
}

off_t Kernel::findKernelSlide()
{
	mach_vm_address_t base;

	mach_vm_address_t text_base;

	struct mach_header_64 *mh;

	base = Kernel::findKernelBase();

#ifdef __arm64__

	return base - 0xfffffe0007004000;

#endif

#ifdef __x86_64__

	mh = reinterpret_cast<struct mach_header_64*>(base);

	uint8_t *q = reinterpret_cast<uint8_t*>(base) + sizeof(struct mach_header_64);

	for(uint32_t i = 0; i < mh->ncmds; i++)
	{
		struct load_command *load_command = reinterpret_cast<struct load_command*>(q);

		if(load_command->cmd == LC_SEGMENT_64)
		{
			struct segment_command_64 *segment_command = reinterpret_cast<struct segment_command_64*>(load_command);

			if(strncmp(segment_command->segname, "__TEXT", strlen("__TEXT")) == 0)
			{
				text_base = segment_command->vmaddr;

				break;
			}
		}

		q += load_command->cmdsize;
	}

	return text_base - 0xfffffe0007004000;
	
#endif
}

mach_vm_address_t Kernel::getBase()
{
	this->base = Kernel::findKernelBase();

	return base;
}

off_t Kernel::getSlide()
{
	if(this->slide)
	{
		return slide;
	}

	this->slide = Kernel::findKernelSlide();

	return this->slide;
}


void Kernel::getKernelObjects()
{
	char buffer1[128];
	char buffer2[128];
	char buffer3[128];

	task_t _kernel_task = *reinterpret_cast<task_t*> (this->getSymbolAddressByName("_kernel_task"));

	this->task = _kernel_task;

	typedef vm_map_t (*get_task_map) (task_t task);
	vm_map_t (*_get_task_map)(task_t);

	_get_task_map = reinterpret_cast<get_task_map> (this->getSymbolAddressByName("_get_task_map"));

	this->map = _get_task_map(_kernel_task);

	typedef pmap_t (*get_task_pmap) (task_t task);
	pmap_t (*_get_task_pmap)(task_t);

	_get_task_pmap = reinterpret_cast<get_task_pmap> (this->getSymbolAddressByName("_get_task_pmap"));

	this->pmap = _get_task_pmap(_kernel_task);
	
	snprintf(buffer1, 128, "0x%llx", (mach_vm_address_t) _get_task_map);
	snprintf(buffer2, 128, "0x%llx", (mach_vm_address_t)  _get_task_pmap);

	MAC_RK_LOG("MacPE::get_task_map = %s get_task_pmap = %s\n", buffer1, buffer2);

	snprintf(buffer1, 128, "0x%llx", (mach_vm_address_t) this->getKernelTask());
	snprintf(buffer2, 128, "0x%llx", (mach_vm_address_t) this->getKernelMap());
	snprintf(buffer3, 128, "0x%llx", (mach_vm_address_t) this->getKernelPmap());

	MAC_RK_LOG("MacPE::kernel_task = %s kernel_map = %s kernel_pmap = %s!\n", buffer1, buffer2, buffer3);
}

void Kernel::createKernelTaskPort()
{
	typedef vm_offset_t ipc_kobject_t;
	typedef natural_t ipc_kobject_type_t;

	typedef void (*ipc_kobject_set) (ipc_port_t port, ipc_kobject_t kobject, ipc_kobject_type_t type);
	void (*_ipc_kobject_set)(ipc_port_t port, ipc_kobject_t kobject, ipc_kobject_type_t type) = 0;

	typedef ipc_port_t (*ipc_port_alloc_special) (ipc_space_t);
	ipc_port_t (*_ipc_port_alloc_special)(ipc_space_t space) = 0;

	typedef void (*ipc_port_dealloc_special) (ipc_port_t port, ipc_space_t space);
	void (*_ipc_port_dealloc_special)(ipc_port_t port, ipc_space_t space) = 0;

	typedef ipc_port_t (*ipc_port_make_send) (ipc_port_t port);
	ipc_port_t (*_ipc_port_make_send)(ipc_port_t port) = 0;

	_ipc_kobject_set = reinterpret_cast<ipc_kobject_set> (this->getSymbolAddressByName("_ipc_kobject_set"));
	_ipc_port_alloc_special = reinterpret_cast<ipc_port_alloc_special> (this->getSymbolAddressByName("_ipc_port_alloc_special"));
	_ipc_port_dealloc_special = reinterpret_cast<ipc_port_dealloc_special> (this->getSymbolAddressByName("_ipc_port_dealloc_special"));
	_ipc_port_make_send = reinterpret_cast<ipc_port_make_send> (this->getSymbolAddressByName("_ipc_port_make_send"));

	task_t _kernel_task = *reinterpret_cast<task_t*> (this->getSymbolAddressByName("_kernel_task"));
	ipc_space_t _ipc_space_kernel = *reinterpret_cast<ipc_space_t*> (this->getSymbolAddressByName("_ipc_space_kernel"));

	char buffer1[128];
	char buffer2[128];
	char buffer3[128];

	snprintf(buffer1, 128, "0x%llx", (mach_vm_address_t) _ipc_kobject_set);
	snprintf(buffer2, 128, "0x%llx", (mach_vm_address_t) _ipc_port_alloc_special);
	snprintf(buffer3, 128, "0x%llx", (mach_vm_address_t)  _ipc_port_make_send);

	MAC_RK_LOG("MacPE::ipc_kobject_set = %s ipc_port_alloc_special = %s ipc_port_make_send = %s\n", buffer1, buffer2, buffer3);

	snprintf(buffer1, 128, "0x%llx", (mach_vm_address_t) _kernel_task);
	snprintf(buffer2, 128, "0x%llx", (mach_vm_address_t) _ipc_space_kernel);

	MAC_RK_LOG("MacPE::kernel_task = %s ipc_space_kernel = %s\n", buffer1, buffer2);

	// use the host_priv to set host special port 4
	host_priv_t host = host_priv_self();

	if(!host) return;

	ipc_port_t port = _ipc_port_alloc_special(_ipc_space_kernel);

	if(!port) return;

	#define IKOT_TASK 2

	#define IO_BITS_KOTYPE          0x000003ff      /* used by the object */
	#define IO_BITS_KOBJECT         0x00000800      /* port belongs to a kobject */

	uint8_t *port_buf = reinterpret_cast<uint8_t*>(port);

	// set io_bits of ipc_object
	*(uint32_t*)(port_buf)			 = (*(uint32_t*) port_buf & ~IO_BITS_KOTYPE);
	*(uint32_t*)(port_buf)			|= IKOT_TASK;
	*(uint32_t*)(port_buf)			|= IO_BITS_KOBJECT;

	// set kobject of ipc_port
	*(uint64_t*)(port_buf + 0x68) 	 = (uint64_t) kernel_task;

	this->kernel_task_port = _ipc_port_make_send(port);
}

bool Kernel::setKernelWriting(bool enable)
{
	static bool interruptsDisabled = false;

	Architecture *architecture = this->getRootKit()->getArchitecture();

	kern_return_t result = KERN_SUCCESS;

	if(enable)
	{
		interruptsDisabled = !setInterrupts(false);
	}

	if(architecture->setWPBit(!enable) != KERN_SUCCESS)
	{
		enable = false;

		result = KERN_FAILURE;
	}

	if(!enable)
	{
		if(!interruptsDisabled)
		{
			interruptsDisabled = setInterrupts(true);
		}
	}

	return interruptsDisabled;
}

bool Kernel::setNXBit(bool enable)
{
	return false;
}

bool Kernel::setInterrupts(bool enable)
{
	Architecture *architecture = this->getRootKit()->getArchitecture();

	return architecture->setInterrupts(enable);
}

uint64_t Kernel::call(char *symbolname, uint64_t *arguments, size_t argCount)
{
	mach_vm_address_t func = this->getSymbolAddressByName(symbolname);

	return this->call(func, arguments, argCount);
}

uint64_t Kernel::call(mach_vm_address_t func, uint64_t *arguments, size_t argCount)
{
	uint64_t ret = 0;

	mach_vm_address_t function = func;

#ifdef __arm64__

	__asm__ volatile("PACIZA %[pac]" : [pac] "+rm" (function));

#endif

	switch(argCount)
	{
		case 0:
		{
			typedef uint64_t (*function0)(void);

			function0 funk = reinterpret_cast<function0>(function);

			ret = (uint64_t)(*funk)();

			break;
		}

		case 1:
		{
			typedef uint64_t (*function1)(uint64_t);

			function1 funk = reinterpret_cast<function1>(function);

			ret = (uint64_t)(*funk)(arguments[0]);

			break;
		}

		case 2:
		{
			typedef uint64_t (*function2)(uint64_t, uint64_t);

			function2 funk = reinterpret_cast<function2>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1]);

			break;
		}

		case 3:
		{
			typedef uint64_t (*function3)(uint64_t, uint64_t, uint64_t);

			function3 funk = reinterpret_cast<function3>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2]);

			break;
		}

		case 4:
		{
			typedef uint64_t (*function4)(uint64_t, uint64_t, uint64_t, uint64_t);

			function4 funk = reinterpret_cast<function4>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3]);

			break;
		}

		case 5:
		{
			typedef uint64_t (*function5)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function5 funk = reinterpret_cast<function5>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);

			break;
		}

		case 6:
		{
			typedef uint64_t (*function6)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function6 funk = reinterpret_cast<function6>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);

			break;
		}

		case 7:
		{
			typedef uint64_t (*function7)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function7 funk = reinterpret_cast<function7>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);

			break;
		}

		case 8:
		{
			typedef uint64_t (*function8)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function8 funk = reinterpret_cast<function8>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);

			break;
		}

		case 9:
		{
			typedef uint64_t (*function9)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function9 funk = reinterpret_cast<function9>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);

			break;
		}

		case 10:
		{
			typedef uint64_t (*function10)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function10 funk = reinterpret_cast<function10>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9]);

			break;
		}

		case 11:
		{
			typedef uint64_t (*function11)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function11 funk = reinterpret_cast<function11>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10]);

			break;
		}

		case 12:
		{
			typedef uint64_t (*function12)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function12 funk = reinterpret_cast<function12>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11]);

			break;
		}

		case 13:
		{
			typedef uint64_t (*function13)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function13 funk = reinterpret_cast<function13>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12]);
			
			break;
		}

		case 14:
		{
			typedef uint64_t (*function14)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function14 funk = reinterpret_cast<function14>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13]);

			break;
		}

		case 15:
		{
			typedef uint64_t (*function15)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function15 funk = reinterpret_cast<function15>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);

			break;
		}

		case 16:
		{
			typedef uint64_t (*function16)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function16 funk = reinterpret_cast<function16>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15]);

			break;
		}

		case 17:
		{
			typedef uint64_t (*function17)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function17 funk = reinterpret_cast<function17>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16]);

			break;
		}

		case 18:
		{
			typedef uint64_t (*function18)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function18 funk = reinterpret_cast<function18>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17]);

			break;
		}

		case 19:
		{
			typedef uint64_t (*function19)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function19 funk = reinterpret_cast<function19>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18]);

			break;
		}

		case 20:
		{
			typedef uint64_t (*function20)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

			function20 funk = reinterpret_cast<function20>(function);

			ret = (uint64_t)(*funk)(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14], arguments[15], arguments[16], arguments[17], arguments[18], arguments[19]);

			break;
		}

		default:
			break;
	}

	return ret;
}

mach_vm_address_t Kernel::vmAllocate(size_t size)
{
	return kernel_vm_allocate(size);
}

#define VM_KERN_MEMORY_KEXT            6
#define VM_KERN_MEMORY_ANY             255

#define VM_INHERIT_SHARE               ((vm_inherit_t) 0)      /* shared with child */
#define VM_INHERIT_COPY                ((vm_inherit_t) 1)      /* copy into child */
#define VM_INHERIT_NONE                ((vm_inherit_t) 2)      /* absent from child */
#define VM_INHERIT_DONATE_COPY         ((vm_inherit_t) 3)      /* copy and delete */

#define VM_INHERIT_DEFAULT             VM_INHERIT_COPY

mach_vm_address_t Kernel::vmAllocate(size_t size, uint32_t flags, vm_prot_t prot)
{
	kern_return_t ret;

	mach_vm_address_t address = 0;

	mach_vm_address_t map;

#ifdef __x86_64__

	address = 0x1000;

	map = this->read64(this->getSymbolAddressByName("_g_kext_map"));

	uint64_t vmEnterArgs[13] = { map, (uint64_t) &address, size, 0, flags, 0, VM_KERN_MEMORY_KEXT, 0, 0, FALSE, (uint64_t) prot, (uint64_t) prot, (uint64_t) VM_INHERIT_DEFAULT };

	ret = static_cast<kern_return_t>(this->call("_vm_map_enter", vmEnterArgs, 13));

#elif __arm64__

#include <arm64/Isa_arm64.hpp>

	using namespace Arch::arm64;

	char buffer[128];

	/*
	mach_vm_address_t vm_allocate_external = this->getSymbolAddressByName("_vm_allocate_external");

	mach_vm_address_t branch = Arch::arm64::PatchFinder::step64(this->macho, vm_allocate_external, 0x10, reinterpret_cast<bool(*)(uint32_t*)>(Arch::arm64::is_b), -1, -1);

	bool sign;

	b_t b = *(b_t*) branch;

	uint64_t imm = b.imm;

	if(imm & 0x2000000)
	{
		imm = ~(imm - 1);
		imm &= 0x1FFFFFF;

		sign = true;
	} else
	{
		sign = false;
	}

	imm *= (1 << 2);

	mach_vm_address_t vm_allocate = sign ? branch - imm : branch + imm;

	branch = Arch::arm64::PatchFinder::step64(this->macho, vm_allocate, 0x100, reinterpret_cast<bool(*)(uint32_t*)>(Arch::arm64::is_bl), -1, -1);

	bl_t bl = *(bl_t*) branch;

	imm = bl.imm;

	if(imm & 0x2000000)
	{
		imm = ~(imm - 1);
		imm &= 0x1FFFFFF;

		sign = true;
	} else
	{
		sign = false;
	}

	imm *= (1 << 2);

	mach_vm_address_t vm_map_enter = sign ? branch - imm : branch + imm;

	map = *reinterpret_cast<mach_vm_address_t*>(this->getSymbolAddressByName("_kernel_map"));

	address = *(uint64_t*) (map + 32);

	uint64_t vmEnterArgs[13] = { map, (uint64_t) &address, size, 0, flags, 0, 1, 0, 0, FALSE, (uint64_t) prot, (uint64_t) VM_PROT_ALL, (uint64_t) VM_INHERIT_DEFAULT };
	
	ret = static_cast<kern_return_t>(this->call(vm_map_enter, vmEnterArgs, 13));
	*/

	map = *reinterpret_cast<mach_vm_address_t*>(this->getSymbolAddressByName("_kernel_map"));

	uint64_t vmAllocateExternalArgs[4] = { map, (uint64_t) &address, size, VM_FLAGS_ANYWHERE };

	ret = static_cast<kern_return_t>(this->call("_vm_allocate_external", vmAllocateExternalArgs, 4));

	/*
	mach_vm_address_t vm_map_enter_mem_object_helper_strref = Arch::arm64::PatchFinder::findStringReference(macho, "VM_FLAGS_RETURN_DATA_ADDR not expected for submap. @%s:%d", 1, __cstring_, __TEXT_XNU_BASE, false);

	mach_vm_address_t vm_map_enter_mem_object_helper = Arch::arm64::PatchFinder::findFunctionBegin(macho, vm_map_enter_mem_object_helper_strref - 0x2FFF, vm_map_enter_mem_object_helper_strref);

	uint64_t vmMapEnterMemObjectHelperArgs[15] = { map, (uint64_t) &address, size, 0, VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE, 0, 0, 0, 0, FALSE, VM_PROT_ALL, VM_PROT_ALL, VM_INHERIT_DEFAULT, 0, 0 };

	ret = static_cast<kern_return_t>(this->call(vm_map_enter_mem_object_helper, vmMapEnterMemObjectHelperArgs, 15)); */

#endif

	if(ret != KERN_SUCCESS)
	{
		address = 0;
	}

	snprintf(buffer, 128, "0x%llx", address);

	MAC_RK_LOG("MacRK::vm_map_enter() return address = %s\n", buffer);

	return address;
}

void Kernel::vmDeallocate(mach_vm_address_t address, size_t size)
{
	kernel_vm_deallocate(address, size);
}

bool Kernel::vmProtect(mach_vm_address_t address, size_t size, vm_prot_t prot)
{
	kern_return_t ret;

#ifdef __x86_64__

	uint64_t mlStaticProtectArgs[3] = { address, size, prot };

	ret = static_cast<kern_return_t>(this->call("_ml_static_protect", mlStaticProtectArgs, 3));

#elif __arm64__

	mach_vm_address_t ml_static_protect;

	mach_vm_address_t ml_static_protect_strref = Arch::arm64::PatchFinder::findStringReference(macho, "ml_static_protect(): attempt to inject executable mapping on %p @%s:%d", 1, __cstring_, __TEXT_XNU_BASE, false);

	static bool patched = false;

	if(!patched)
	{
		uint32_t nop = 0xd503201f;

		mach_vm_address_t panic = Arch::arm64::PatchFinder::stepBack64(macho, ml_static_protect_strref, 0x20, reinterpret_cast<bool(*)(uint32_t*)>(Arch::arm64::is_movz), -1, -1);

		mach_vm_address_t panic_xref = Arch::arm64::PatchFinder::xref64(macho, panic - 0xFFF, panic, panic);

		char buffer[128];

		snprintf(buffer, 128, "0x%llx", panic_xref);

		MAC_RK_LOG("MacRK::panic_xref = %s\n", buffer);

		if(panic_xref)
		{
			*(uint32_t*) panic_xref = nop;

			patched = true;
		}
	}

	ml_static_protect = Arch::arm64::PatchFinder::findFunctionBegin(macho, ml_static_protect_strref - 0x2FF, ml_static_protect_strref);

	uint64_t mlStaticProtectArgs[3] = { address, size, (uint64_t) prot };

	ml_set_interrupts_enabled(false);

	ret = static_cast<kern_return_t>(this->call(ml_static_protect, mlStaticProtectArgs, 3));

	ml_set_interrupts_enabled(true);

	if(ret != KERN_SUCCESS)
	{
		MAC_RK_LOG("MacRK::ml_static_protect failed! error = %d\n", ret);

		return false;
	}

#endif

	MAC_RK_LOG("MacRK::ml_static_protect success!\n");

	return ret == KERN_SUCCESS;
}

void* Kernel::vmRemap(mach_vm_address_t address, size_t size)
{

#ifdef __arm64__

#include <arm64/Isa_arm64.hpp>

	using namespace Arch::arm64;

	kern_return_t ret;

	char buffer[128];

	mach_vm_address_t map;

	mach_vm_address_t addr = 0;

	vm_prot_t cur_protection = VM_PROT_ALL;
	vm_prot_t max_protection = VM_PROT_ALL;

	mach_vm_address_t vm_map_remap;

	mach_vm_address_t vm_map_protect_strref = Arch::arm64::PatchFinder::findStringReference(this->macho, "vm_map_protect(%p,0x%llx,0x%llx) new=0x%x wired=%x @%s:%d", 1, __cstring_, __TEXT_XNU_BASE, false);

	mach_vm_address_t cbz = Arch::arm64::PatchFinder::stepBack64(macho, vm_map_protect_strref, 0xFFF, reinterpret_cast<bool(*)(uint32_t*)>(is_cbz), -1, -1);

	cbz = Arch::arm64::PatchFinder::stepBack64(macho, cbz - sizeof(uint32_t), 0xFFF, reinterpret_cast<bool(*)(uint32_t*)>(is_cbz), -1, -1);

	mach_vm_address_t branch = Arch::arm64::PatchFinder::stepBack64(macho, cbz, 0x10, reinterpret_cast<bool(*)(uint32_t*)>(is_bl), -1, -1);

	bool sign;

	bl_t bl = *(bl_t*) branch;

	uint64_t imm = bl.imm;

	if(imm & 0x2000000)
	{
		imm = ~(imm - 1);
		imm &= 0x1FFFFFF;

		sign = true;
	} else
	{
		sign = false;
	}

	imm *= (1 << 2);

	vm_map_remap = sign ? branch - imm : branch + imm;

	map = *reinterpret_cast<mach_vm_address_t*>(this->getSymbolAddressByName("_kernel_map"));

	uint64_t vmMapRemapArgs[13] = { map, (uint64_t) &addr, size, 0, VM_FLAGS_ANYWHERE, 0, 0, map, address, false, (uint64_t) &cur_protection, (uint64_t) &max_protection, (uint64_t) VM_INHERIT_DEFAULT };

	ret = static_cast<kern_return_t>(this->call(vm_map_remap, vmMapRemapArgs, 13));

	if(ret != KERN_SUCCESS)
	{
		MAC_RK_LOG("MacRK::vm_map_remap failed!\n");

		return 0;
	}

	return reinterpret_cast<void*>(addr);
	
#endif


	return kernel_vm_remap(address, size);
}

uint64_t Kernel::virtualToPhysical(mach_vm_address_t address)
{
	return kernel_virtual_to_physical(address);
}

bool Kernel::physicalRead(uint64_t paddr, void *data, size_t size)
{
	const uint8_t *read_data = reinterpret_cast<uint8_t*>(data);

	while(size > 0)
	{
		size_t read_size = size;

		if(read_size >= 8)
			read_size = 8;
		else if(read_size >= 4)
			read_size = 4;
		else if(read_size >= 2)
			read_size = 2;
		else if(read_size >= 1)
			read_size = 1;

		if(read_size == 8)
			*(uint64_t*) read_data = physical_read64(paddr);
		if(read_size == 4)
			*(uint32_t*) read_data = physical_read32(paddr);
		if(read_size == 2)
			*(uint16_t*) read_data = physical_read16(paddr);
		if(read_size == 1)
			*(uint8_t*) read_data = physical_read8(paddr);

		paddr += read_size;
		read_data += read_size;
		size -= read_size;
	}

	return true;
}

uint64_t Kernel::physicalRead64(uint64_t paddr)
{
	return physical_read64(paddr);
}

uint32_t Kernel::physicalRead32(uint64_t paddr)
{
	return physical_read32(paddr);
}

uint16_t Kernel::physicalRead16(uint64_t paddr)
{
	return physical_read16(paddr);
}

uint8_t Kernel::physicalRead8(uint64_t paddr)
{
	return physical_read8(paddr);
}

bool Kernel::physicalWrite(uint64_t paddr, void *data, size_t size)
{
	const uint8_t *write_data = reinterpret_cast<uint8_t*>(data);

	while(size > 0)
	{
		size_t write_size = size;

		if(write_size >= 8)
			write_size = 8;
		else if(write_size >= 4)
			write_size = 4;
		else if(write_size >= 2)
			write_size = 2;
		else if(write_size >= 1)
			write_size = 1;

		if(write_size == 8)
			 physical_write64(paddr, *(uint64_t*) write_data);
		if(write_size == 4)
			physical_write32(paddr, *(uint32_t*) write_data);
		if(write_size == 2)
			physical_write16(paddr, *(uint16_t*) write_data);
		if(write_size == 1)
			physical_write8(paddr, *(uint8_t*) write_data);

		paddr += write_size;
		write_data += write_size;
		size -= write_size;
	}

	return true;
}

void Kernel::physicalWrite64(uint64_t paddr, uint64_t value)
{
	return physical_write64(paddr, value);
}

void Kernel::physicalWrite32(uint64_t paddr, uint32_t value)
{
	return physical_write32(paddr, value);
}

void Kernel::physicalWrite16(uint64_t paddr, uint16_t value)
{
	return physical_write16(paddr, value);
}

void Kernel::physicalWrite8(uint64_t paddr, uint8_t value)
{
	return physical_write8(paddr, value);
}

bool Kernel::read(mach_vm_address_t address, void *data, size_t size)
{
	ml_set_interrupts_enabled(false);

	for(int i = 0; i < size; i++)
		*((uint8_t*) data + i) = *((uint8_t*) address + i);

	ml_set_interrupts_enabled(true);

	// return kernel_read(address, data, size);

	return true;
}

bool Kernel::readUnsafe(mach_vm_address_t address, void *data, size_t size)
{
	return kernel_read_unsafe(address, data, size);
}

uint8_t Kernel::read8(mach_vm_address_t address)
{
	uint8_t value;

	kernel_read(address, reinterpret_cast<void*>(&value), sizeof(value));

	return value;
}

uint16_t Kernel::read16(mach_vm_address_t address)
{
	uint16_t value;

	kernel_read(address, reinterpret_cast<void*>(&value), sizeof(value));

	return value;
}

uint32_t Kernel::read32(mach_vm_address_t address)
{
	uint32_t value;

	bool success = kernel_read(address, reinterpret_cast<void*>(&value), sizeof(value));

	if(!success)
		return 0;

	return value;
}

uint64_t Kernel::read64(mach_vm_address_t address)
{
	uint64_t value;

	kernel_read(address, reinterpret_cast<void*>(&value), sizeof(value));

	return value;
}

bool Kernel::write(mach_vm_address_t address, void *data, size_t size)
{
	mach_vm_address_t pmap = *reinterpret_cast<mach_vm_address_t*>(this->getSymbolAddressByName("_kernel_pmap"));

	uint64_t src_pmapFindPhysArgs[2] = { pmap, (uint64_t) data };

	uint64_t dest_pmapFindPhysArgs[2] = { pmap, address };

	uint64_t src_ppnum;
	uint64_t src_pa;

	uint64_t dest_ppnum;
	uint64_t dest_pa;

	src_ppnum = this->call("_pmap_find_phys", src_pmapFindPhysArgs, 2);

	src_pa = ((src_ppnum << 14) | (src_ppnum ? (mach_vm_address_t) data & 0x3FFF : 0));

	dest_ppnum = this->call("_pmap_find_phys", dest_pmapFindPhysArgs, 2);

	dest_pa = ((dest_ppnum << 14) | (dest_ppnum ? (mach_vm_address_t) address & 0x3FFF : 0));

	if(src_pa && dest_pa)
	{
		uint64_t bcopyPhysArgs[3] = { src_pa, dest_pa, size };

		ml_set_interrupts_enabled(false);

		this->call("_bcopy_phys", bcopyPhysArgs, 3);

		ml_set_interrupts_enabled(true);

		return true;
	}

#ifdef __arm64__

	#include <arm64/Isa_arm64.hpp>

	using namespace Arch::arm64;

	mach_vm_address_t kernel_map = *reinterpret_cast<mach_vm_address_t*>(this->getSymbolAddressByName("_kernel_map"));

	mach_vm_address_t vm_map_copy_discard = this->getSymbolAddressByName("_vm_map_copy_discard");

	mach_vm_address_t ipc_kmsg_copyout_body_strref = Arch::arm64::PatchFinder::findStringReference(macho, "Inconsistent OOL/copyout size on %p: expected %d, got %lld @%p @%s:%d", 1, __cstring_, __TEXT_XNU_BASE, false);

 	mach_vm_address_t vm_map_copy_discard_xref = Arch::arm64::PatchFinder::xref64(macho, ipc_kmsg_copyout_body_strref - 0xFFF, ipc_kmsg_copyout_body_strref, vm_map_copy_discard);

	mach_vm_address_t vm_map_copy_overwrite_branch = Arch::arm64::PatchFinder::step64(macho, vm_map_copy_discard_xref, 0x100, reinterpret_cast<bool(*)(uint32_t*)>(is_bl), -1, -1);
	
	vm_map_copy_overwrite_branch = Arch::arm64::PatchFinder::step64(macho, vm_map_copy_overwrite_branch + sizeof(uint32_t), 0x100, reinterpret_cast<bool(*)(uint32_t*)>(is_bl), -1, -1);

	vm_map_copy_overwrite_branch = Arch::arm64::PatchFinder::step64(macho, vm_map_copy_overwrite_branch + sizeof(uint32_t), 0x100, reinterpret_cast<bool(*)(uint32_t*)>(is_bl), -1, -1);

	bl_t bl = *(bl_t*) vm_map_copy_overwrite_branch;

	bool sign;

	uint64_t imm = bl.imm;

	if(imm & 0x2000000)
	{
		imm = ~(imm - 1);
		imm &= 0x1FFFFFF;

		sign = true;
	} else
	{
		sign = false;
	}

	imm *= (1 << 2);

	mach_vm_address_t vm_map_copy_overwrite = sign ? vm_map_copy_overwrite_branch - imm : vm_map_copy_overwrite_branch + imm;

	char buffer[128];

	snprintf(buffer, 128, "0x%llx", vm_map_copy_overwrite);

	MAC_RK_LOG("MacRK::Kernel vm_map_copy_overwrite = %s\n", buffer);

	struct vm_map_copy
	{
		int                    type;
		vm_object_offset_t     offset;
		vm_map_size_t          size;
		void                  *kdata;
	};

	typedef struct vm_map_copy *vm_map_copy_t;

	vm_map_copy_t copy;

	const uint8_t *write_data = (uint8_t*) data;

	while(size > 0)
	{
		size_t write_size = size;

		if(write_size > 0x4000)
			write_size = 0x4000;

		uint64_t ret;

		uint64_t vmMapCopyInArgs[5] = { kernel_map, (uint64_t) write_data, write_size, FALSE, (uint64_t) &copy };

		ret = this->call("_vm_map_copyin", vmMapCopyInArgs, 5);

		uint64_t vmMapCopyOverwriteArgs[5] = { kernel_map, address, (uint64_t) copy, write_size, FALSE };

		ret = this->call(vm_map_copy_overwrite, vmMapCopyOverwriteArgs, 5);

		address += write_size;
		write_data += write_size;
		size -= write_size;
	}

	return true;
#elif

	return kernel_write(address, data, size) || kernel_write_unsafe(address, data, size);

#endif
}

bool Kernel::writeUnsafe(mach_vm_address_t address, void *data, size_t size)
{
	return kernel_write_unsafe(address, data, size);
}

void Kernel::write8(mach_vm_address_t address, uint8_t value)
{
	kernel_write(address, reinterpret_cast<const void*>(&value), sizeof(value));
}

void Kernel::write16(mach_vm_address_t address, uint16_t value)
{
	kernel_write(address, reinterpret_cast<const void*>(&value), sizeof(value));
}

void Kernel::write32(mach_vm_address_t address, uint32_t value)
{
	kernel_write(address, reinterpret_cast<const void*>(&value), sizeof(value));
}

void Kernel::write64(mach_vm_address_t address, uint64_t value)
{
	kernel_write(address, reinterpret_cast<const void*>(&value), sizeof(value));
}

char* Kernel::readString(mach_vm_address_t address)
{
	return NULL;
}

Symbol* Kernel::getSymbolByName(char *symbolname)
{
	MachO *macho = this->macho;

	return macho->getSymbolByName(symbolname);
}

Symbol* Kernel::getSymbolByAddress(mach_vm_address_t address)
{
	MachO *macho = this->macho;

	return macho->getSymbolByAddress(address);
}

mach_vm_address_t Kernel::getSymbolAddressByName(char *symbolname)
{
	MachO *macho = this->macho;

	return macho->getSymbolAddressByName(symbolname);
}