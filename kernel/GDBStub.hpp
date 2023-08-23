#ifndef __GDBSTUB_HPP_
#define __GDBSTUB_HPP_

#include <Architecture.hpp>
#include <Array.hpp>

namespace xnu
{
	class Kernel;
	class Thread;
}

namespace mrk
{
	class MacRootKit;
	class KernelPatcher;
};

namespace Debug
{
	enum class DebuggerAction
	{
		Interrupt,
		Continue,
		StepThreadLocked,
		StepThreadUnlocked,
		ShutdownEmulation,
	};

	constexpr char GDB_STUB_START = '$';
	constexpr char GDB_STUB_END = '#';
	constexpr char GDB_STUB_ACK = '+';
	constexpr char GDB_STUB_NACK = '-';
	constexpr char GDB_STUB_INT3 = 0x03;
	constexpr int  GDB_STUB_SIGTRAP = 5;

	constexpr char GDB_STUB_REPLY_ERR[] = "E01";
	constexpr char GDB_STUB_REPLY_OK[] = "OK";
	constexpr char GDB_STUB_REPLY_EMPTY[] = "";

	class Debugger
	{
		public:
			explicit Debugger(xnu::Kernel *kernel);

			virtual void connected() = 0;

			virtual uint8_t* readFromClient() = 0;

			virtual void writeToClient(std::span<const u8> data) = 0;

			virtual xnu::Thread* getActiveThread() = 0;

			virtual void setActiveThread(xnu::Thread *thread) = 0;
			
			virtual void stopped(xnu::Thread* thread) = 0;

			virtual void shuttingDown() = 0;

			virtual void watchpoint(xnu::Thread* thread) = 0;

			virtual std::Array<DebuggerAction> clientData(uint8_t *data) = 0;

			bool notifyThreadStopped(Kernel::KThread* thread);

			void notifyShutdown();

			bool notifyThreadWatchpoint(xnu::Thread* thread);

		protected:
			GDBStubArch *arch;

			xnu::Kernel *kernel;

			Debug::Dwarf *dwarf;

			xnu::KernelMachO *macho;

			mrk::MacRootKit *mrk;

	};

	class GDBStubA64 : public Debugger
	{
		public:
			explicit GDBStubArm64(xnu::Kernel *kernel);

			void addBreakpoint(mach_vm_address_t address);

			void removeBreakpoint(mach_vm_address_t address);

			// std::String registerWrite(xnu::Thread* thread, std::String value) const override;

			// std::String registerRead(xnu::Thread* thread, size_t id) const override;

			// void registerWrite(xnu::Thread* thread, size_t id, std::String value) const override;

			// void writeRegisters(xnu::Thread* thread, std::String register_data) const override;

			// std::String threadStatus(xnu::Thread* thread, u8 signal) const override;

		private:
			Arch::Architecture architecture;

			Array<Hook*> breakpoints;
	};

	class GDBStubX64 : public Debugger
	{
		public:
			explicit GDBStubX64(xnu::Kernel *kernel);

			void addBreakpoint(mach_vm_address_t address);

			void removeBreakpoint(mach_vm_address_t address);

			// std::String registerWrite(xnu::Thread* thread, std::String value) const override;

			// std::String registerRead(xnu::Thread* thread, size_t id) const override;

			// void registerWrite(xnu::Thread* thread, size_t id, std::String value) const override;

			// void writeRegisters(xnu::Thread* thread, std::String register_data) const override;

			// std::String threadStatus(xnu::Thread* thread, u8 signal) const override;

		private:
			Arch::Architecture architecture;

			Array<Hook*> breakpoints;
	}
};

#endif