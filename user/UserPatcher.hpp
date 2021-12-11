#ifndef __USERPATCHER_HPP_
#define __USERPATCHER_HPP_

#include "Patcher.hpp"

class Hook;

class UserPatcher : Patcher
{
	public:
		Patcher();

		~Patcher();

		virtual void findAndReplace(void *data,
									size_t data_size,
									const void *find, size_t find_size,
									const void *replace, size_t replace_size);

		virtual void onKextLoad(void *kext, kmod_info_t *kmod);

	private:
};
}

#endif