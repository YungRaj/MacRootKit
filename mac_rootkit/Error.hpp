#ifndef __ERROR_HPP_

namespace Error
{
	enum Error : int32_t
	{
		kErrorUnknown          = -1,
		kErrorNone,
		kErrorInvalidArgument,
		kErrorOutOfMemory,
		kErrorNotSupported,
		kErrorNotAvailable,
		kErrorCompatibilityFailure,
	};
}

#endif