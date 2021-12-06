#ifndef __PAYLOAD_HPP_
#define __PAYLOAD_HPP_

#include <stddef.h>
#include <stdint.h>

#include <mach/mach_types.h>
#include <mach/vm_types.h>

#include "Hook.hpp"

class Kernel;
class Task;
class Payload;

class Payload
{
	static constexpr uint32_t expectedSize = 1 << 12;

	public:
		Payload(Task *task, Hook *hook, vm_prot_t prot);

		~Payload();

		Hook* getHook() { return hook; }

		mach_vm_address_t getAddress() { return address; }

		void setCurrentOffset(off_t offset);

		off_t getCurrentOffset() { return current_offset; }

		size_t getSize() { return size; }

		vm_prot_t getProt() { return protection; }

		Task* getTask() { return task; }

		bool readBytes(uint8_t *bytes, size_t size);
		bool readBytes(off_t offset, uint8_t *bytes, size_t size);

		bool writeBytes(uint8_t *bytes, size_t size);
		bool writeBytes(off_t offset, uint8_t *bytes, size_t size);

		bool prepare();
	
	private:
		mach_vm_address_t address;

		off_t current_offset;

		Hook *hook;

		bool kernelPayload = false;

		size_t size;

		vm_prot_t prot;
};

#endif