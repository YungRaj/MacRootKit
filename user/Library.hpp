#ifndef __LIBRARY_HPP_
#define __LIBRARY_HPP_

#include "Task.hpp"

namespace dyld
{
	class Library
	{
		public:
			Library(xnu::Task *task, dyld::Dyld *dyld, struct dyld_image_info *image_info)
				: task(task), dyld(dyld), image_info(image_info) { }

			~Library() { }

			dyld::Dyld* getDyld() { return dyld; }

			xnu::Task* getTask() { return task; }

		private:
			xnu::Task *task;

			dyld::Dyld *dyld;

			struct dyld_image_info *image_info;
	};
};

#endif