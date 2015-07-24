#include "LibRetroCore.h"
#include "libretro.h"
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <string>
#include <Platform\MemoryMappedFile.h>

namespace
{
	class LibRetroCore : public ILibRetroCore
	{
		struct CoreOption
		{
			std::string Description;
			std::vector<std::string> Options;
			unsigned int CurrentOption = 0;
		};
	public:
		LibRetroCore(std::unique_ptr<CR::Platform::ISharedLibrary>&& a_core);
		virtual ~LibRetroCore();

		PixelFormat GetPixelformat() const override { return m_pixelFormat; }
		bool LoadGame(const char* a_gameFile) override;
		void UnloadGame() override;

		bool OnRetroEnvironmentCmd(unsigned int cmd, void *data);
	private:
		//Handlers for retro environment commands
		bool OnEnvironmentSetVariables(void* data);
		bool OnEnvironmentSetPixelFormat(void* data);
		bool OnEnvironmentGetVariable(void* data);

		std::unique_ptr<CR::Platform::ISharedLibrary> m_coreLibrary;

		//retro api loaded from a lib retro core dll.
		std::function<void()> m_retroInit;
		std::function<void()> m_retroDeinit;
		std::function<void(retro_environment_t)> m_retroSetEnvironment;
		std::function<void(retro_video_refresh_t)> m_retroSetVideoRefresh;
		std::function<void(retro_audio_sample_t)> m_retroSetAudioSample;
		std::function<void(retro_audio_sample_batch_t)> m_retroSetAudioSampleBatch;
		std::function<void(retro_input_poll_t)> m_retroSetInputPoll;
		std::function<void(retro_input_state_t)> m_retroSetInputState;
		std::function<void(retro_system_info*)> m_retroGetSystemInfo;
		std::function<bool(const retro_game_info*)> m_retroLoadGame;
		std::function<void()> m_retroUnloadGame;

		std::function<bool(void* data)> m_onRetroEnvironmentCmd[40];

		std::unordered_map<std::string, CoreOption> m_options;
		PixelFormat m_pixelFormat{PixelFormat::XRGB8888};
		retro_system_info m_retroSysInfo;
		std::unique_ptr<CR::Platform::IMemoryMappedFile> m_gameMMap; //Docs not clear if we need to actually keep this around
		bool m_gameLoaded{false};
	};

	LibRetroCore* g_libRetroCore = nullptr;

	//have to pass to a lib retro core, no idea what its for
	bool retro_environment(unsigned cmd, void *data)
	{
		data;
		std::cout << "retro_environment " << cmd << std::endl;	
		assert(g_libRetroCore);
		return g_libRetroCore->OnRetroEnvironmentCmd(cmd, data);
	}

	void retro_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch)
	{
		data;
		std::cout << "retro_video_refresh " << width << height << pitch << std::endl;
	}
	
	void retro_audio_sample(int16_t left, int16_t right)
	{
		std::cout << "retro_audio_sample " << left << right << std::endl;
	}

	//l and r are interleaved
	size_t retro_audio_sample_batch(const int16_t *data, size_t frames)
	{
		data;
		std::cout << "retro_audio_sample_batch " << frames << std::endl;
		return 0;
	}

	void retro_input_poll(void)
	{
		std::cout << "retro_input_poll " << std::endl;
	}

	int16_t retro_input_state(unsigned port, unsigned device, unsigned index, unsigned id)
	{
		std::cout << "retro_input_state " << port << device << index << id << std::endl;
		return 0;
	}
}


LibRetroCore::LibRetroCore(std::unique_ptr<CR::Platform::ISharedLibrary>&& a_core) : m_coreLibrary(std::move(a_core))
{
	assert(!g_libRetroCore);
	g_libRetroCore = this;

	m_retroInit = m_coreLibrary->GetStdFunction<void()>("retro_init");
	m_retroDeinit = m_coreLibrary->GetStdFunction<void()>("retro_deinit");
	m_retroSetEnvironment = m_coreLibrary->GetStdFunction<void(retro_environment_t)>("retro_set_environment");
	m_retroSetVideoRefresh = m_coreLibrary->GetStdFunction<void(retro_video_refresh_t)>("retro_set_video_refresh");
	m_retroSetAudioSample = m_coreLibrary->GetStdFunction<void(retro_audio_sample_t)>("retro_set_audio_sample");
	m_retroSetAudioSampleBatch = m_coreLibrary->GetStdFunction<void(retro_audio_sample_batch_t)>("retro_set_audio_sample_batch");
	m_retroSetInputPoll = m_coreLibrary->GetStdFunction<void(retro_input_poll_t)>("retro_set_input_poll");
	m_retroSetInputState = m_coreLibrary->GetStdFunction<void(retro_input_state_t)>("retro_set_input_state");
	m_retroGetSystemInfo = m_coreLibrary->GetStdFunction<void(retro_system_info*)>("retro_get_system_info");
	m_retroLoadGame = m_coreLibrary->GetStdFunction<bool(const retro_game_info*)>("retro_load_game");
	m_retroUnloadGame = m_coreLibrary->GetStdFunction<void()>("retro_unload_game");

	m_onRetroEnvironmentCmd[RETRO_ENVIRONMENT_SET_VARIABLES] = [this](void* data) { 
		return this->OnEnvironmentSetVariables(data); };
	m_onRetroEnvironmentCmd[RETRO_ENVIRONMENT_SET_PIXEL_FORMAT] = [this](void* data) { 
		return this->OnEnvironmentSetPixelFormat(data); };
	m_onRetroEnvironmentCmd[RETRO_ENVIRONMENT_GET_VARIABLE] = [this](void* data) { 
		return this->OnEnvironmentGetVariable(data); };
	

	m_retroSetEnvironment(retro_environment);
	m_retroSetVideoRefresh(retro_video_refresh);
	m_retroSetAudioSample(retro_audio_sample);
	m_retroSetAudioSampleBatch(retro_audio_sample_batch);
	m_retroSetInputPoll(retro_input_poll);
	m_retroSetInputState(retro_input_state);
	m_retroGetSystemInfo(&m_retroSysInfo);

	m_retroInit();
}

LibRetroCore::~LibRetroCore()
{
	assert(g_libRetroCore);
	UnloadGame();
	m_retroDeinit();
	g_libRetroCore = nullptr;
}

bool LibRetroCore::OnRetroEnvironmentCmd(unsigned cmd, void *data)
{
	if(!m_onRetroEnvironmentCmd[cmd])
		return false; //not a cmd we handle
	return m_onRetroEnvironmentCmd[cmd](data);
}

bool LibRetroCore::OnEnvironmentSetPixelFormat(void* data)
{
	retro_pixel_format pixfmt = *(retro_pixel_format *)data;
	switch(pixfmt)
	{
	case retro_pixel_format::RETRO_PIXEL_FORMAT_0RGB1555:
		m_pixelFormat = PixelFormat::XRGB1555;
		return true;
	case retro_pixel_format::RETRO_PIXEL_FORMAT_RGB565:
		m_pixelFormat = PixelFormat::RGB565;
		return true;
	case retro_pixel_format::RETRO_PIXEL_FORMAT_XRGB8888:
		m_pixelFormat = PixelFormat::XRGB8888;
		return true;
	};
	return false;
}

bool LibRetroCore::OnEnvironmentGetVariable(void* data)
{
	retro_variable* variable = (retro_variable*)data;
	variable->value = nullptr;
	auto optionIter = m_options.find(variable->key);
	if(optionIter != end(m_options))
		variable->value = optionIter->second.Options[optionIter->second.CurrentOption].c_str();
	return true;
}

bool LibRetroCore::OnEnvironmentSetVariables(void* data)
{
	const retro_variable* variables = (const retro_variable*)data;
	while(variables->key)
	{
		CoreOption option;
		std::string value = variables->value;
		auto currentEnd = value.find(';');
		option.Description = value.substr(0, currentEnd);
		++currentEnd;
		while(currentEnd < value.size() && currentEnd != std::string::npos)
		{
			++currentEnd;
			auto offset = currentEnd;
			currentEnd = value.find('|', offset);
			option.Options.push_back(value.substr(offset, currentEnd-offset));
		}
		option.CurrentOption = 0;
		m_options[variables->key] = option;
		++variables;
	}
	return true;
}

bool LibRetroCore::LoadGame(const char* a_gameFile)
{
	retro_game_info gameInfo;
	if(m_retroSysInfo.need_fullpath)
	{
		gameInfo.path = a_gameFile;
		gameInfo.data = nullptr;
		gameInfo.size = 0;
		gameInfo.meta = nullptr;
	}
	else
	{
		m_gameMMap = CR::Platform::OpenMMapFile(a_gameFile);
		gameInfo.path = a_gameFile;
		gameInfo.data = m_gameMMap->data();
		gameInfo.size = m_gameMMap->size();;
		gameInfo.meta = nullptr;
	}

	m_gameLoaded = m_retroLoadGame(&gameInfo);
	return m_gameLoaded;
}

void LibRetroCore::UnloadGame()
{
	if(m_gameLoaded)
	{
		m_retroUnloadGame();
		m_gameMMap.reset(nullptr);
		m_gameLoaded = false;
	}
}

std::unique_ptr<ILibRetroCore> LoadCore(const char* a_coreName)
{
	auto core = CR::Platform::LoadSharedLibrary(a_coreName);
	if(!core)
		return nullptr;
	return std::make_unique<LibRetroCore>(std::move(core));
}