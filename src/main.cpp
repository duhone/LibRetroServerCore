#include <tchar.h>
#include "LibRetroCore.h"

int _tmain(int /*argc*/, _TCHAR* /*argv[]*/)
{
	auto core = LoadCore("cores/mame_libretro.dll");
	return 0;
}

