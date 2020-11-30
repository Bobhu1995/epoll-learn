#pragma once
#include <inttypes.h>
namespace Server {

#if defined(_MSC_VER) //microsoft vc studio 
#if defined(WIN32) || defined(_WIN32)

	typedef char				int8_t;
	typedef unsigned char		uint8_t;
	typedef short				int16_t;
	typedef unsigned short		uint16_t;
	typedef int					int32_t;
	typedef unsigned int		uint32_t;
	typedef long long			int64_t;
	typedef unsigned long long	uint64_t;
	typedef __int64				time_t;
#else if defined(_WIN64)
	typedef char				int8_t;
	typedef unsigned char		uint8_t;
	typedef short				int16_t;
	typedef unsigned short		uint16_t;
	typedef int					int32_t;
	typedef unsigned int		uint32_t;
	typedef long 				int64_t;
	typedef unsigned long		uint64_t;
	typedef __int64				time_t;
#endif
#endif	//_MSC_VER

}