#pragma once
#include <memory>
#include <Platform\SharedLibrary.h>

struct ILibRetroCore
{
	enum class PixelFormat
	{
		XRGB1555,
		RGB565,
		XRGB8888
	};
	virtual ~ILibRetroCore() = default;
	virtual PixelFormat GetPixelformat() const = 0;
	virtual bool LoadGame(const char* a_gameFile) = 0;
	virtual void UnloadGame() = 0;
	virtual void RunOneFrame() = 0;
};

//This should only be called once. There can only be one core loaded at a time.
std::unique_ptr<ILibRetroCore> LoadCore(const char* a_coreName);
