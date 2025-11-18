

#include <cassert>
#include "dataflow/serdes.h"
#include "wsanet/wsa_common.h"

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

void Dataflow::SerDes::serializeMessageWsa(char* pOut, const Dataflow::Message& message, uint32_t* pSerializedMessageSize)
{
	WsaSerializedMessage serializedMessage;
	constexpr uint32_t bytesPerField = WsaNetworking::WsaMessageFrame::sPrependLength;

	serializedMessage.topicId = static_cast<uint32_t>(message.topicId);
	memcpy(&serializedMessage.userData, &message.userData, bytesPerField);
	serializedMessage.sourceLength = static_cast<uint32_t>(message.source.size());
	serializedMessage.payloadLength = static_cast<uint32_t>(message.payload.size());

	uint32_t serializedMsgSize = getSerializedSize(serializedMessage) + 1;

	assert(pOut != nullptr && strlen(pOut) >= serializedMsgSize && "the char array must have enough space for the serialized message");

	char* pSerMsgHead = pOut;

	WsaNetworking::Common::storeHostu32(pSerMsgHead, serializedMessage.topicId);
	pSerMsgHead += bytesPerField;

	WsaNetworking::Common::storeHostu32(pSerMsgHead, serializedMessage.userData);
	pSerMsgHead += bytesPerField;

	WsaNetworking::Common::storeHostu32(pSerMsgHead, serializedMessage.sourceLength);
	pSerMsgHead += bytesPerField;

	WsaNetworking::Common::storeHostu32(pSerMsgHead, serializedMessage.payloadLength);
	pSerMsgHead += bytesPerField;

	memcpy(pSerMsgHead, message.source.data(), serializedMessage.sourceLength);
	pSerMsgHead += serializedMessage.sourceLength;

	memcpy(pSerMsgHead, message.payload.data(), serializedMessage.payloadLength);
	pSerMsgHead[serializedMsgSize - 1] = '\0';

	if (pSerializedMessageSize != nullptr)
	{
		*pSerializedMessageSize = serializedMsgSize;
	}
}

void Dataflow::SerDes::deserializeMessageWsa(Dataflow::Message* pOut, const char* pMessage)
{
	WsaSerializedMessage serializedMessage;
	constexpr uint32_t bytesPerField = WsaNetworking::WsaMessageFrame::sPrependLength;

	const char* pSerMsgHead = pMessage;

	serializedMessage.topicId = WsaNetworking::Common::readHostu32(pSerMsgHead);
	pSerMsgHead += bytesPerField;

	serializedMessage.userData = WsaNetworking::Common::readHostu32(pSerMsgHead);
	pSerMsgHead += bytesPerField;

	serializedMessage.sourceLength = WsaNetworking::Common::readHostu32(pSerMsgHead);
	pSerMsgHead += bytesPerField;

	serializedMessage.payloadLength = WsaNetworking::Common::readHostu32(pSerMsgHead);
	pSerMsgHead += bytesPerField;

	pOut->topicId = static_cast<Dataflow::TopicId>(serializedMessage.topicId);
	memcpy(&pOut->userData, &serializedMessage.userData, bytesPerField);

	pOut->source.assign(pSerMsgHead, pSerMsgHead + serializedMessage.sourceLength);
	pSerMsgHead += serializedMessage.sourceLength;

	pOut->payload.assign(pSerMsgHead, pSerMsgHead + serializedMessage.payloadLength);
}