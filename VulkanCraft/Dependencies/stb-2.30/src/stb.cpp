#include <stdlib.h>
#include <string.h>

#define STBI_MALLOC(sz)                     ::operator new(sz)
#define STBI_REALLOC(p, newsz)              static_assert(false)
#define STBI_FREE(p)                        ::operator delete(p)

void* _stbi_realloc_sized_impl(void* p, size_t old_size, size_t new_size)
{
	if (old_size < new_size)
	{
		void* new_p = STBI_MALLOC(new_size);
		memcpy(new_p, p, old_size);
		STBI_FREE(p);
		p = new_p;
	}
	return p;
}
#define STBI_REALLOC_SIZED(p, oldsz, newsz) ::_stbi_realloc_sized_impl((p), (oldsz), (newsz))

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined(_WIN32) && (defined(UNICODE) || defined(_UNICODE))
	#define STBIW_WINDOWS_UTF8
#endif
#include "stb_image_write.h"
