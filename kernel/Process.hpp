#ifndef __PROCESS_HPP_
#define __PROCESS_HPP_

namespace xnu
{
	class Task;
};

using namespace xnu;

class Process
{
	public:
		Process();

		~Process();

		Task* getTask();
	private:
		Task *task;
};

#endif