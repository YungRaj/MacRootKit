#ifndef __PROCESS_HPP_
#define __PROCESS_HPP_

namespace xnu
{
	class Task;
};

typedef void* pmap_t;

typedef void* proc_t;

namespace bsd
{
	class Process
	{
		public:
			Process();

			~Process();

			xnu::Task* getTask();
		private:
			proc_t proc;
			task_t task;

			xnu::Task *task;
	};

};

#endif