#ifndef DATAFLOWSERDES_H
#define DATAFLOWSERDES_H

#include "dataflow/message.h"

namespace Dataflow
{
	namespace SerDes
	{
		void serializeMessageWsa(char* pOut, const Message& message, uint32_t* pSerializedMessageSize = nullptr);

		void deserializeMessageWsa(Message* pOut, const char* pMessage);
	}
}

#endif // DATAFLOWSERDES_H