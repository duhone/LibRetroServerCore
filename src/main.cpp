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
	assert(a_msgSize >= sizeof(Messages::ServerMessageTypeT));
	auto msgType = (Messages::ServerMessageTypeT*)a_msg;
	static_assert(Messages::NumServerMessages == sizeof(g_msgHandlers) / sizeof(std::function<void(void*, size_t)>), 
				  "incorrect number of msg handlers");
	assert(*msgType < Messages::NumServerMessages);
	if(g_msgHandlers[*msgType])
		g_msgHandlers[*msgType](a_msg, a_msgSize);
}

int _tmain(int argc, _TCHAR* argv[])
{
	//we require getting the name of our pipe on the commandline
	if(argc != 1 || strlen(argv[0]) != 32)
		return 1;

	std::string pipeName{R"(\\.\pipe\)"};
	pipeName += argv[0];
	auto pipeClient = Platform::CreatePipeClient(pipeName.c_str(), OnPipeMessage);
	if(!pipeClient)
		return 1;

	pipeClient->SendPipeMessage(Messages::CoreAcceptingMsgsMessage{});
	std::unique_lock<std::mutex> initLock(g_initializedMutex);
	g_initializedCondition.wait(initLock, []() { return g_initialized; });
			
	while(!g_close)
	{
		g_retroCore->RunOneFrame();
	}

	return 0;
}

