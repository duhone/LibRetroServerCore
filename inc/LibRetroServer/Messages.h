#pragma once
#include <core\Guid.h>

namespace CR
{
	namespace LibRetroServer
	{
		namespace Messages
		{
			//messages from the server
			enum ServerMessageTypeT
			{
				Initialize,
				Shutdown,
				SharedMemoryInit,
				ReadyToRecieveVideo,
				NumServerMessages
			};

			//messages from the client
			enum ClientMessageTypeT
			{
				CoreAcceptingMsgs,
				SetupVideo,
				VideoReady,
				NumClientMessages
			};

			struct InitializeMessage
			{
				ServerMessageTypeT MessageType{Initialize};
				char CorePath[255];
				char GamePath[255];
			};

			struct ShutdownMessage
			{
				ServerMessageTypeT MessageType{Shutdown};
			};

			struct SharedMemoryInitMessage
			{
				ServerMessageTypeT MessageType{SharedMemoryInit};
				CR::Core::Guid SharedMemoryName;
			};

			struct ReadyToRecieveVideoMessage
			{
				ServerMessageTypeT MessageType{ReadyToRecieveVideo};
			};
			
			struct CoreAcceptingMsgsMessage
			{
				ClientMessageTypeT MessageType{CoreAcceptingMsgs};
			};

			struct VideoReadyMessage
			{
				ClientMessageTypeT MessageType{VideoReady};
			};

			struct SetupVideoMessage
			{
				ClientMessageTypeT MessageType{SetupVideo};
				uint16_t Width;
				uint16_t Height;
			};
		}
	}
}