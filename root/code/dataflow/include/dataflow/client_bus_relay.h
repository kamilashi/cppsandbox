#ifndef DATAFLOWCLIENTBUSRELAY_H
#define DATAFLOWCLIENTBUSRELAY_H

#include "dataflow/network_pipe.h"
#include "dataflow/message_bus.h"
#include <memory>

namespace Dataflow
{
	namespace Ipc
	{
		class ClientBusRelay
		{
		public:
			ClientBusRelay(NetworkBackend networkBackend);

			~ClientBusRelay() = default;

			void relay(const Message&);

			std::shared_ptr <Bus> getMessageBus() const;

		private:
			NetworkPipe m_networkPipe;
		};
	}
}

#endif // DATAFLOWCLIENTBUSRELAY_H