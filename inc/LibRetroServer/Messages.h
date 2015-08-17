#pragma once

namespace CR
{
	namespace LibRetroServer
	{
		namespace Messages
		{
			enum ServerMessageTypeT
			{
				Initialize,
				Shutdown,
				NumServerMessages
			};

			enum ClientMessageTypeT
			{
				CoreAcceptingMsgs,
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
			
			struct CoreAcceptingMsgsMessage
			{
				ClientMessageTypeT MessageType{CoreAcceptingMsgs};
			};
		}
	}
}