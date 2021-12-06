#ifndef __ERROR_HPP_

namespace Error
{
	enum
	{
		kErrorUnknown          = -1,
		kErrorNone             = 0,
		kErrorInvalidArgument,
		kErrorOutOfMemory,
		kErrorNotSupported,
		kErrorNotAvailable,
		kErrorCompatibilityFailure,
	};

	typedef Error int32_t;
}

#endif