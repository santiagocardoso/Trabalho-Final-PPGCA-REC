#ifndef RTT_HEADER_H
#define RTT_HEADER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/nstime.h"
#include "ns3/header.h"
#include "ns3/internet-module.h"


namespace ns3 {

enum RTTMessageType {
    DISCOVERY_MSG,
    REPLY_MSG,
    INVITE_MSG,
    HEARTBEAT_MSG
};

class RTTPacketHeader: public Header {

public:
	RTTPacketHeader();
	virtual ~RTTPacketHeader();

	static TypeId GetTypeId();

	void SetMessageType(RTTMessageType type);
    RTTMessageType GetMessageType() const;

    void SetTimestamp(Time ts);
    Time GetTimestamp() const;

    void SetOriginalTimestamp(Time ts);
    Time GetOriginalTimestamp() const;

    void SetSenderId(uint32_t id);
    uint32_t GetSenderId() const;

    void SetSenderState(uint8_t state);
    uint8_t GetSenderState() const;

    void SetSenderIp(Ipv4Address ip);
    Ipv4Address GetSenderIp() const;

    virtual TypeId GetInstanceTypeId(void) const;
    virtual void Print(std::ostream &os) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);

private:
	uint8_t m_messageType;
    Time    m_timestamp;
    Time    m_originalTimestamp;
    uint32_t m_senderId;
    uint8_t  m_senderState;
    Ipv4Address m_senderIp;
};

}

#endif
