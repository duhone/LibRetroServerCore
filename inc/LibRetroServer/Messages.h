#pragma once

namespace CR
{
	namespace LibRetroServer
	{
		namespace Messages
		{
			enum MessageTypeT
			{
				Initialize,
				Shutdown,
				CoreAcceptingMsgs,
				NumMessages
			};

			struct InitializeMessage
			{
				MessageTypeT MessageType{Initialize};
				char CorePath[255];
				char GamePath[255];
			};
			
			struct CoreAcceptingMsgsMessage
			{
				MessageTypeT MessageType{CoreAcceptingMsgs};
			};

			struct ShutdownMessage
			{
				MessageTypeT MessageType{Shutdown};
			};
		}
	}
}