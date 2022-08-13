#include <stdio.h>
#include <stdlib.h>

#include <mach/mach.h>

#include <iostream>

#include <IOKit/IOKitLib.h>

#include "Kernel.hpp"
#include "Task.hpp"

using namespace std;

int main()
{
	Kernel *kernel = new Kernel();

	printf("Kernel base = 0x%llx slide = 0x%llx\n", kernel->getBase(), kernel->getSlide());

	Task *task = new Task(kernel, 1382);

	printf("PID 1382 task = 0x%llx proc = 0x%llx\n", task->getTask(), task->getProc());
}