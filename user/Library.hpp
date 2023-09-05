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
			Library(xnu::Task *task, dyld::Dyld *dyld, struct dyld_image_info *image_info)
				: task(task), dyld(dyld), image_info(image_info), path(task->readString(image_file_path)) { }

			~Library() { free(this->path); }

			char* getPath() { return path; }

			dyld::Dyld* getDyld() { return dyld; }

			xnu::Task* getTask() { return task; }

		private:
			char *path;

			xnu::Task *task;

			dyld::Dyld *dyld;

			struct dyld_image_info *image_info;
	};
};

#endif