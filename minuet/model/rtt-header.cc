#include "rtt-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RTTPacketHeader");

/*****************  RTTPacketHeader  **********************/
NS_OBJECT_ENSURE_REGISTERED(RTTPacketHeader);

TypeId RTTPacketHeader::GetTypeId() {
    static TypeId typeId = TypeId("ns3::RTTPacketHeader")
                            .SetParent<Header>()
                            .AddConstructor<RTTPacketHeader>();
    return typeId;
}

RTTPacketHeader::RTTPacketHeader() {
    m_messageType = DISCOVERY_MSG; // Valor padrão
    m_timestamp = Seconds(0);
    m_originalTimestamp = Seconds(0);
    m_senderId = 0;
    m_senderState = 0;
    m_senderIp = Ipv4Address();
	NS_LOG_DEBUG("Class RTTPacketHeader RTTPacketHeader Method");
}


RTTPacketHeader::~RTTPacketHeader() {
	NS_LOG_DEBUG("Class RTTPacketHeader: ~RTTPacketHeader Method");
}

TypeId RTTPacketHeader::GetInstanceTypeId(void) const {
	NS_LOG_DEBUG("Class RTTPacketHeader " << this);
	return GetTypeId();
}

void RTTPacketHeader::SetMessageType(RTTMessageType type) {
    m_messageType = static_cast<uint8_t>(type);
}

RTTMessageType RTTPacketHeader::GetMessageType() const {
    return static_cast<RTTMessageType>(m_messageType);
}

void RTTPacketHeader::SetTimestamp(Time ts) {
    m_timestamp = ts;
}

Time RTTPacketHeader::GetTimestamp() const {
    return m_timestamp;
}

void RTTPacketHeader::SetOriginalTimestamp(Time ts) {
    m_originalTimestamp = ts;
}

Time RTTPacketHeader::GetOriginalTimestamp() const {
    return m_originalTimestamp;
}

void RTTPacketHeader::SetSenderId(uint32_t id) {
    m_senderId = id;
}

uint32_t RTTPacketHeader::GetSenderId() const {
    return m_senderId;
}

void RTTPacketHeader::SetSenderState(uint8_t state) {
    m_senderState = state;
}

uint8_t RTTPacketHeader::GetSenderState() const {
    return m_senderState;
}

void RTTPacketHeader::SetSenderIp(Ipv4Address ip) {
    m_senderIp = ip;
}

Ipv4Address RTTPacketHeader::GetSenderIp() const {
    return m_senderIp;
}

uint32_t RTTPacketHeader::GetSerializedSize(void) const {
    return sizeof(uint8_t) // m_messageType
         + sizeof(int64_t) * 2 // m_timestamp e m_originalTimestamp
         + sizeof(uint32_t) // m_senderId
         + sizeof(uint8_t) // m_senderState
         + 4; // m_senderIp
}

void RTTPacketHeader::Serialize(Buffer::Iterator start) const {
    Buffer::Iterator i = start;
    i.WriteU8(m_messageType);
    i.WriteHtonU64(m_timestamp.GetNanoSeconds());
    i.WriteHtonU64(m_originalTimestamp.GetNanoSeconds());
    i.WriteHtonU32(m_senderId);
    i.WriteU8(m_senderState);
    i.WriteHtonU32(m_senderIp.Get());
}

uint32_t RTTPacketHeader::Deserialize(Buffer::Iterator start) {
    Buffer::Iterator i = start;
    m_messageType = i.ReadU8();
    m_timestamp = NanoSeconds(i.ReadNtohU64());
    m_originalTimestamp = NanoSeconds(i.ReadNtohU64());
    m_senderId = i.ReadNtohU32();
    m_senderState = i.ReadU8();
    m_senderIp.Set(i.ReadNtohU32());
    return GetSerializedSize();
}

void RTTPacketHeader::Print(std::ostream &os) const {
    os << "RttPacketHeader(Type=" << (int)m_messageType 
       << ", SenderID=" << m_senderId
       << ", SenderIP=" << m_senderIp
       << ", SenderState=" << (int)m_senderState
       << ", TS=" << m_timestamp 
       << ", OrigTS=" << m_originalTimestamp << ")";
}
/*****************  RTTPacketHeader END  **********************/

}
