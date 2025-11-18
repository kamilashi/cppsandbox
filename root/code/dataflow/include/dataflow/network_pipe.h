#ifndef DATAFLOWNETWORKPIPE_H
#define DATAFLOWNETWORKPIPE_H

#include "message.h"

namespace Dataflow
{
	namespace Ipc
	{
		enum class NetworkBackend
		{
			NetworkBackend_WSA
		};

		enum class NetworkType
		{
			NetworkType_Client,
			NetworkType_Server,

			NetworkType_Unknown
		};

		class NetworkPipe
		{
		public:
			NetworkPipe(NetworkBackend);
			~NetworkPipe() = default;

			void initializeClient();
			void initializeServer();
			void send(const Message& message);

		private:
			NetworkBackend m_netBackend;
			NetworkType m_netType;
		};
	}
}

#endif //DATAFLOWNETWORKPIPE_H

