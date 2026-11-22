// SPDX-License-Identifier: MIT
/*
 * libmain.so - loader stub for the Android build.
 *
 * The game wrapper stores every pointer it hands to the translated x86 code
 * truncated to 32 bits (see AGENTS.md: push32 / PTR32).  That is only lossless
 * when the game code, its DATA/BSS and its string literals live in the low
 * 4 GB of the address space.  On Linux this is achieved with -no-pie; on
 * Android every library is position independent and the dynamic linker maps
 * it far above 4 GB, which would also make the R_AARCH64_ABS32 relocations in
 * the DATA initializers fail to load ("out of range" in the bionic linker).
 *
 * So the game itself is built as libnfs2se.so and this stub - which SDL loads
 * as the "main" shared object and in which it looks up SDL_main - reserves a
 * low address range and asks the bionic linker to map libnfs2se.so there
 * (ANDROID_DLEXT_RESERVED_ADDRESS), then calls nfs2se_main inside it.
 *
 * The range [0x10000000, 0x18000000) is the gap left free by malloc32's
 * identity-mapped pool chunks (see pool_grow in src/Wrapper.c).
 */
#include <android/dlext.h>
#include <android/log.h>
#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

#ifndef MAP_FIXED_NOREPLACE
#define MAP_FIXED_NOREPLACE 0x100000
#endif

#define TAG "NFSIISE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define GAME_LIB      "libnfs2se.so"
#define GAME_LIB_ADDR ((void *)(uintptr_t)0x10000000)
#define GAME_LIB_SIZE ((size_t)0x08000000) /* 128 MB */

typedef int (*nfs2se_main_t)(int argc, char *argv[]);

__attribute__((visibility("default")))
int SDL_main(int argc, char *argv[])
{
	void *handle = NULL;
	nfs2se_main_t gameMain;

	void *reserved = mmap(GAME_LIB_ADDR, GAME_LIB_SIZE, PROT_NONE,
	                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_FIXED_NOREPLACE,
	                      -1, 0);
	if (reserved == MAP_FAILED)
	{
		LOGE("Cannot reserve the low address range: %s", strerror(errno));
	}
	else if (reserved != GAME_LIB_ADDR)
	{
		LOGE("Low address range unavailable (got %p)", reserved);
		munmap(reserved, GAME_LIB_SIZE);
	}
	else
	{
		android_dlextinfo info;
		memset(&info, 0, sizeof info);
		info.flags         = ANDROID_DLEXT_RESERVED_ADDRESS;
		info.reserved_addr = reserved;
		info.reserved_size = GAME_LIB_SIZE;

		handle = android_dlopen_ext(GAME_LIB, RTLD_NOW | RTLD_LOCAL, &info);
		if (!handle)
		{
			LOGE("android_dlopen_ext(%s) failed: %s", GAME_LIB, dlerror());
			munmap(reserved, GAME_LIB_SIZE);
		}
		else
		{
			LOGI("%s mapped at the reserved address %p", GAME_LIB, reserved);
		}
	}

	if (!handle)
	{
		/* Last resort: the game will almost certainly crash because its
		   32-bit pointers are truncated, but at least say why. */
		LOGE("Loading %s at the default address - expect a crash", GAME_LIB);
		handle = dlopen(GAME_LIB, RTLD_NOW | RTLD_LOCAL);
		if (!handle)
		{
			LOGE("dlopen(%s) failed: %s", GAME_LIB, dlerror());
			return 1;
		}
	}

	gameMain = (nfs2se_main_t)dlsym(handle, "nfs2se_main");
	if (!gameMain)
	{
		LOGE("nfs2se_main not found: %s", dlerror());
		return 1;
	}
	return gameMain(argc, argv);
}
