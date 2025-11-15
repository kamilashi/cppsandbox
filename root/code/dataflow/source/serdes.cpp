

#include <cassert>
#include "dataflow/serdes.h"
#include "wsanet/wsa_helpers.h"

struct WsaSerializedMessage
{
	uint32_t topicId;
	uint32_t userData;
	uint32_t sourceLength;
	uint32_t payloadLength;
};

uint32_t getSerializedSize(const WsaSerializedMessage& message)
{
	uint32_t size =
		sizeof(WsaSerializedMessage::topicId) +
		sizeof(WsaSerializedMessage::userData) +
		sizeof(WsaSerializedMessage::sourceLength) +
		sizeof(WsaSerializedMessage::payloadLength) +
		message.sourceLength +
		message.payloadLength;

	return size;
}

void serializeMessageWSA(char* pOut, const Dataflow::Message& message, uint32_t* pSerializedMessageSize = nullptr)
{
	WsaSerializedMessage serializedMessage;
	constexpr uint32_t bitsPerField = WsaNetworking::WsaMessageFrame::sPrependLength;

	serializedMessage.topicId = static_cast<uint32_t>(message.topicId);
	memcpy(&serializedMessage.userData, &message.userData, bitsPerField);
	serializedMessage.sourceLength = static_cast<uint32_t>(message.source.size());
	serializedMessage.payloadLength = static_cast<uint32_t>(message.payload.size());

	uint32_t serializedMsgSize = getSerializedSize(serializedMessage) + 1;

	assert(strlen(pOut) >= serializedMsgSize && "the char array must have enough space for the serialized message");

	char* pSerMsgHead = pOut;

	WsaNetworking::storeHostu32(pSerMsgHead, serializedMessage.topicId);
	pSerMsgHead += bitsPerField;

	WsaNetworking::storeHostu32(pSerMsgHead, serializedMessage.userData);
	pSerMsgHead += bitsPerField;

	WsaNetworking::storeHostu32(pSerMsgHead, serializedMessage.sourceLength);
	pSerMsgHead += bitsPerField;

	WsaNetworking::storeHostu32(pSerMsgHead, serializedMessage.payloadLength);
	pSerMsgHead += bitsPerField;

	memcpy(pSerMsgHead, message.source.data(), serializedMessage.sourceLength);
	pSerMsgHead += serializedMessage.sourceLength;

	memcpy(pSerMsgHead, message.payload.data(), serializedMessage.payloadLength);
	pSerMsgHead[serializedMsgSize - 1] = '\0';

	if (pSerializedMessageSize != nullptr)
	{
		*pSerializedMessageSize = serializedMsgSize;
	}
}

void deserializeMessageWSA(Dataflow::Message* pOut, const char* pMessage)
{
	WsaSerializedMessage serializedMessage;
	constexpr uint32_t bitsPerField = WsaNetworking::WsaMessageFrame::sPrependLength;

	const char* pSerMsgHead = pMessage;

	serializedMessage.topicId = WsaNetworking::readHostu32(pSerMsgHead);
	pSerMsgHead += bitsPerField;

	serializedMessage.userData = WsaNetworking::readHostu32(pSerMsgHead);
	pSerMsgHead += bitsPerField;

	serializedMessage.sourceLength = WsaNetworking::readHostu32(pSerMsgHead);
	pSerMsgHead += bitsPerField;

	serializedMessage.payloadLength = WsaNetworking::readHostu32(pSerMsgHead);
	pSerMsgHead += bitsPerField;

	pOut->topicId = static_cast<Dataflow::TopicId>(serializedMessage.topicId);
	memcpy(&pOut->userData, &serializedMessage.userData, bitsPerField);

	pOut->source.assign(pSerMsgHead, pSerMsgHead + serializedMessage.sourceLength);
	pSerMsgHead += serializedMessage.sourceLength;

	pOut->payload.assign(pSerMsgHead, pSerMsgHead + serializedMessage.payloadLength);
}

void Dataflow::SerDes::serializeMessage(char* pOut, const Dataflow::Message& message, uint32_t* pSerializedMessageSize)
{
	serializeMessageWSA(pOut, message, pSerializedMessageSize); // #wip: add support for different serializers
}

void Dataflow::SerDes::deserializeMessage(Dataflow::Message* pOut, const char* pMessage)
{
	deserializeMessageWSA(pOut, pMessage);  // #wip: add support for different deserializers
}