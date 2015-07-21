#include "LibRetroCore.h"
#include "libretro.h"
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <string>

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
		bool OnRetroEnvironmentCmd(unsigned int cmd, void *data);
	private:
		bool OnEnvironmentSetVariables(void* data);

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

		std::function<bool(void* data)> m_onRetroEnvironmentCmd[40];

		std::unordered_map<std::string, CoreOption> m_options;
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

	m_onRetroEnvironmentCmd[RETRO_ENVIRONMENT_SET_VARIABLES] = [this](void* data) { return this->OnEnvironmentSetVariables(data); };


	m_retroSetEnvironment(retro_environment);
	m_retroSetVideoRefresh(retro_video_refresh);
	m_retroSetAudioSample(retro_audio_sample);
	m_retroSetAudioSampleBatch(retro_audio_sample_batch);
	m_retroSetInputPoll(retro_input_poll);
	m_retroSetInputState(retro_input_state);
	
	m_retroInit();
}

LibRetroCore::~LibRetroCore()
{
	assert(g_libRetroCore);
	m_retroDeinit();
	g_libRetroCore = nullptr;
}

bool LibRetroCore::OnRetroEnvironmentCmd(unsigned cmd, void *data)
{
	if(!m_onRetroEnvironmentCmd[cmd])
		return false; //not a cmd we handle
	return m_onRetroEnvironmentCmd[cmd](data);
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

std::unique_ptr<ILibRetroCore> LoadCore(const char* a_coreName)
{
	auto core = CR::Platform::LoadSharedLibrary(a_coreName);
	if(!core)
		return nullptr;
	return std::make_unique<LibRetroCore>(std::move(core));
}