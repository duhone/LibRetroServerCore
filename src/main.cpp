#include <tchar.h>
#include "LibRetroCore.h"
#include <Platform\PathUtils.h>
#include <Platform\PipeClient.h>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include "Messages.h"
#include <functional>

using namespace CR;
using namespace CR::LibRetroServer;

std::unique_ptr<ILibRetroCore> g_retroCore;
std::mutex g_initializedMutex;
std::condition_variable g_initializedCondition;
bool g_initialized;
volatile bool g_close{false};

void OnInitializeMsg(void* a_msg, size_t a_msgSize)
{
	assert(a_msgSize == sizeof(Messages::InitializeMessage));
	auto msg = (Messages::InitializeMessage*)a_msg;
	g_retroCore = LoadCore(msg->CorePath);

	auto romPath = CR::Platform::RelativeToAbsolute(msg->GamePath);
	g_retroCore->LoadGame(romPath.c_str());
	g_initialized = true;
	g_initializedCondition.notify_one();
}

void OnShutdownMsg(void*, size_t)
{
	g_close = true;
	g_initialized = true;
	g_initializedCondition.notify_one(); //in case we never loaded a core and rom
}

std::function<void(void*, size_t)> g_msgHandlers[] = {OnInitializeMsg, OnShutdownMsg};

void OnPipeMessage(void* a_msg, size_t a_msgSize)
{
	assert(a_msgSize >= sizeof(Messages::MessageTypeT));
	auto msgType = (Messages::MessageTypeT*)a_msg;
	assert(*msgType < Messages::NumMessages);
	g_msgHandlers[*msgType](a_msg, a_msgSize);
}

int _tmain(int argc, _TCHAR* argv[])
{
	//we require getting the name of our pipe on the commandline
	if(argc < 2)
		return 1;

	auto pipeClient = Platform::CreatePipeClient(argv[1], OnPipeMessage);
	if(!pipeClient)
		return 1;

	std::unique_lock<std::mutex> initLock(g_initializedMutex);
	g_initializedCondition.wait(initLock, []() { return g_initialized; });
			
	while(!g_close)
	{
		g_retroCore->RunOneFrame();
	}

	return 0;
}

