#ifndef __PROCESS_HPP_
#define __PROCESS_HPP_

class Task;

class Disassembler;

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