#pragma once
#include <memory>
#include <Platform\SharedLibrary.h>

struct ILibRetroCore
{
	virtual ~ILibRetroCore() = default;
};

//This should only be called once. There can only be one core loaded at a time.
std::unique_ptr<ILibRetroCore> LoadCore(const char* a_coreName);
