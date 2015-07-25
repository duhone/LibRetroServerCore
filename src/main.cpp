#include <tchar.h>
#include "LibRetroCore.h"
#include <Platform\PathUtils.h>

int _tmain(int /*argc*/, _TCHAR* /*argv[]*/)
{
	auto core = LoadCore("cores/mame_libretro.dll");
	auto romPath = CR::Platform::RelativeToAbsolute("roms/galaga.zip");
	core->LoadGame(romPath.c_str());

	for(int i = 0; i < 200; ++i)
	{
		core->RunOneFrame();
	}

	return 0;
}

