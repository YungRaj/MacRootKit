#ifndef __PROCESS_HPP_
#define __PROCESS_HPP_

namespace xnu
{
	class Task;
};

namespace bsd
{
	class Process
	{
		public:
			Process();

			~Process();

			xnu::Task* getTask();
		private:
			xnu::Task *task;
	};

};

#endif