#pragma once

#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define RAY_PLATFORM_WINDOWS
	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
	#endif
/* We have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
	#define RAY_PLATFORM_ANDROID
#elif defined(__linux__)
	#define RAY_PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	/* Not supported compiler/platform */
	#error "Platform is not supported!"
#endif // End of platform detection
