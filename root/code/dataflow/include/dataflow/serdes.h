#ifndef DATAFLOWSERDES_H
#define DATAFLOWSERDES_H

#include "dataflow/message.h"

namespace Dataflow
{
	namespace SerDes
	{
		void serializeMessage(char* pOut, const Message& message, uint32_t* pSerializedMessageSize = nullptr);

		void deserializeMessage(Message* pOut, const char* pMessage);

		void serializeMessageWSA(char* pOut, const Message& message, uint32_t* pSerializedMessageSize = nullptr);

		void deserializeMessageWSA(Message* pOut, const char* pMessage);
	}
}

#endif // DATAFLOWSERDES_H