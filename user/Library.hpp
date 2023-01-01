#ifndef __LIBRARY_HPP_
#define __LIBRARY_HPP_

class Task;
class Thread;

class Dyld;

class MachO;
class UserMachO;

namespace dyld
{
	class Library
	{
		public:
			Library();

			~Library();

			xnu::UserMachO* getMachO() { return macho; }

			dyld::Dyld* getDyld() { return dyld; }

			xnu::Task* getTask() { return task; }

			static Library* injectLibrary(Task *task, const char *path);

		private:
			xnu::UserMachO *macho;

			dyld::Dyld *dyld;

			xnu::Task *task;
	};

};

#endif