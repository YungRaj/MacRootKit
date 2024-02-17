#include "FileIO.hpp"
#include "Log.hpp"

UInt8 *FileIO::readFile(const char *path, Size *size) 
{
	vnode_t vnode = NULLVP;

	vfs_context_t ctxt = vfs_context_create(nullptr);

	UInt8 *buf = nullptr;

	int err = vnode_lookup(path, 0, &vnode, ctxt);

	if(!err)
	{
		*size = FileIO::getSize(vnode, ctxt);
		
		if(*size > 0) 
		{
			buf = new UInt8[*size+1];

			if (buf)
			{
				if(FileIO::read(buf, 0, *size, vnode, ctxt))
				{
					MAC_RK_LOG("MacPE::failed to read %s file of %lu size", path, *size);

					buf = nullptr;
				} else
				{
					buf[*size] = 0x00;
				}
			} else
			{
				MAC_RK_LOG("MacPE::failed to allocate memory for reading %s file of %lu size", path, *size);
			}
		} else
		{
			MAC_RK_LOG("MacPE::failed to obtain %s size", path);
		}

		vnode_put(vnode);
	} else
	{
		MAC_RK_LOG("MacPE::failed to find %s", path);
	}

	vfs_context_rele(ctxt);

	return buf;
}


int FileIO::read(void *buffer, Offset off, Size size, vnode_t vnode, vfs_context_t ctxt)
{
	return FileIO::performIO(buffer, off, size, vnode, ctxt, false);
}

Size FileIO::getSize(vnode_t vnode, vfs_context_t ctxt)
{
	// Taken from XNU vnode_size
	vnode_attr va;

	VATTR_INIT(&va);
	VATTR_WANTED(&va, va_data_size);

	return vnode_getattr(vnode, &va, ctxt) ? 0 : (Size)va.va_data_size;
}

int FileIO::writeFile(const char *path, void *buffer, Size size, int fmode, int cmode)
{
	vnode_t vnode = NULLVP;

	vfs_context_t ctxt = vfs_context_create(nullptr);

	int err = vnode_open(path, fmode, cmode, VNODE_LOOKUP_NOFOLLOW, &vnode, ctxt);

	if (!err)
	{
		err = FileIO::write(buffer, 0, size, vnode, ctxt);

		if (!err)
		{
			err = vnode_close(vnode, FWASWRITTEN, ctxt);

			if (err)
				MAC_RK_LOG("MacPE::vnode_close(%s) failed with error %d!", path, err);

		} else 
		{
			MAC_RK_LOG("MacPE::failed to write %s file of %lu size", path, size);
		}

	} else
	{
		MAC_RK_LOG("MacPE::failed to create file %s with error %d", path, err);
	}

	vfs_context_rele(ctxt);

	return err;
}

int FileIO::write(void *buffer, Offset off, Size size, vnode_t vnode, vfs_context_t ctxt)
{
	return FileIO::performIO(buffer, off, size, vnode, ctxt, true);
}

int FileIO::performIO(void *buffer, Offset off, Size size, vnode_t vnode, vfs_context_t ctxt, bool write)
{
	uio_t uio = uio_create(1, off, UIO_SYSSPACE, write ? UIO_WRITE : UIO_READ);

	if (!uio)
	{
		MAC_RK_LOG("MacPE::uio_create returned null!");

		return EINVAL;
	}

	// imitate the kernel and read a single page from the file
	int error = uio_addiov(uio, CAST_USER_ADDR_T(buffer), size);

	if (error)
	{
		MAC_RK_LOG("MacPE::uio_addiov returned error %d!", error);

		return error;
	}

	if (write)
		error = VNOP_WRITE(vnode, uio, 0, ctxt);
	else
		error = VNOP_READ(vnode, uio, 0, ctxt);

	if (error)
	{
		MAC_RK_LOG("MacPE::%s failed %d!", write ? "VNOP_WRITE" : "VNOP_READ", error);

		return error;
	}

	if (uio_resid(uio))
	{
		MAC_RK_LOG("MacPE::uio_resid returned non-null!");

		return EINVAL;
	}

	return 0;
}