#ifndef RTT_H
#define RTT_H

#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "rtt-header.h"
#include "rtt-utils.h"
#include "minuet-communication-layer.h"

namespace ns3 {

class CommunicationLayer;

class RTTInterface : public Object {
public:
	RTTInterface();
	virtual ~RTTInterface();

	virtual void StartClustering() = 0;
	virtual void StopClustering(void) = 0;

	virtual bool IsClusterHead() = 0;
	virtual bool IsClusterMember() = 0;
	virtual bool IsIsolated() = 0;
	virtual uint32_t GetClusterId() = 0;
	virtual bool IsStarted() = 0;

	virtual void ReceiveControlMessage(Ptr<Packet> packet, Address addr) = 0;

	void AddCommunicationLayer(const Ptr<CommunicationLayer>& comunicationLayer);

protected:
	Ptr<CommunicationLayer> m_communicationLayer;
};

class RTT : public RTTInterface {

public:
	static TypeId GetTypeId();

	RTT();
	virtual ~RTT();

    virtual void StartClustering(void);
    virtual void StopClustering(void);

    virtual bool IsClusterHead();
	virtual bool IsClusterMember();
	virtual bool IsIsolated();
	virtual uint32_t GetClusterId();
	virtual bool IsStarted();

private:
    enum NodeState {
        IDLE,
        CLUSTER_HEAD,
        CLUSTER_MEMBER
    };

    virtual void ReceiveControlMessage(Ptr<Packet> packet, Address addr);
    void HandlePacket(Ptr<Packet> packet, Address from);
    void SendPeriodicMessage();
	void CleanUp();

    void PrintInLog(string message);

	///// Attributes Node  //////
	Ptr<Node> m_node;
	NodeState m_currentState;
	uint32_t m_clusterHeadId;
	Ptr<MobilityModel> m_mobilityModel;
    uint32_t m_myId;

    Ipv4Address m_myIp;
    Ptr<Socket> m_unicastSocket;
    Ipv4Address m_clusterHeadIp;
	////////////////////////////

	std::map<uint32_t, Time> m_clusterMembers;
    std::map<uint32_t, Ipv4Address> m_neighborIpMap;

	bool m_clusteringStarted;

	EventId m_sendEvent;
	EventId m_cleanEvent;

    Time m_lastContactWithCH;
    Time m_timeElectedAsCH;

    Ptr<UniformRandomVariable> m_rand;
};

}


#endif
