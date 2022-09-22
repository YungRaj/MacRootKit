#ifndef __LIBRARY_HPP_
#define __LIBRARY_HPP_

class Task;
class Thread;

class Dyld;

class MachO;
class UserMachO;

class Library
{
	public:
		Library();

		~Library();

		UserMachO* getMachO() { return macho; }

		Dyld* getDyld() { return dyld; }

		Task* getTask() { return task; }

		static Library* injectLibrary(Task *task, const char *path);

	private:
		UserMachO *macho;

		Dyld *dyld;

		Task *task;
}

#endif