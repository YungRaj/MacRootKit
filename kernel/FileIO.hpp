#ifndef __FILE_IO_HPP_
#define __FILE_IO_HPP_

#include <sys/kernel_types.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/vnode.h>

#include <IOKit/IOLib.h>

namespace FileIO
{
	vnode_t vnodeForPath(const char *path);

	int performIO(void *buffer, off_t off, size_t size, vnode_t vnode, vfs_context_t ctxt, bool write);

	int read(void *buffer, off_t off, size_t sz, vnode_t vnode, vfs_context_t ctxt);

	int write(void *buffer, off_t off, size_t sz, vnode_t vnode, vfs_context_t ctxt);

	uint8_t* readFile(const char *path, size_t *size);

	int writeFile(const char *path, void *buffer, size_t size, int fmode = O_TRUNC | O_CREAT | FWRITE | O_NOFOLLOW,
											 	  			   int cmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	size_t getSize(vnode_t vnode, vfs_context_t ctxt);
}

#endif