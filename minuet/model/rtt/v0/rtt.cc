#include "rtt.h"
#include "rtt-utils.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RTT");

/*************** ClusteringManagerInterface ************************/
NS_OBJECT_ENSURE_REGISTERED (RTTInterface);

RTTInterface::RTTInterface() {
	NS_LOG_FUNCTION(this);
}

RTTInterface::~RTTInterface() {
	NS_LOG_FUNCTION(this);
}

void RTTInterface::AddCommunicationLayer(const Ptr<CommunicationLayer>& comunicationLayer) {
	RTTInterface::m_communicationLayer = comunicationLayer;
	RTTInterface::m_communicationLayer->AttachRTTInterface(this);
}
/*************** END ClusteringManagerInterface ************************/


NS_OBJECT_ENSURE_REGISTERED(RTT);

TypeId RTT::GetTypeId() {
	static TypeId typeId =
			TypeId("ns3::RTT")
			.AddConstructor<RTT>()
			.SetParent<Object>()
			.AddAttribute("Node",
					"The Node of the RTT", PointerValue(),
					MakePointerAccessor(&RTT::m_node),
					MakePointerChecker<Node>())
			.AddAttribute("CommunicationLayer",
					"The Communication Instance", PointerValue(),
					MakePointerAccessor(&RTT::m_communicationLayer),
					MakePointerChecker<CommunicationLayer>());

	return typeId;
}

RTT::RTT () {
	NS_LOG_FUNCTION(this);
	m_clusteringStarted = false;

    m_rand = CreateObject<UniformRandomVariable> ();
    m_rand->SetAttribute("Min", DoubleValue(0.0));
    m_rand->SetAttribute("Max", DoubleValue(1.0));
}

RTT::~RTT() {
	NS_LOG_FUNCTION(this);
}

void RTT::StartClustering () {
	NS_LOG_FUNCTION(this);
	NS_LOG_INFO("RUNNING STARTCLUSTERING!");

    m_unicastSocket = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
    
    m_myId = m_node->GetId();
    m_myIp = m_node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

	m_mobilityModel = m_node->GetObject<MobilityModel>();
	AddCommunicationLayer(m_communicationLayer);

	// Iniciando informações do nó
	m_currentState = IDLE; 
	m_clusterHeadId = m_myId;
    m_lastContactWithCH = Simulator::Now();

	m_clusteringStarted = true;
	PrintInLog("EVENT=NODE_ACTIVE"); 

	m_sendEvent = Simulator::Schedule(Seconds(RTTUtils::DISCOVERY_INTERVAL * m_rand->GetValue()), &RTT::SendPeriodicMessage, this);
    m_cleanEvent = Simulator::Schedule(Seconds(RTTUtils::NEIGHBOR_EXPIRATION_TIME), &RTT::CleanUp, this);
}

void RTT::StopClustering () {
	NS_LOG_FUNCTION(this);
	NS_LOG_INFO("RUNNING STOPCLUSTERING!");

	m_clusteringStarted = false;
	Simulator::Cancel(m_sendEvent);
	Simulator::Cancel(m_cleanEvent);
	PrintInLog("EVENT=NODE_INACTIVE");

    if (m_unicastSocket)
        m_unicastSocket->Close();
}

void RTT::SendPeriodicMessage() {
    if (m_currentState == IDLE || m_currentState == CLUSTER_HEAD) {
        RTTPacketHeader header;
        header.SetMessageType(DISCOVERY_MSG);
        header.SetTimestamp(Simulator::Now());
        header.SetSenderId(m_myId);
        header.SetSenderState(static_cast<uint8_t>(m_currentState));
        header.SetSenderIp(m_myIp);

        Ptr<Packet> packet = Create<Packet>();
        packet->AddHeader(header);
        m_communicationLayer->SendControlMenssage(packet);

        PrintInLog("EVENT=PACKET_SENT;TYPE=DISCOVERY");
    }
    else if (m_currentState == CLUSTER_MEMBER) {
        RTTPacketHeader header;
        header.SetMessageType(HEARTBEAT_MSG);
        header.SetSenderId(m_myId);
        header.SetSenderState(static_cast<uint8_t>(m_currentState));
        header.SetSenderIp(m_myIp);
        
        Ptr<Packet> packet = Create<Packet>();
        packet->AddHeader(header);
        
        m_unicastSocket->Connect(InetSocketAddress(m_clusterHeadIp, MinuetConfig::PORT_CONTROL));
        m_unicastSocket->Send(packet);
        m_unicastSocket->Connect(InetSocketAddress(Ipv4Address::GetBroadcast(), MinuetConfig::PORT_CONTROL));

        PrintInLog("EVENT=PACKET_SENT;TYPE=HEARTBEAT");
    }

    m_sendEvent = Simulator::Schedule(Seconds(RTTUtils::DISCOVERY_INTERVAL), &RTT::SendPeriodicMessage, this);
}

void RTT::ReceiveControlMessage (Ptr<Packet> packet, Address addr) {
	HandlePacket(packet, addr);
}

void RTT::HandlePacket(Ptr<Packet> packet, Address from) {
    RTTPacketHeader header;
    packet->PeekHeader(header);

    uint32_t senderId = header.GetSenderId();
    NodeState senderState = static_cast<NodeState>(header.GetSenderState());
    Ipv4Address senderIp = header.GetSenderIp();

    if (senderId == m_myId) {return;}

    m_neighborIpMap[senderId] = senderIp;

    switch (header.GetMessageType()) {
        case DISCOVERY_MSG:
        {
            if (m_currentState == CLUSTER_MEMBER && senderId == m_clusterHeadId) {
                m_lastContactWithCH = Simulator::Now();
            }

            if (m_currentState == CLUSTER_HEAD && senderState == CLUSTER_HEAD && senderId < m_myId) {
                m_currentState = IDLE;
                m_clusterMembers.clear();
                PrintInLog("EVENT=CH_RENOUNCED;CH_ID=" + std::to_string(m_myId) + ";REASON=CONFLICT");
            }

            if (m_currentState == IDLE) {
                RTTPacketHeader replyHeader;
                replyHeader.SetMessageType(REPLY_MSG);
                replyHeader.SetOriginalTimestamp(header.GetTimestamp());
                replyHeader.SetSenderId(m_myId);
                replyHeader.SetSenderState(static_cast<uint8_t>(m_currentState));
                replyHeader.SetSenderIp(m_myIp);

                Ptr<Packet> replyPacket = Create<Packet>();
                replyPacket->AddHeader(replyHeader);
                m_communicationLayer->SendControlMenssage(replyPacket);

                PrintInLog("EVENT=PACKET_SENT;TYPE=REPLY");
            }
            break;
        }
        case REPLY_MSG:
        {
            Time rtt = Simulator::Now() - header.GetOriginalTimestamp();
            PrintInLog("EVENT=RTT_MEASUREMENT;FROM=" + std::to_string(senderId) + ";TO=" + std::to_string(m_myId) + ";RTT=" + std::to_string(rtt.GetSeconds()));

            if (rtt.GetSeconds() <= RTTUtils::RTT_THRESHOLD) {
                if (m_currentState == IDLE) {
                    m_currentState = CLUSTER_HEAD;
                    m_clusterHeadId = m_myId;
                    PrintInLog("EVENT=CH_ELECTED;CH_ID=" + std::to_string(m_myId));
                }
                
                if (m_currentState == CLUSTER_HEAD && senderState == IDLE) {
                    RTTPacketHeader inviteHeader;
                    inviteHeader.SetMessageType(INVITE_MSG);
                    inviteHeader.SetSenderId(m_myId);
                    inviteHeader.SetSenderState(static_cast<uint8_t>(m_currentState));
                    inviteHeader.SetSenderIp(m_myIp);

                    Ptr<Packet> invitePacket = Create<Packet>();
                    invitePacket->AddHeader(inviteHeader);
                    Ipv4Address destinationIp = m_neighborIpMap[senderId];
                    m_unicastSocket->Connect(InetSocketAddress(destinationIp, MinuetConfig::PORT_CONTROL));
                    m_unicastSocket->Send(invitePacket);
                    m_unicastSocket->Connect(InetSocketAddress(Ipv4Address::GetBroadcast(), MinuetConfig::PORT_CONTROL));

                    PrintInLog("EVENT=PACKET_SENT;TYPE=INVITE");
                }
            }
            break;
        }
        case INVITE_MSG:
        {
            if (m_currentState == IDLE && senderState == CLUSTER_HEAD) { 
                m_currentState = CLUSTER_MEMBER;
                m_clusterHeadId = senderId;
                m_clusterHeadIp = senderIp;
                m_lastContactWithCH = Simulator::Now(); 

                PrintInLog("EVENT=MEMBER_JOIN;CH_ID=" + std::to_string(senderId) + ";MEMBER_ID=" + std::to_string(m_myId));
            }
            break;
        }
        case HEARTBEAT_MSG:
        {
            if (m_currentState == CLUSTER_HEAD) {
                if (m_clusterMembers.find(senderId) == m_clusterMembers.end()) {
                    PrintInLog("EVENT=MEMBER_JOIN;CH_ID=" + std::to_string(m_myId) + ";MEMBER_ID=" + std::to_string(senderId) + ";REASON=HEARTBEAT");
                }
                m_clusterMembers[senderId] = Simulator::Now();
            }
            break;
        }
        default:
            break;
    }
}

bool RTT::IsClusterHead () {
	return m_currentState == CLUSTER_HEAD;
}

bool RTT::IsClusterMember () {
	return m_currentState == CLUSTER_MEMBER;
}

bool RTT::IsIsolated () {
	return m_currentState == IDLE;
}

uint32_t RTT::GetClusterId() {
	return m_clusterHeadId;
}

bool RTT::IsStarted() {
	return m_clusteringStarted;
}

void RTT::CleanUp() {
    if (m_currentState == CLUSTER_HEAD) {
        PrintInLog("EVENT=CLUSTER_SIZE;CH_ID=" + std::to_string(m_myId) + ";SIZE=" + std::to_string(m_clusterMembers.size()));
        for (auto it = m_clusterMembers.begin(); it != m_clusterMembers.end(); ) {
            if (Simulator::Now() > (it->second + Seconds(RTTUtils::NEIGHBOR_EXPIRATION_TIME))) {
                PrintInLog("EVENT=MEMBER_LEAVE;CH_ID=" + std::to_string(m_myId) + ";MEMBER_ID=" + std::to_string(it->first) + ";REASON=TIMEOUT");
                it = m_clusterMembers.erase(it);
            }
            else {
                ++it;
            }
        }
    } 
    else if (m_currentState == CLUSTER_MEMBER) {
        if (Simulator::Now() > (m_lastContactWithCH + Seconds(RTTUtils::NEIGHBOR_EXPIRATION_TIME))) {
            PrintInLog("EVENT=MEMBER_LEAVE;CH_ID=" + std::to_string(m_clusterHeadId) + ";MEMBER_ID=" + std::to_string(m_myId) + ";REASON=CH_TIMEOUT");
            m_currentState = IDLE;
            m_clusterHeadId = m_myId;
        }
    }
    m_cleanEvent = Simulator::Schedule(Seconds(RTTUtils::NEIGHBOR_EXPIRATION_TIME), &RTT::CleanUp, this);
}

void RTT::PrintInLog(string message) {
	ofstream os;
	os.open (MinuetConfig::LOG_FILE_CLUSTERING_ALGORITHM.c_str(), ofstream::out | ofstream::app);
	os << Simulator::Now().GetSeconds() << "s - RTT - Node #" << m_node->GetId() << " : " << message << endl;
	os.close();
}

}