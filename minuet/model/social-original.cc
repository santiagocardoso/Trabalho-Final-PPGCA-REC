#include "src/core/model/rng-seed-manager.h"
#include "social.h"
#include "minuet-utils.h"
#include "time.h"
#include <stdlib.h>     /* srand, rand */
#include <sys/stat.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SOCIAL");

/*************** ClusteringManagerInterface ************************/
NS_OBJECT_ENSURE_REGISTERED (SOCIALInterface);

SOCIALInterface::SOCIALInterface() {
	//NS_LOG_FUNCTION(this);
}

SOCIALInterface::~SOCIALInterface() {
	NS_LOG_FUNCTION(this);
}

void SOCIALInterface::AddCommunicationLayer(const Ptr<CommunicationLayer>& comunicationLayer) {
	SOCIALInterface::m_communicationLayer = comunicationLayer;
	SOCIALInterface::m_communicationLayer->AttachSOCIALInterface(this);
}
/*************** END ClusteringManagerInterface ************************/

/*************** VehicleSOCIAL ************************/

VehicleSOCIAL::VehicleSOCIAL(){/* Empty */}

VehicleSOCIAL::VehicleSOCIAL(uint16_t id, uint32_t score, bool isRelay, uint16_t relayId, Vector position, bool isPerfilSocial, bool isGateway, uint32_t eigenScore, float closenessScore, vector<vector<uint16_t>> neighborsList, uint16_t atividadeSocial) { //Todas
//VehicleSOCIAL::VehicleSOCIAL(uint16_t id, uint32_t score, bool isRelay, uint16_t relayId, Vector position, bool isPerfilSocial, bool isGateway, uint32_t eigenScore, float closenessScore, vector<vector<uint16_t>> neighborsList) { //STR
//VehicleSOCIAL::VehicleSOCIAL(uint16_t id, uint32_t atividadeSocial, bool isRelay, uint16_t relayId, Vector position, bool isPerfilSocial, bool isGateway) { //Atividade Social
//VehicleSOCIAL::VehicleSOCIAL(uint16_t id, uint32_t amizade, bool isRelay, uint16_t relayId, Vector position, bool isPerfilSocial, bool isGateway) { //Amizade
	m_id = id;
	m_score = score; //STR
	m_eigenScore = eigenScore; //STR
	m_closenessScore = closenessScore; //STR
	m_atividadeSocial = atividadeSocial; //Atividade Social
	//m_amizade = amizade; //Amizade
	m_isRelay = isRelay;
	m_relayId = relayId;
	m_position = position;
	m_isPerfilSocial = isPerfilSocial;
	m_isGateway = isGateway;
	m_neighborsList = neighborsList; //STR
	m_timeExpire = Simulator::Now().GetSeconds();
}

VehicleSOCIAL::~VehicleSOCIAL(){/* Empty */}

uint16_t VehicleSOCIAL::GetRelayId() const {
	return m_relayId;
}

void VehicleSOCIAL::SetRelayId(uint16_t relayId) {
	m_relayId = relayId;
}

Vector VehicleSOCIAL::GetDirection() const {
	return m_direction;
}

void VehicleSOCIAL::SetDirection(Vector direction) {
	m_direction = direction;
}

uint16_t VehicleSOCIAL::GetId() const {
	return m_id;
}

void VehicleSOCIAL::SetId(uint16_t id) {
	m_id = id;
}

bool VehicleSOCIAL::IsRelay() const {
	return m_isRelay;
}

void VehicleSOCIAL::SetIsRelay(bool isRelay) {
	m_isRelay = isRelay;
}

bool VehicleSOCIAL::IsGateway() const {
	return m_isGateway;
}

void VehicleSOCIAL::SetIsGateway(bool isGateway) {
	m_isGateway = isGateway;
}

Vector VehicleSOCIAL::GetPosition() const {
	return m_position;
}

void VehicleSOCIAL::SetPosition(const Vector position) {
	m_position = position;
}

uint32_t VehicleSOCIAL::GetScore() const {
	return m_score;
}

void VehicleSOCIAL::SetScore(uint32_t score) {
	m_score = score;
}

uint32_t VehicleSOCIAL::GetEigenScore() const {
	return m_eigenScore;
}

void VehicleSOCIAL::SetEigenScore(uint32_t eigenScore) {
	m_eigenScore = eigenScore;
}

float VehicleSOCIAL::GetClosenessScore() const {
	return m_closenessScore;
}

void VehicleSOCIAL::SetClosenessScore(float closenessScore) {
	m_closenessScore = closenessScore;
}

uint16_t VehicleSOCIAL::GetAtividadeSocial() const {
	return m_atividadeSocial;
}

void VehicleSOCIAL::SetAtividadeSocial(uint16_t atividadeSocial) {
	m_atividadeSocial = atividadeSocial;
}

double_t VehicleSOCIAL::GetTimeExpire() const {
	return m_timeExpire;
}

bool VehicleSOCIAL::IsPerfilSocial() const {
	return m_isPerfilSocial;
}

void VehicleSOCIAL::SetIsPerfilSocial(bool isPerfilSocial) {
	m_isPerfilSocial = isPerfilSocial;
}

void VehicleSOCIAL::SetTimeExpire(double_t timeExpire) {
	m_timeExpire = timeExpire;
}

vector<vector<uint16_t>> VehicleSOCIAL::GetNeighborsList() const{
	return m_neighborsList;
}

/*************** END VehicleSOCIAL ************************/

NS_OBJECT_ENSURE_REGISTERED(SOCIAL);

TypeId SOCIAL::GetTypeId() {
	static TypeId typeId =
			TypeId("ns3::SOCIAL")
			.AddConstructor<SOCIAL>()
			.SetParent<Object>()
			.AddAttribute("Node",
					"The Node of the SOCIAL", PointerValue(),
					MakePointerAccessor(&SOCIAL::m_node),
					MakePointerChecker<Node>())
			.AddAttribute("CommunicationLayer",
					"The Communication Instance", PointerValue(),
					MakePointerAccessor(&SOCIAL::m_communicationLayer),
					MakePointerChecker<CommunicationLayer>());

	return typeId;
}

SOCIAL::SOCIAL () {
	//NS_LOG_FUNCTION(this);
	m_sentCounter = 0;
	m_outOfSim = true;
}

SOCIAL::~SOCIAL() {
	//NS_LOG_FUNCTION(this);
}

/******************** Novos critérios **********************/
// std::vector<std::vector<int>>  criterios;
std::map<uint16_t, Ptr<Node>> nodeMap;

/*
 * Matriz de critérios

 * Velocidade média em estradas e rodovias
 * Tempo total dirigido em horário de pico
 * Tempo dirigido total durante a sessão
 * Quilometragem percorrida durante a sessão
 * Tempo habilitado
 * Cometimento de infrações de trânsito
 * Ano de fabricação do veículo
 * Potência do veículo x Idade do motorista

 * Linha = critérios
 * Colunas = nós e valores
*/

//time_t now = time(0);
//tm* timeinfo = localtime(&now);
//int year = 1900 + timeinfo->tm_year;

void SOCIAL::StartClustering () {
	//NS_LOG_FUNCTION(this);
	//NS_LOG_INFO("RUNNING STARTCLUSTERING!" << m_node->GetId());

	m_mobilityModel = m_node->GetObject<MobilityModel>();
	nodeMap[m_node->GetId()] = m_node;
	
	m_outOfSim = false;

	AddCommunicationLayer(m_communicationLayer);

	// Iniciando informações do nó
	m_isRelay = false;
	m_relayId = NaN; //m_node->GetId();
	m_isGateway = false;

	m_score = 0;
	m_eigenScore = 0;
	m_closenessScore = 0;

	m_atividadeSocial = 0;


	// Iniciando peso das métricas
	wBC = wCC = wDC = wEC = 0.25;


	int x = MinuetConfig::RandonNumberGeneratorBetweenRange(0, 100);

	/*RngSeedManager::SetSeed (m_node->GetId());
	RngSeedManager::SetRun(1);
	int x = RngSeedManager::GetNextStreamIndex();*/

	if ((x % 100) < 90){
		//NS_LOG_INFO("Menor que 90: " << x);
		m_isPerfilSocial = true;
	}
	else{
		//NS_LOG_INFO("Maior que 90: " << x);
		m_isPerfilSocial = true;
	}

	//if(!m_isPerfilSocial)
		//NS_LOG_INFO("É falso");

	MinuetConfig::SetTotalNodes(MinuetConfig::GetTotalNodes() + 1);

	PrintInLog("SOCIAL Algorithm Started! " + std::to_string(MinuetConfig::GetTotalNodes()) + " na comunidade.");
	//NS_LOG_INFO("SOCIAL Algorithm Started! " << std::to_string(MinuetConfig::GetTotalNodes()) << " na comunidade.");

	m_sendEvent = Simulator::Schedule (Seconds(m_node->GetId() * 0.0005), &SOCIAL::SendBeacon, this);
	//Simulator::Schedule (Seconds(m_node->GetId() * 0.000001), &SOCIAL::SendBeacon, this);
//	SendBeacon();

	// Agendamento de manutenção de listas de cluster e eventos agendados
	m_cleanEvent = Simulator::Schedule (Seconds(m_node->GetId() * 0.0005), &SOCIAL::CleanUp, this);
	m_maintenanceEvent = Simulator::Schedule (Seconds(m_node->GetId() * 0.0005), &SOCIAL::EventsListMaintenance, this);
	//m_neighborMaintenanceEvent = Simulator::Schedule (Seconds(m_node->GetId() * 0.001), &SOCIAL::NeighborListMaintenance, this);

	m_clusteringStarted = true;

	/*
	// Criteria initialization
	const char* folderPath = "/root/ns3/ns-allinone-3.29/ns-3.29/src/minuet/utils/trace/";
	const std::string filePath = std::string(folderPath) + "random.txt";

	std::ifstream inFile(filePath);
	if (inFile.is_open()) {
		std::string line;
		while (std::getline(inFile, line)) {
            std::vector<int> lineValues;
            std::istringstream stream(line);
            int value;
            while (stream >> value) {
                lineValues.push_back(value);
            }
            criterios.push_back(lineValues);
        }
	}
	*/
}

void SOCIAL::StopClustering () {
	//NS_LOG_FUNCTION(this);
	//NS_LOG_INFO("RUNNING STOPCLUSTERING! " << m_node->GetId());

	//Flag declarando-se fora da simulação
	m_outOfSim = true;

	//Cancelando eventos ativos
	m_sendEvent.Cancel();
	m_cleanEvent.Cancel();
	m_maintenanceEvent.Cancel();

	if (scheduledCheckEvents.size() != 0) {
		for (EventId eventId : scheduledCheckEvents) {
			eventId.Cancel();
		}
		scheduledCheckEvents.clear();
	}

	//cout << m_node->GetId();

	m_clusteringStarted = false;
	MinuetConfig::SetTotalNodes(MinuetConfig::GetTotalNodes() - 1);

	//NS_LOG_INFO("SOCIAL Algorithm Stopped! " << std::to_string(MinuetConfig::GetTotalNodes()) << " na comunidade.");

	PrintInLog("SOCIAL Algorithm Stopped! " + std::to_string(MinuetConfig::GetTotalNodes()) + " na comunidade.");
}

void SOCIAL::ReceiveControlMessage (Ptr<Packet> packet, Address addr) {
	//NS_LOG_FUNCTION(this);
	//NS_LOG_INFO("MESSAGE RECEIVED!");

	time_t ini, end;

	time(&ini);

	if (!m_outOfSim) {
		BeaconHeader beaconHeader;
		BeaconHeader::VehicleInfo vehicleInfo;

		packet->RemoveHeader(beaconHeader);

		//if(beaconHeader.GetLenNeighborsList() == beaconHeader.GetNeighborsList().size()){
			vehicleInfo = beaconHeader.GetVehicleInfo();
			//cout << vehicleInfo.direction.x << " " << vehicleInfo.direction.y << " " << vehicleInfo.direction.z << endl;
			VehicleSOCIAL vehicle2(vehicleInfo.id, vehicleInfo.score, vehicleInfo.isRelay, vehicleInfo.relayId, vehicleInfo.position, vehicleInfo.isPerfilSocial, vehicleInfo.isGateway, vehicleInfo.eigenScore, vehicleInfo.closenessScore, beaconHeader.GetNeighborsList(), vehicleInfo.atividadeSocial); //Todas

			//VehicleSOCIAL vehicle2(vehicleInfo.id, vehicleInfo.score, vehicleInfo.isRelay, vehicleInfo.relayId, vehicleInfo.position, vehicleInfo.isPerfilSocial, vehicleInfo.isGateway, vehicleInfo.eigenScore, vehicleInfo.closenessScore, beaconHeader.GetNeighborsList(), vehicleInfo.atividadeSocial, 1); //FCV

			vehicle2.SetDirection(vehicleInfo.direction);
			//VehicleSOCIAL vehicle2(vehicleInfo.id, vehicleInfo.score, vehicleInfo.isRelay, vehicleInfo.relayId, vehicleInfo.position, vehicleInfo.isPerfilSocial, vehicleInfo.isGateway, vehicleInfo.eigenScore, vehicleInfo.closenessScore, beaconHeader.GetNeighborsList()); //STR
			//VehicleSOCIAL vehicle2(vehicleInfo.id, vehicleInfo.atividadeSocial, vehicleInfo.isRelay, vehicleInfo.relayId, vehicleInfo.position, vehicleInfo.isPerfilSocial, vehicleInfo.isGateway); //Atividade Social
			//VehicleSOCIAL vehicle2(vehicleInfo.id, vehicleInfo.amizade, vehicleInfo.isRelay, vehicleInfo.relayId, vehicleInfo.position, vehicleInfo.isPerfilSocial, vehicleInfo.isGateway); //Amizade

			PrintInLog("Receive Control Message: Beacon");

			HandleBeacon(vehicle2);
		/*} else {
			vector<vector<uint16_t>> auxList;
			bool isOnPacketList = false;
			for (uint32_t j=0; j<packetList.size(); j++){
				if((beaconHeader.GetSeq() == (packetList[j].GetSeq()+1) || beaconHeader.GetSeq() == (packetList[j].GetSeq()-1)) && (beaconHeader.GetVehicleInfo().id == packetList[j].GetVehicleInfo().id)){
					isOnPacketList = true;
					if(beaconHeader.GetSeq() < packetList[j].GetSeq()){
						auxList = beaconHeader.GetNeighborsList();
						for (uint32_t i=0; i<packetList[j].GetNeighborsList().size(); i++){
							 auxList.push_back(packetList[j].GetNeighborsList()[i]);
						}
					} else {
						auxList = packetList[j].GetNeighborsList();
						for (uint32_t i=0; i<beaconHeader.GetNeighborsList().size(); i++){
							 auxList.push_back(beaconHeader.GetNeighborsList()[i]);
						}
					}
					packetList.erase(packetList.begin() + j);
					break;
				}
			}
			if(isOnPacketList){
				vehicleInfo = beaconHeader.GetVehicleInfo();

				VehicleSOCIAL vehicle2(vehicleInfo.id, vehicleInfo.score, vehicleInfo.isRelay, vehicleInfo.relayId, vehicleInfo.position, vehicleInfo.isPerfilSocial, vehicleInfo.isGateway, vehicleInfo.eigenScore, vehicleInfo.closenessScore, auxList);

				PrintInLog("Receive Control Message: Beacon");

				HandleBeacon(vehicle2);
			} else {
				packetList.push_back(beaconHeader);
			}
		}*/

		//NS_LOG_DEBUG(Simulator::Now().GetSeconds());
		if (int(Simulator::Now().GetSeconds()) == 29000){
			string perfil = (m_isPerfilSocial && vehicle2.IsPerfilSocial()) ? "1" : "0";
			NS_LOG_DEBUG(perfil);
			PrintInGetEdgesOnAnInstant(std::to_string(m_node->GetId()) + "," + std::to_string(vehicle2.GetId()) + ",undirected,1," + perfil);
		}

	}

	time(&end);

	//double tempo = end - ini;

	//std::cout << "A função ReceiveControlMessage foi executada em " << tempo << " segundos.\n";
}

void SOCIAL::HandleBeacon (VehicleSOCIAL vehicle2) {
	//NS_LOG_FUNCTION(this);
	// printf("Beacon recebido %i: %zu %zu\n", m_node->GetId(), closeNeighbors.size());

	time_t ini, end;

	time(&ini);

	if(m_node->GetId() == 1265){
		//Instrução vazia
		NS_LOG_DEBUG("1265");
	}

	if(!m_isPerfilSocial)
		return;


	//NS_LOG_INFO("HANDLING BEACON FROM NODE: " << vehicle2.GetId());

	double_t separation = CalculateDistance(m_mobilityModel->GetPosition(), vehicle2.GetPosition());

	//PrintInLog("Handling Beacon From Node #" + std::to_string(vehicle2.GetId()) + " AngleDiff: " + std::to_string(angleDiff) + " Separation: " + std::to_string(separation));

	bool isCloseNeighbor = false;
	bool isGateway = false;
	vector<VehicleSOCIAL>::iterator vehicle;
	vector<VehicleSOCIAL>::iterator closeNeighbor;

	for (closeNeighbor = closeNeighbors.begin() ; closeNeighbor != closeNeighbors.end() ; closeNeighbor++) {
		if ((*closeNeighbor).GetId() == vehicle2.GetId()) {
			isCloseNeighbor = true;
			break;
		}
	}
//cout << vehicle2.GetId() << ": " << vehicle2.GetDirection().x << " " << vehicle2.GetDirection().y << " " << vehicle2.GetDirection().z << " " << endl;
	if (separation < SOCIALUtils::MAX_SEPARATION_CLOSENEIGHBORS && vehicle2.IsPerfilSocial() == true) {
		if(!isCloseNeighbor) {
			//NS_LOG_INFO("CLUSTERING NODE "<< vehicle2.GetId() <<" INTO "<< m_node->GetId() <<" CLOSENEIGHBORS WITH SEPARATION: " << separation);
			PrintInLog("Adding Node #" + std::to_string(vehicle2.GetId()) + " in Close Neighbors List");
			cout << "Adding Node #" << std::to_string(vehicle2.GetId()) << " in Close Neighbors List" << endl;
			closeNeighbors.push_back(vehicle2);

			//if(!m_isPerfilSocial)
			//	return;

			bool isNewNeighbor = false;
			for (uint32_t i = 0; i<novosVizinhos.size(); i++){
				if (novosVizinhos[i][0] == vehicle2.GetId()){
					isNewNeighbor = true;
					break;
				}
			}

			if(!isNewNeighbor){
				vector<uint16_t> viz;
				viz.push_back(vehicle2.GetId());
				viz.push_back((uint16_t) Simulator::Now().GetSeconds());
				//NS_LOG_INFO("Veículo " << viz[0] << " adicionado à novos vizinhos de " << m_node->GetId() << " no instante " << (uint16_t) Simulator::Now().GetSeconds() << " segundos");
				novosVizinhos.push_back(viz);
			}


		} else {
			//NS_LOG_INFO("UPDATE NODE "<< vehicle2.GetId() <<" INTO "<< m_node->GetId() <<" CLOSENEIGHBORS");
			PrintInLog("Update Node #" + std::to_string(vehicle2.GetId()) + " in Close Neighbors List");

			if((*closeNeighbor).IsGateway()){
				isGateway = true;
				//NS_LOG_INFO(vehicle2.GetId() << " já era gateway");
			}

			(*closeNeighbor).SetDirection(vehicle2.GetDirection());
			(*closeNeighbor).SetPosition(vehicle2.GetPosition());
			(*closeNeighbor).SetIsRelay(vehicle2.IsRelay());
			(*closeNeighbor).SetRelayId(vehicle2.GetRelayId());
			(*closeNeighbor).SetScore(vehicle2.GetScore());
			(*closeNeighbor).SetIsPerfilSocial(vehicle2.IsPerfilSocial());
			(*closeNeighbor).SetIsGateway(vehicle2.IsGateway());
			(*closeNeighbor).SetEigenScore(vehicle2.GetEigenScore());
			(*closeNeighbor).SetClosenessScore(vehicle2.GetClosenessScore());
			(*closeNeighbor).SetTimeExpire((*closeNeighbor).GetTimeExpire() + SOCIALUtils::EXPIRY_TIME_INTERVAL_1);

		}

		if (MinuetConfig::wSTR != 0){

			//Verifica se o vehicle2 é novo gateway (eigenscore++) ou não é mais gateway (eigenscore--)
			if (!isGateway && vehicle2.IsGateway()){ //Verifica se o nó não era gateway e agora é
				m_eigenScore++;
				//NS_LOG_INFO(vehicle2.GetId() << " não era gateway e agora é!");
			} else if (isGateway && !vehicle2.IsGateway()){ //Verifica se o nó era gateway e agora não é mais
				m_eigenScore--;
				//NS_LOG_INFO(vehicle2.GetId() << " era gateway e agora não é mais!");
			}

			//Verifica se vehicle2, que foi adicionado ou atualizado recentemente ao closeNeighbors, já existe em m_neighborsList.
			//Não é necessário verificar todos os closeneighbors, apenas o vehicle2.
			//for (closeNeighbor = closeNeighbors.begin() ; closeNeighbor != closeNeighbors.end() ; closeNeighbor++) {

			vector<vector<uint16_t>> auxListV2 = vehicle2.GetNeighborsList();

			//NS_LOG_DEBUG("Início: " << auxListV2.size() << ", " << vehicle2.GetNeighborsList().size() << ", " << m_neighborsList.size());

			for (uint32_t j = 0 ; j< auxListV2.size() ; j++) {
				if(auxListV2[j][0] == m_node->GetId()){
					auxListV2.erase(auxListV2.begin() + j);
					j--;
					continue;
				}

				if(auxListV2[j][1]>3){
					auxListV2.erase(auxListV2.begin() + j);
					j--;
					continue;
				}
			}

			bool isNeighbor = false;
			for(uint32_t i = 0; i<m_neighborsList.size(); i++){

				if(m_neighborsList[i][2] == vehicle2.GetId()){
					isNeighbor = true;
					m_neighborsList[i][0] = vehicle2.GetId();
					m_neighborsList[i][1] = 1;
					continue;
				}

				bool isOnNeighborList = false;
				if(m_neighborsList[i][0] == vehicle2.GetId() && (m_neighborsList[i][1] > 1) ){
					for (uint32_t j = 0 ; j< auxListV2.size() ; j++) {
						if(m_neighborsList[i][2] == auxListV2[j][2]){
							if(auxListV2[j][1]<4){
								m_neighborsList[i][1] = auxListV2[j][1] + 1;
								auxListV2.erase(auxListV2.begin() + j);
								isOnNeighborList = true;
								break;
							}
							else{
								auxListV2.erase(auxListV2.begin() + j);
								isOnNeighborList = false;
								break;
							}

						}
					}
					if(!isOnNeighborList){
						m_neighborsList.erase(m_neighborsList.begin() + i);
					}
				} else {
					for (uint32_t j = 0; j<auxListV2.size(); j++){
						if (m_neighborsList[i][2] == auxListV2[j][2]){
							if(((auxListV2[j][1] + 1) < m_neighborsList[i][1]) && ((auxListV2[j][1] + 1) < 4 )){
								m_neighborsList[i][0] = vehicle2.GetId();
								m_neighborsList[i][1] = auxListV2[j][1] + 1;
								m_neighborsList[i][2] = auxListV2[j][2];

							}
							auxListV2.erase(auxListV2.begin() + j);
							break;
						}
					}
				}
			}

			if(!isNeighbor){
				vector<uint16_t> m_pathList;
				m_pathList.push_back(vehicle2.GetId());
				m_pathList.push_back(1);
				m_pathList.push_back(vehicle2.GetId());

				m_neighborsList.push_back(m_pathList);
			}

			//NS_LOG_DEBUG("Após atualizar: " << auxListV2.size() << ", " << m_neighborsList.size());

			for (uint32_t i = 0; i < auxListV2.size(); i++){
				if(m_neighborsList.size() > 70)
					break;
				vector<uint16_t> m_pathList;
				m_pathList.push_back(vehicle2.GetId());
				m_pathList.push_back(auxListV2[i][1] + 1);
				m_pathList.push_back(auxListV2[i][2]);

				m_neighborsList.push_back(m_pathList);

				auxListV2.erase(auxListV2.begin() + i);
				i--;
			}

			//NS_LOG_DEBUG("Final: " << auxListV2.size() << ", " << m_neighborsList.size());

		}
		/*bool isOnNeighborList = false;

		//Verifica se o "vehicle2" está na lista de vizinhos e atualiza para nós com 1 salto, caso esteja
		for(uint32_t i = 0; i<m_neighborsList.size(); i++){
			//Esse if funciona, mas não precisava. Invés de "m_neighborList[i].size()-1", poderia usar m_neighborList[i][2])
			//if((*closeNeighbor).GetId() == m_neighborsList[i][2]){
			if(vehicle2.GetId() == m_neighborsList[i][2]){
				//vector<uint16_t> m_pathList;
				//m_pathList.push_back((*closeNeighbor).GetId());
				//m_pathList.push_back(1);
				//m_pathList.push_back((*closeNeighbor).GetId());

				//m_neighborsList.push_back(m_pathList);
				m_neighborsList[i][0] = vehicle2.GetId();
				m_neighborsList[i][1] = 1;
				isOnNeighborList = true;
				break;
			}
		}

		//Verifica se o "vehicle2" NÃO está na lista de vizinhos e acrescenta o "vehicle2" com 1 salto
		if(!isOnNeighborList){
			vector<uint16_t> m_pathList;
			m_pathList.push_back(vehicle2.GetId());
			m_pathList.push_back(1);
			m_pathList.push_back(vehicle2.GetId());

			m_neighborsList.push_back(m_pathList);

			//std::cout << m_neighborsList[0][1] << std::endl;
		}
		//}

		vector<uint16_t> onNeighborList;
		for(uint32_t i = 0; i<vehicle2.GetNeighborsList().size(); i++){
			for (uint32_t j = 0 ; j< m_neighborsList.size() ; j++) {
				//Se o atual ID já estiver na lista de vizinho do nó atual, pula para o próximo
				if (vehicle2.GetId() == m_neighborsList[j][2]){
					continue;
				}


				if (m_neighborsList[j][2] == vehicle2.GetNeighborsList()[i][2]){
					onNeighborList.push_back(vehicle2.GetNeighborsList()[i][2]);
					if( ((vehicle2.GetNeighborsList()[i][1] + 1) < m_neighborsList[j][1]) && ((vehicle2.GetNeighborsList()[i][1] + 1) < 4 )){

						m_neighborsList[j][0] = vehicle2.GetId();
						m_neighborsList[j][1] = vehicle2.GetNeighborsList()[i][1] + 1;
						m_neighborsList[j][2] = vehicle2.GetNeighborsList()[i][2];

						*for (uint32_t k = 0; k < vehicle2.GetNeighborsList()[i].size(); k++){
							m_neighborsList[j].push_back(vehicle2.GetNeighborsList()[i][k]);
						}*
					}
					break;
				}

			}
		}

		if(onNeighborList.size()>0){

			for(uint32_t j = 0; j < vehicle2.GetNeighborsList().size(); j++){

				if (vehicle2.GetNeighborsList()[j][2] == m_node->GetId()){
					//Se o atual ID já estiver na lista de vizinho do nó atual, pula para o próximo
					continue;
				}

				bool isOnNeighborList = false;
				for(uint32_t k = 0; k < onNeighborList.size(); k++){
					if(vehicle2.GetNeighborsList()[j][2] == onNeighborList[k]){
						isOnNeighborList = true;
						break;
					}
				}

				if(!isOnNeighborList){
					for(uint32_t k = 0; k < m_neighborsList.size(); k++){
						if (vehicle2.GetNeighborsList()[j][2] == m_neighborsList[k][2]){
							//Se o atual ID já estiver na lista de vizinho do nó atual, pula para o próximo
							isOnNeighborList = true;
							break;
						}
					}
				}

				if(!isOnNeighborList){
					if((vehicle2.GetNeighborsList()[j][1] + 1) < 4){

						vector<uint16_t> m_pathList;
						m_pathList.push_back(vehicle2.GetId());
						m_pathList.push_back(vehicle2.GetNeighborsList()[j][1] + 1);
						m_pathList.push_back(vehicle2.GetNeighborsList()[j][2]);
						*for (uint32_t k = 0; k < vehicle2.GetNeighborsList()[j][1]; k++){
							m_pathList.push_back(vehicle2.GetNeighborsList()[j][k]);
						///}*
						//m_pathList.insert(m_pathList.end(), vehicle2.GetNeighborsList()[j].begin(), vehicle2.GetNeighborsList()[j].end());
						//m_neighborsList[i].insert(m_neighborsList[i].end(), vehicle2.GetNeighborsList()[i].begin(), vehicle2.GetNeighborsList()[i].end());
						m_neighborsList.push_back(m_pathList);
					}
				}
			}
		}



		for(uint32_t i = 0; i<m_neighborsList.size(); i++){
			bool isOnNeighborList = false;
			if(m_neighborsList[i][0] == vehicle2.GetId() && (m_neighborsList[i][1] > 1) ){
				for (uint32_t j = 0 ; j< vehicle2.GetNeighborsList().size() ; j++) {
					if(m_neighborsList[i][2] == vehicle2.GetNeighborsList()[j][2]){
						isOnNeighborList = true;
						break;
					}
				}
				if(!isOnNeighborList){
					m_neighborsList.erase(m_neighborsList.begin() + i);
				}
			}
		}*/
	} else {
		cout << "else" << endl;
	}

	float closenessScore = 0;
	//Calcula closenessScore do nó atual
	for(uint32_t i = 0; i<m_neighborsList.size(); i++){
		closenessScore = closenessScore + m_neighborsList[i][1];
	}

	closenessScore = m_neighborsList.size()/closenessScore;

	m_score = closeNeighbors.size();
	m_closenessScore = closenessScore;
	m_atividadeSocial = novosVizinhos.size();


	//NS_LOG_INFO("NODE: " << m_node->GetId() << " TEM " << m_score << " VIZINHOS E " << m_atividadeSocial << " SÃO NOVOS VIZINHOS.");

	time(&end);

	//double tempo = end - ini;

	//std::cout << "A função HandleBeacon foi executada em " << tempo << " segundos.\n";
	//NS_LOG_INFO("NODE: " << m_node->GetId() << " TEM " << m_score << " VIZINHOS E " << m_eigenScore << " SÃO GATEWAYS.");
}

	// TOPSIS Functions
	vector<vector<double>> normalize(vector<vector<double>> &matrix) {
		size_t rows = matrix.size();
		size_t cols = matrix[0].size();
		
		vector<vector<double>> normalizedMatrix(rows, vector<double> (cols));

		for (size_t j = 0; j < cols; j++) {
			// Euclidean norm
			double columnNorm = 0;
			for (const auto &row : matrix) {
				columnNorm += row[j] * row[j];
			}
			columnNorm = sqrt(columnNorm);

			for (size_t i = 0; i < rows; i++) {
				normalizedMatrix[i][j] = matrix[i][j] / columnNorm;
			}
		}

		return normalizedMatrix;
	}

	vector<vector<double>> multiplyWeights(vector<vector<double>> &matrix, vector<double> &weights) {
		size_t rows = matrix.size();
		size_t cols = matrix[0].size();
		
		vector<vector<double>> weightedMatrix(rows, vector<double> (cols));

		for (size_t i = 0; i < rows; i++) {
			for (size_t j = 0; j < cols; j++) {
				weightedMatrix[i][j] = matrix[i][j] * weights[j];
			}
		}

		return weightedMatrix;
	}

	vector<double> idealSolution(vector<vector<double>> &matrix, vector<double> &weights) {
		size_t cols = matrix[0].size();

		vector<double> ideal(cols, numeric_limits<double>::min());

		for (const auto &row : matrix) {
			for (size_t j = 0; j < cols; j++) {
				ideal[j] = max(ideal[j], row[j]);
			}
		}

		return ideal;
	}

	vector<double> negativeIdealSolution(vector<vector<double>> &matrix, vector<double> &weights) {
		size_t cols = matrix[0].size();

		vector<double> negativeIdeal(cols, numeric_limits<double>::max());

		for (const auto &row : matrix) {
			for (size_t j = 0; j < cols; j++) {
				negativeIdeal[j] = min(negativeIdeal[j], row[j]);
			}
		}

		return negativeIdeal;
	}

	// Euclidean distance
	vector<double> euclideanDistance(vector<vector<double>> &matrix, vector<double> &target) {
		size_t rows = matrix.size();
		vector<double> distances(rows, 0);

		for (size_t i = 0; i < rows; i++) {
			for (size_t j = 0; j < target.size(); j++) {
				distances[i] += (matrix[i][j] - target[j] * (matrix[i][j] - target[j]));
			}
			distances[i] = sqrt(distances[i]);
		}

		return distances;
	}

	// Closeness to ideal
	vector<double> closenessCoefficients(vector<double> &euclideanDistanceIdeal, vector<double> &negativeEuclidianDistanceIdeal) {
		size_t size = euclideanDistanceIdeal.size();
		vector<double> coefficients(size);

		for (size_t i = 0; i < size; i++) {
			coefficients[i] = negativeEuclidianDistanceIdeal[i] / (euclideanDistanceIdeal[i] + negativeEuclidianDistanceIdeal[i]);
		}

		return coefficients;
	}

	// Best alternative
	int bestAlternative(vector<double> coefficients) {
		return distance(coefficients.begin(), max_element(coefficients.begin(), coefficients.end()));
	}

	void printMatrix(vector<vector<double>> &matrix) {
		size_t rows = matrix.size();
		size_t cols = matrix[0].size();

		for (size_t i = 0; i < rows; i++) {
			for (size_t j = 0; j < cols; j++) {
				cout << matrix[i][j] << " ";
			}
			cout << endl;
		}
	}

// Create directory for log files
bool createDirectory(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        // O diretório não existe; crie-o
        if (mkdir(path.c_str(), 0777) != 0) {
            return false;
        }
    }
    return true;
}

std::map<uint16_t, int> countTimesRelay; // Number of times that a node has previously been retransmitted
uint16_t sameVehicleID = 0xFFFF;


// Veículos leves (Potências x Faixas etárias)
//                                  P | (18-29 30-39 40-49 50-59 60-69 70-79 80+) 
float faixa_matrix_leves[20][8] = {{0  , 18  , 30  , 40  , 50  , 60  , 70  , 80},
								   {60 , 1   , 1   , 1   , 1   , 0.75, 0.75, 0.25},
								   {70 , 1   , 1   , 1   , 1   , 0.75, 0.75, 0.25},
								   {80 , 1   , 1   , 1   , 1   , 0.75, 0.75, 0.25},
								   {90 , 1   , 1   , 1   , 1   , 0.75, 0.75, 0.25},
								   {100, 1   , 1   , 1   , 1   , 0.75, 0.75, 0.25},
								   {110, 0.75, 1   , 1   , 1   , 0.75, 0.75, 0.25},
								   {120, 0.75, 1   , 1   , 1   , 0.75, 0.75, 0.25},
								   {130, 0.75, 1   , 1   , 1   , 0.75, 0.5 , 0.25},
								   {140, 0.75, 1   , 1   , 1   , 0.75, 0.5 , 0.25},
								   {150, 0.75, 1   , 1   , 1   , 0.75, 0.5 , 0.25},
								   {160, 0.5 , 1   , 1   , 1   , 0.5 , 0.5 , 0   },
								   {170, 0.5 , 1   , 1   , 1   , 0.5 , 0.5 , 0   },
	 							   {180, 0.5 , 0.75, 1   , 1   , 0.5 , 0.5 , 0   },
		 						   {190, 0.25, 0.75, 1   , 1   , 0.5 , 0.5 , 0   },
			 					   {200, 0.25, 0.75, 1   , 0.75, 0.5 , 0.25, 0   },
				 				   {225, 0   , 0.75, 1   , 0.75, 0.25, 0   , 0   },
					 			   {250, 0   , 0.5 , 0.75, 0.75, 0.25, 0   , 0   },
						 		   {275, 0   , 0.5 , 0.75, 0.75, 0.25, 0   , 0   },
							 	   {300, 0   , 0.5 , 0.75, 0.75, 0.25, 0   , 0   },
};

// Veículos pesados (Potências x Faixas etárias)
//                                 (18-29   30-49  50-59  60-64  65-66  67-69  70+)
float faixa_matrix_pesados[2][7] = {{18    , 30   , 50   , 60   , 65   , 67   , 70},
                                    {0.75  , 1    , 1    , 0.75 , 0.5  , 0.25 , 0}};

/*
double CalculateFuelEfficiencyScore(double fuelEfficiency, double minEfficiency, double maxEfficiency) {
    // Defina os limites de eficiência de combustível desejados
    const double minEfficiency = minEfficiency;  // Eficiência mínima desejada (por litro, por exemplo)
    const double maxEfficiency = maxEfficiency;  // Eficiência máxima desejada

    // Normaliza a eficiência de combustível para um intervalo entre 0 e 1
    double normalizedEfficiency = std::max(0.0, std::min((fuelEfficiency - minEfficiency) / (maxEfficiency - minEfficiency), 1.0));

    // Quanto mais eficiente o veículo, maior a pontuação
    double efficiencyScore = normalizedEfficiency;

    return efficiencyScore;
}

double CalculateManeuverabilityScore(double turnRadius, double minTurnRadius, double maxTurnRadius) {
    // Defina o raio mínimo e máximo desejado para uma boa manobrabilidade
    const double minTurnRadius = minTurnRadius;  // Raio mínimo desejado
    const double maxTurnRadius = maxTurnRadius; // Raio máximo desejado

    // Normaliza o raio de curvatura para um intervalo entre 0 e 1
    double normalizedTurnRadius = std::max(0.0, std::min((maxTurnRadius - turnRadius) / (maxTurnRadius - minTurnRadius), 1.0));

    // Quanto menor o raio de curvatura, melhor a manobrabilidade e maior a pontuação
    double maneuverabilityScore = normalizedTurnRadius;

    return maneuverabilityScore;
}

double CalculateSafetyScore(double collisionRate) {
    // Defina a taxa máxima de colisão desejada
    const double maxCollisionRate = 0.02; // Taxa máxima de colisão desejada por unidade de tempo

    // Normaliza a taxa de colisão para um intervalo entre 0 e 1
    double normalizedCollisionRate = std::min(collisionRate / maxCollisionRate, 1.0);

    // Quanto menor a taxa de colisão, melhor a segurança e maior a pontuação
    double safetyScore = 1.0 - normalizedCollisionRate;

    return safetyScore;
}

double CalculateVehicleConditionScore(double mileage, int maintenanceIssues) {
    // Defina a quilometragem máxima desejada para uma boa condição do veículo
    const double maxMileage = 100000.0;  // Quilometragem máxima desejada

    // Normaliza a quilometragem para um intervalo entre 0 e 1
    double normalizedMileage = std::max(0.0, std::min(mileage / maxMileage, 1.0));

    // Defina o número máximo de problemas de manutenção aceitáveis
    const int maxMaintenanceIssues = 2;  // Número máximo de problemas de manutenção aceitáveis

    // Normaliza o número de problemas de manutenção para um intervalo entre 0 e 1
    double normalizedMaintenanceIssues = 1.0 - std::min(static_cast<double>(maintenanceIssues) / maxMaintenanceIssues, 1.0);

    // A condição geral é uma combinação da baixa quilometragem e do baixo número de problemas de manutenção
    double vehicleConditionScore = 0.5 * normalizedMileage + 0.5 * normalizedMaintenanceIssues;

    return vehicleConditionScore;
}

double CalculateTrafficRulesAdherenceScore(bool followsTrafficRules) {
    // Se o veículo segue as regras de trânsito, atribui a pontuação máxima, caso contrário, pontuação mínima
    double adherenceScore = followsTrafficRules ? 1.0 : 0.0;

    return adherenceScore;
}

double CalculateTireWearScore(double tireWear) {
    // Defina o desgaste máximo desejado para uma boa pontuação
    const double maxTireWear = 5.0;  // Desgaste máximo desejado em milímetros

    // Normaliza o desgaste dos pneus para um intervalo entre 0 e 1
    double normalizedTireWear = std::max(0.0, std::min(tireWear / maxTireWear, 1.0));

    // Quanto menor o desgaste dos pneus, melhor, e maior a pontuação
    double tireWearScore = 1.0 - normalizedTireWear;

    return tireWearScore;
}
*/

// Function to find a node by ID
Ptr<Node> FindNodeById(uint16_t nodeId) {
    auto it = nodeMap.find(nodeId);
    if (it != nodeMap.end())
        return it->second;
	else
        return nullptr; // Node with the given ID not found
}

// Function to calculate Euclidean distance between two vectors
double VectorDistance(const Vector &v1, const Vector &v2) {
    return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}

// Função para normalizar uma matriz (dividir cada elemento pela soma da coluna)
void normalizeMatrix(vector<vector<double>>& matrix) {
    int numRows = matrix.size();
    int numCols = matrix[0].size();

    for (int j = 0; j < numCols; ++j) {
        double colSum = 0.0;
        for (int i = 0; i < numRows; ++i) {
            colSum += matrix[i][j];
        }

        for (int i = 0; i < numRows; ++i) {
            matrix[i][j] /= colSum;
        }
    }
}

// Função para calcular a média de cada linha
vector<double> calculateRowMeans(const vector<vector<double>>& matrix) {
    int numRows = matrix.size();
    int numCols = matrix[0].size();

    vector<double> means(numRows, 0.0);

    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            means[i] += matrix[i][j];
        }
        means[i] /= numCols;
    }

    return means;
}

// Função para calcular os pesos finais usando o método AHP
vector<double> calculateAHPWeights(const vector<vector<double>>& comparisonMatrix) {
    int numCriteria = comparisonMatrix.size();

    // Normalizar a matriz de comparação
    vector<vector<double>> normalizedMatrix = comparisonMatrix;
    normalizeMatrix(normalizedMatrix);

    // Calcular a média de cada linha
    vector<double> rowMeans = calculateRowMeans(normalizedMatrix);

    // Calcular a média das médias
    double meanOfMeans = 0.0;
    for (int i = 0; i < numCriteria; ++i) {
        meanOfMeans += rowMeans[i];
    }
    meanOfMeans /= numCriteria;

    // Calcular os pesos finais
    vector<double> weights(numCriteria, 0.0);
    for (int i = 0; i < numCriteria; ++i) {
        weights[i] = rowMeans[i] / meanOfMeans;
    }

    return weights;
}

uint16_t SOCIAL::RelayElection() {
	time_t ini, end;

	time(&ini);

	vector<VehicleSOCIAL>::iterator closeNeighbor;

	float score = 0 ;

	int n = closeNeighbors.size();

	vector<vector<double>> evaluationMatrix;
	vector<int> ids;
	vector<Vector>::iterator basestationPos;
	double_t basestationDistance;
	double_t closestBasestationDistance;
	Vector closestBasestationPosition;
	Vector closestBasestationDirection;
	double closeNeighborAngle;

	double closestVelocityDifference;
	int peakTime;
	int timeTraveled;
	double distanceTraveled;
	int timeLicensed;
	int penaltyHistory;
	int vehicleAge;
	double ageVsPotency;

	/*
	m_mobilityModel = m_node->GetObject<MobilityModel>();
	double mySpeed = m_mobilityModel->GetVelocity().GetLength();

	double totalNeighborSpeed = 0;
    int numNeighbors = 0;

    for (closeNeighbor = closeNeighbors.begin(); closeNeighbor != closeNeighbors.end(); closeNeighbor++) {
		int neighborId = closeNeighbor->GetId();

		Ptr<Node> neighborNode = FindNodeById(neighborId);

		if (neighborNode != nullptr) {
			m_mobilityModel = neighborNode->GetObject<MobilityModel>();
            totalNeighborSpeed += m_mobilityModel->GetVelocity().GetLength();
            numNeighbors++;
        }
    }*/

	m_mobilityModel = m_node->GetObject<MobilityModel>();
	Vector myVelocity = m_mobilityModel->GetVelocity();

	Vector totalNeighborVelocity(0.0, 0.0, 0.0);
	// Vector totalNeighborAcceleration(0.0, 0.0, 0.0);
    int numNeighbors = 0;


	for (closeNeighbor = closeNeighbors.begin(); closeNeighbor != closeNeighbors.end(); closeNeighbor++) {
        uint16_t neighborId = closeNeighbor->GetId();

        Ptr<Node> neighborNode = FindNodeById(neighborId);

        if (neighborNode != nullptr) {
			m_mobilityModel = neighborNode->GetObject<MobilityModel>();

            totalNeighborVelocity.x += m_mobilityModel->GetVelocity().x;
			totalNeighborVelocity.y += m_mobilityModel->GetVelocity().y;
			totalNeighborVelocity.z += m_mobilityModel->GetVelocity().z;

			/*
			// Aproximação da aceleração usando diferença finita
			double deltaT = 1.0; // Suponha um intervalo de tempo fixo (pode ser ajustado conforme necessário)
			totalNeighborAcceleration.x += (m_mobilityModel->GetVelocity().x - myVelocity.x) / deltaT;
			totalNeighborAcceleration.y += (m_mobilityModel->GetVelocity().y - myVelocity.y) / deltaT;
			totalNeighborAcceleration.z += (m_mobilityModel->GetVelocity().z - myVelocity.z) / deltaT;
			*/

            numNeighbors++;
        }
    }

	Vector averageNeighborVelocity = Vector(
		totalNeighborVelocity.x / numNeighbors,
		totalNeighborVelocity.y / numNeighbors,
		totalNeighborVelocity.z / numNeighbors
	);

	/*
	Vector averageNeighborAcceleration = Vector(
		totalNeighborAcceleration.x / numNeighbors,
		totalNeighborAcceleration.y / numNeighbors,
		totalNeighborAcceleration.z / numNeighbors
	);
	*/

	closestVelocityDifference = VectorDistance(averageNeighborVelocity, myVelocity);
    uint16_t closestVelocityId = m_node->GetId();

	// double closestAccelerationDifference = 0.0;


	for (closeNeighbor = closeNeighbors.begin(); closeNeighbor != closeNeighbors.end(); closeNeighbor++) {
		//cout << closeNeighbor->GetId() << ": " << closeNeighbor->GetDirection().x << " " << closeNeighbor->GetDirection().y << " " << closeNeighbor->GetDirection().z << " " << endl;
		m_mobilityModel = m_node->GetObject<MobilityModel>();
		//double_t newSeparation = CalculateDistance(m_mobilityModel->GetPosition(), closeNeighbor->GetPosition());
		closestBasestationDistance = MinuetConfig::FLOOR_SIZE_X + MinuetConfig::FLOOR_SIZE_Y;

		for (basestationPos = MinuetConfig::GetBasestationPosIterator(); basestationPos != MinuetConfig::GetBasestationPosEnd(); basestationPos++) {
			basestationDistance = CalculateDistance(*basestationPos, closeNeighbor->GetPosition());
			closestBasestationDistance = min(closestBasestationDistance, basestationDistance);
			closestBasestationPosition = *basestationPos;
		}

		// Direction
		if (closeNeighbor->GetDirection().x == 0.0 && closeNeighbor->GetDirection().y == 0.0) {
			// Supondo que um veículo parado é equivalente a um veículo a um ângulo de 45 graus
			closeNeighborAngle = 0.785398;
		} else {
			closestBasestationDirection.x = closestBasestationPosition.x - closeNeighbor->GetDirection().x;
			closestBasestationDirection.y = closestBasestationPosition.y - closeNeighbor->GetDirection().y;

			double x1 = closestBasestationDirection.x;
			double x2 = closeNeighbor->GetDirection().x;
			double y1 = closestBasestationDirection.y;
			double y2 = closeNeighbor->GetDirection().y;

			closeNeighborAngle = acos((x1 * x2 + y1 * y2) / (sqrt(pow(x1, 2) + pow(y1, 2)) * sqrt(pow(x2, 2) + pow(y2, 2))));
		}

		/*
		Ptr<Node> neighborNode = FindNodeById(closeNeighbor->GetId());

		if (neighborNode != nullptr) {
            Vector neighborVelocity = neighborNode->GetVelocity();
            double velocityDifference = VectorDistance(averageNeighborVelocity, neighborVelocity);

            if (velocityDifference < closestVelocityDifference) {
                closestVelocityDifference = velocityDifference;
                closestVelocityId = neighborID;
            }
        }*/

		uint16_t neighborID = closeNeighbor->GetId();

        Ptr<Node> neighborNode = FindNodeById(neighborID);

        if (neighborNode != nullptr) {
			m_mobilityModel = neighborNode->GetObject<MobilityModel>();
            Vector neighborVelocity = m_mobilityModel->GetVelocity();

			double km_neighborVelocityx = neighborVelocity.x * 3.6;
			double km_neighborVelocityy = neighborVelocity.y * 3.6;
			double km_neighborVelocityz = neighborVelocity.z * 3.6;
			double km_averageNeighborVelocityx = averageNeighborVelocity.x * 3.6;
			double km_averageNeighborVelocityy = averageNeighborVelocity.y * 3.6;
			double km_averageNeighborVelocityz = averageNeighborVelocity.z * 3.6;

			double magnitudeAverageNeighborVelocity = sqrt(pow(km_averageNeighborVelocityx, 2) + pow(km_averageNeighborVelocityy, 2) + pow(km_averageNeighborVelocityz, 2));
			double magnitudeMyVelocity = sqrt(pow(km_neighborVelocityx, 2) + pow(km_neighborVelocityy, 2) + pow(km_neighborVelocityz, 2));

			if (magnitudeMyVelocity == magnitudeAverageNeighborVelocity || magnitudeMyVelocity < magnitudeAverageNeighborVelocity * 2)
				closestVelocityDifference = 1.0;
			else if (magnitudeMyVelocity >= magnitudeAverageNeighborVelocity / 2 && magnitudeMyVelocity < magnitudeAverageNeighborVelocity)
				closestVelocityDifference = 0.75;
			else
				closestVelocityDifference = 0.5;

			cout << "MEU: " << magnitudeMyVelocity << " CLUSTER: " << magnitudeAverageNeighborVelocity << endl;
        }

		/*
		// Aproximação da aceleração usando diferença finita
        double deltaT = 1.0; // Suponha um intervalo de tempo fixo (pode ser ajustado conforme necessário)
        double accelerationX = (m_mobilityModel->GetVelocity().x - myVelocity.x) / deltaT;
        double accelerationY = (m_mobilityModel->GetVelocity().y - myVelocity.y) / deltaT;
        double accelerationZ = (m_mobilityModel->GetVelocity().z - myVelocity.z) / deltaT;

        // Calcula a diferença de aceleração mais próxima
        double accelerationDifferenceX = abs(accelerationX - averageNeighborAcceleration.x);
        double accelerationDifferenceY = abs(accelerationY - averageNeighborAcceleration.y);
        double accelerationDifferenceZ = abs(accelerationZ - averageNeighborAcceleration.z);

        double closestAccelerationDifference = sqrt(
            pow(accelerationDifferenceX, 2) +
            pow(accelerationDifferenceY, 2) +
            pow(accelerationDifferenceZ, 2)
        ); 
		*/

		/*
		std::ifstream file("/root/ns3/ns-allinone-3.29/ns-3.29/src/minuet/utils/trace/random.txt");
		string line;
		map<string, vector<int>> data;

		// Ler o arquivo e armazenar os valores em um mapa
		while (getline(file, line)) {
			istringstream iss(line);
			string key;
			int value;
			iss >> key;
			while (iss >> value)
				data[key].push_back(value);
		}

		int pv = data["pv"][closeNeighbor->GetId()];
		int im = data["im"][closeNeighbor->GetId()];
		int lp = data["lp"][closeNeighbor->GetId()];

		if (!lp) {
			for (int i = 0; i < 20; i++) {
				if (pv == faixa_matrix_leves[i][0]) {
					for (int j = 1; j < 8; j++) {
						if (im >= faixa_matrix_leves[i][j] && im < faixa_matrix_leves[i][j + 1]) {
							ageVsPotency = faixa_matrix_leves[i][j];
						}
					}
				}
			}
		}
		else {
			for (int i = 0; i < 8; i++) {
				if (im >= faixa_matrix_pesados[i] && im < faixa_matrix_pesados[i + 1]) {
					ageVsPotency = faixa_matrix_pesados[i];
				}
			}
		}
		*/

		ids.push_back(
			closeNeighbor->GetId()
		);

		evaluationMatrix.push_back({
			(double)closeNeighbor->GetScore(), // Number of neighbors
			(closestBasestationDistance), // Distance from closest basestation (in cm),
			(closeNeighborAngle), // Direction
			(closestVelocityDifference) // Average velocity
			// (closestAccelerationDifference) // Average acceleration
			// (ageVsPotency) // Idade do motorista x Potência do veículo
		});
	}

	int bestIndex = 0;
	double bestScore = 0.0;
	
	//Simple math
	
	vector<double> minValues = {0, 0, 0, 0};
	vector<double> maxValues = {100, sqrt(pow(MinuetConfig::FLOOR_SIZE_X, 2) + pow(MinuetConfig::FLOOR_SIZE_Y, 2)), 3.14159, 300};

	int numCriteria = 4; // Atualizado para incluir o novo critério
	for (size_t i = 0; i < evaluationMatrix.size(); i++) {
		for (int j = 0; j < numCriteria; j++) {
			if (evaluationMatrix[i][j] < minValues[j])
				minValues[j] = evaluationMatrix[i][j];
			if (evaluationMatrix[i][j] > maxValues[j])
				maxValues[j] = evaluationMatrix[i][j];
		}
	}

	double currentScore;
	double c1, c2, c3, c4; // Atualizado para incluir o novo coeficiente

	for (size_t i = 0; i < evaluationMatrix.size(); i++) {
		c1 = minValues[0] == maxValues[0]
			? 1.0
			: (evaluationMatrix[i][0] - minValues[0]) / (maxValues[0] - minValues[0]);
		c2 = minValues[1] == maxValues[1]
			? 1.0
			: (maxValues[1] - evaluationMatrix[i][1]) / (maxValues[1] - minValues[1]);
		c3 = minValues[2] == maxValues[2]
			? 1.0
			: (maxValues[2] - evaluationMatrix[i][2]) / (maxValues[2] - minValues[2]);
		c4 = minValues[3] == maxValues[3]
			? 1.0
			: (maxValues[3] - evaluationMatrix[i][3]) / (maxValues[3] - minValues[3]);

		vector<double> weights = {0.2, 0.5, 0.3, 0.9};
		double sumWeights = accumulate(weights.begin(), weights.end(), 0.0);
		for (size_t i = 0; i < weights.size(); i++)
 			weights[i] /= sumWeights;

		currentScore = c1 * weights[0] + c2 * weights[1] + c3 * weights[2] + c4 * weights[3];

		if (m_node->GetId() == 2) {
			printf("normalização %i:  %f %f %f (%f) \n", ids[i], c1, c2, c3, currentScore);
		}

		if (currentScore > bestScore) {
			bestIndex = i;
			bestScore = currentScore;
		}
	}

	PrintInLog("NODE " + std::to_string(ids[bestIndex]) + " IS THE NEXT RELAY WITH SCORE: " + std::to_string(bestScore));
	PrintInLog("Velocity difference with closest neighbor: " + std::to_string(closestVelocityDifference));
	// PrintInLog("Acceleration difference with closest neighbor: " + std::to_string(closestAccelerationDifference));

	// AHP Functions
	vector<double> weights = calculateAHPWeights(evaluationMatrix);

	cout << "Final Weights: ";
    for (double weight : weights)
        cout << weight << " ";
    cout << endl;

	/*
	// TOPSIS Functions
	// Criteria Weights
    vector<double> weights = {0.2, 0.4, 0.3, 0.1};

    // Normalization
    vector<vector<double>> normalizedMatrix = normalize(evaluationMatrix);

    // Multiply Weights
    vector<vector<double>> weightedMatrix = multiplyWeights(normalizedMatrix, weights);
    
    // ideal and anti-ideal solution
    vector<double> ideal = idealSolution(weightedMatrix, weights);
    vector<double> antiIdeal = negativeIdealSolution(weightedMatrix, weights);

    // Euclidean distance
    vector<double> euclideanDistanceIdeal = euclideanDistance(weightedMatrix, ideal);
    vector<double> euclideanDistanceAntiIdeal = euclideanDistance(weightedMatrix, antiIdeal);

    // Coefficients
    vector<double> coeffs = closenessCoefficients(euclideanDistanceIdeal, euclideanDistanceAntiIdeal);

    // Best Alternative
    bestIndex = bestAlternative(coeffs);

    cout << "O melhor é " << bestIndex << endl;
	*/

	// Start of collection of times that a vehicle became relay
	uint16_t vehicleID = ids[bestIndex]; // Get the ID of the best index

	// Use the vehicle ID as the key in the map
	auto it = countTimesRelay.find(vehicleID);

	if (it != countTimesRelay.end() && vehicleID != sameVehicleID) {
		it->second++; // Increment the count associated with the ID
	}
	else if (countTimesRelay[vehicleID] < 1) {
		countTimesRelay[vehicleID] = 1; // Initialize the count for this ID to 1
	}

	sameVehicleID = vehicleID;

	// Creation of log files 
	PrintInLogFCV("Relay node: " + std::to_string(ids[bestIndex]));
	PrintInLogFCV("Score: " + std::to_string(bestScore));

	for (const auto& entry : countTimesRelay)
		PrintInLogFCV("Node " + std::to_string(entry.first) + " was relay: " + std::to_string(entry.second));
	PrintInLogFCV("");

	return ids[bestIndex];
	time(&end);
}

/*
uint16_t SOCIAL::RelayElection() {
	time_t ini, end;

	time(&ini);

	if(!m_isPerfilSocial)
		return -1;

	vector<VehicleSOCIAL>::iterator closeNeighbor;

	float score = 0 ;
	float degreeScore = 0;
	float closenessScore = 0;
	float eigenScore = 0;
	float betweenScore = 0;
	float str = 0;
	float atividadeSocial = 0;
	float amizade = 0;

	float highestScore = 0;
	float highestDegreeScore;
	float highestClosenessScore;
	float highestEigenScore;
	float highestBetweenScore;
	float highestAtividadeSocial;
	float highestAmizade;
	float highestSTR;
	uint16_t highestScoreId = 0;

	highestBetweenScore = betweenScore;
	highestClosenessScore = closenessScore;
	highestDegreeScore = degreeScore;
	highestEigenScore = eigenScore;
	highestSTR = str;
	highestAtividadeSocial = atividadeSocial;
	highestAmizade = amizade;


	if (highestBetweenScore && highestClosenessScore && highestDegreeScore && highestEigenScore){
		;Empty
	}

	for (closeNeighbor = closeNeighbors.begin() ; closeNeighbor != closeNeighbors.end() ; closeNeighbor++) {
		str = 0;
		degreeScore = (*closeNeighbor).GetScore();
		closenessScore = (*closeNeighbor).GetClosenessScore();
		eigenScore = (*closeNeighbor).GetEigenScore();
		betweenScore = 0;

		for (uint32_t i = 0; i<m_neighborsList.size(); i++) {
			if((*closeNeighbor).GetId() == m_neighborsList[i][0]){
				betweenScore++;
			}
		}

		//betweenScore = (2*betweenScore) / ( pow(m_neighborsList.size(), 2) - (3*m_neighborsList.size()) + 2 );
		betweenScore = betweenScore / m_neighborsList.size();

		//Se o STR não for calculado, isto é, a lista de vizinhos não existir, BC e CC são NaN, fiz esse if para não dar esse problema.
		if(!m_neighborsList.size()){
			betweenScore = 0;
			closenessScore = 0;
		}

		if(degreeScore == 0){
			eigenScore = 0;
		} else {
			eigenScore = eigenScore/degreeScore;
		}

		degreeScore = degreeScore/(MinuetConfig::GetTotalNodes()-1);
		if(degreeScore>1)
			degreeScore = 1;


		//NS_LOG_INFO("Nó " << (*closeNeighbor).GetId() << " degree: " << degreeScore << " closeness: " << closenessScore << " betweenness: " << betweenScore << " eigen: " <<  eigenScore);

		if (MinuetConfig::wSTR)
			str = float((wBC * betweenScore) + (wCC * closenessScore) + (wDC * degreeScore) + (wEC * eigenScore));
		else
			str = 0;

		amizade = (float) MinuetConfig::GetAmizade((m_node->GetId()-MinuetConfig::GetMenorId()), ((*closeNeighbor).GetId())-MinuetConfig::GetMenorId())/4;

		atividadeSocial = (float) (*closeNeighbor).GetAtividadeSocial()/(*closeNeighbor).GetScore();

		//NS_LOG_INFO("Nó " << (*closeNeighbor).GetId() << " Str: " << str << " Atividade Social: " << atividadeSocial << " Amizade: " << amizade);

		//wSTR = 0;
		//wAm = 0.5;
		//wAS = 0.5;

		score = ((MinuetConfig::wSTR * str) + (MinuetConfig::wAm * amizade) + (MinuetConfig::wAS * atividadeSocial));


		if(score > highestScore){
			highestBetweenScore = betweenScore;
			highestClosenessScore = closenessScore;
			highestDegreeScore = degreeScore;
			highestEigenScore = eigenScore;
			highestSTR = str;
			highestAtividadeSocial = atividadeSocial;
			highestAmizade = amizade;

			highestScore = score;
			highestScoreId = (*closeNeighbor).GetId();
		}
	}

	if(highestScoreId == 0){
		//NS_LOG_ERROR("CAN'T FIND THE NEXT RELAY NODE!");
		//cerr << "## ERROR: Can't find the next relay node!" << endl;
		//exit(1);
		return -1;
	}
	else{
		//PrintInLog("NODE " + std::to_string(highestScoreId) + " IS THE NEXT RELAY WITH STR " + std::to_string(highestScore) + " DEGREE " + std::to_string(highestDegreeScore) + ", CLOSENESS " + std::to_string(highestClosenessScore) + ", BETWEENNESS: " + std::to_string(highestBetweenScore) + ", EIGENVECTOR: " +  std::to_string(highestEigenScore) + ". WITH QoS WEIGHT " + std::to_string(wBC*3) + " AND QoE WEIGHT " + std::to_string(wEC));
		PrintInLog("NODE " + std::to_string(highestScoreId) + " IS THE NEXT RELAY WITH SCORE: " + std::to_string(highestScore) + " STR: " + std::to_string((MinuetConfig::wSTR * highestSTR)) + " SOCIAL ACTIVITY: " + std::to_string((MinuetConfig::wAS * highestAtividadeSocial)) + " AND FRIENDSHIP: " + std::to_string((MinuetConfig::wAm * highestAmizade)));
		//NS_LOG_INFO("NODE " << std::to_string(highestScoreId) << " IS THE NEXT RELAY WITH SCORE " << std::to_string(highestScore) << " STR " << std::to_string(highestSTR) << ", ATIVIDADE SOCIAL " << std::to_string(highestAtividadeSocial) << ", AMIZADE: " << std::to_string(highestAmizade));
		//NS_LOG_INFO("NODE " << (*closeNeighbor).GetId() << " IS THE NEXT RELAY WITH STR: " << (MinuetConfig::wSTR * str) << " SOCIAL ACTIVITY: " << (MinuetConfig::wAS * atividadeSocial) << " AND FRIENDSHIP: " << (MinuetConfig::wAm * amizade));
		//NS_LOG_INFO("NODE " << highestScoreId << " IS THE NEXT RELAY WITH STR " << str << " DEGREE " << degreeScore << ", CLOSENESS " << closenessScore << ", BETWEENNESS: " << betweenScore << ", EIGENVECTOR: " <<  eigenScore);
		return highestScoreId;

	}

	int n = closeNeighbors.size();

	vector<vector<double>> evaluationMatrix;
	vector<int> ids;
	vector<Vector>::iterator basestationPos;
	double_t basestationDistance;
	double_t closestBasestationDistance;
	Vector closestBasestationPosition;
	Vector closestBasestationDirection;
	double closeNeighborAngle;

	int idadeVeiculo;
	int idadeMotorista;
	int multasVeiculo;
	int mesesMultadoVeiculo;
	int potenciaVeiculo;
	int pesoVeiculo;
	int tempoHabilitadoMotorista;

	for (closeNeighbor = closeNeighbors.begin(); closeNeighbor != closeNeighbors.end(); closeNeighbor++) {
		//cout << closeNeighbor->GetId() << ": " << closeNeighbor->GetDirection().x << " " << closeNeighbor->GetDirection().y << " " << closeNeighbor->GetDirection().z << " " << endl;
		m_mobilityModel = m_node->GetObject<MobilityModel>();
		//double_t newSeparation = CalculateDistance(m_mobilityModel->GetPosition(), closeNeighbor->GetPosition());
		closestBasestationDistance = MinuetConfig::FLOOR_SIZE_X + MinuetConfig::FLOOR_SIZE_Y;

		for (basestationPos = MinuetConfig::GetBasestationPosIterator(); basestationPos != MinuetConfig::GetBasestationPosEnd(); basestationPos++) {
			basestationDistance = CalculateDistance(*basestationPos, closeNeighbor->GetPosition());
			closestBasestationDistance = min(closestBasestationDistance, basestationDistance);
			closestBasestationPosition = *basestationPos;
		}

		// Direction
		if (closeNeighbor->GetDirection().x == 0.0 && closeNeighbor->GetDirection().y == 0.0) {
			// Supondo que um veículo parado é equivalente a um veículo a um ângulo de 45 graus
			closeNeighborAngle = 0.785398;
		} else {
			closestBasestationDirection.x = closestBasestationPosition.x - closeNeighbor->GetDirection().x;
			closestBasestationDirection.y = closestBasestationPosition.y - closeNeighbor->GetDirection().y;

			double x1 = closestBasestationDirection.x;
			double x2 = closeNeighbor->GetDirection().x;
			double y1 = closestBasestationDirection.y;
			double y2 = closeNeighbor->GetDirection().y;

			closeNeighborAngle = acos((x1 * x2 + y1 * y2) / (sqrt(pow(x1, 2) + pow(y1, 2)) * sqrt(pow(x2, 2) + pow(y2, 2))));
		}

		int no_atual = closeNeighbor->GetId();

		// Idade do veículo
		idadeVeiculo = year - criterios[0][no_atual];
		float idadeScore = 0;

		if (idadeVeiculo < 10) idadeScore = 1;
		else if (idadeVeiculo > 10 && idadeVeiculo <= 20) idadeScore = 0.75;
		else if (idadeVeiculo > 20 && idadeVeiculo <= 40) idadeScore = 0.5;
		else idadeScore = 0.25;

		// Multas do veículo
		float multasScore = 0;

		int maiorMulta = 0;
		int quantMultas = 0;
		for (const int& multa : criterios[1]) {
			quantMultas += multa;
			if (multa > maiorMulta)
				maiorMulta = multa;
		}

		multasScore = (std::pow(1.0 - static_cast<double>(criterios[1][no_atual]) / quantMultas, static_cast<double>(criterios[1][no_atual]) / maiorMulta) * (1.0 - static_cast<double>(criterios[2][no_atual]) / 12.0));

		// Idade do motorista e potência do veículo
		int potencia = criterios[3][no_atual];
		int idade = criterios[4][no_atual];

		float idadePotenciaScore = -1;
		int lin, col;

		if (!criterios[5][no_atual]) { // Leve
			for (int i = 0; i < 20; i++) {
				if (potencia == faixa_matrix_leves[i][0]) {
					lin = i;
					break;
				}
			}

			if (idade >= 18 && idade <= 29) col = 1;
			else if (idade > 29 && idade <= 39) col = 2;
			else if (idade > 39 && idade <= 49) col = 3;
			else if (idade > 49 && idade <= 59) col = 4;
			else if (idade > 59 && idade <= 69) col = 5;
			else if (idade > 69 && idade <= 79) col = 6;
			else if (idade > 79) col = 7;
			else idadePotenciaScore = 0;

			if (idadePotenciaScore != 0)
				idadePotenciaScore = faixa_matrix_leves[lin][col];
		}
		else { // Pesado
			if (idade >= 18 && idade <= 29) idadePotenciaScore = 0.75;
			else if (idade > 29 && idade <= 39) idadePotenciaScore = 1;
			else if (idade > 39 && idade <= 49) idadePotenciaScore = 1;
			else if (idade > 49 && idade <= 59) idadePotenciaScore = 0.75;
			else if (idade > 59 && idade <= 69) idadePotenciaScore = 0.5;
			else if (idade > 69 && idade <= 79) idadePotenciaScore = 0.25;
			else idadePotenciaScore = 0;
		}

		// Tempo de habilitação
		tempoHabilitadoMotorista = criterios[6][no_atual];
		float habilitacaoScore = 0;

		if (tempoHabilitadoMotorista > 10) habilitacaoScore = 1;
		else if (tempoHabilitadoMotorista > 5 && tempoHabilitadoMotorista <= 10) habilitacaoScore = 0.75;
		else if (tempoHabilitadoMotorista > 2 && tempoHabilitadoMotorista <= 5) habilitacaoScore = 0.5;
		else if (tempoHabilitadoMotorista > 5 && tempoHabilitadoMotorista <= 2) habilitacaoScore = 0.25;
		else habilitacaoScore = 0;

		ids.push_back(
			closeNeighbor->GetId()
		);

		evaluationMatrix.push_back({
			(double)closeNeighbor->GetScore(), // Number of neighbors
			(closestBasestationDistance), // Distance from closest basestation (in cm),
			(closeNeighborAngle) // Direction
			//(idadeScore), // Vehicle age score
			//(multasScore), // Traffic ticket score
			//(idadePotenciaScore), // Vehicle power x driver's age
			//(habilitacaoScore) // Driver's license age score
			//(vm) // Active neighbor speed
		});
	}

	int bestIndex = 0;
	double bestScore = 0.0;
	
	//Simple math
	
	vector<double> minValues = {0, MinuetConfig::FLOOR_SIZE_X + MinuetConfig::FLOOR_SIZE_Y, 0};
	vector<double> maxValues = {0, 0, 3.14159};

	int numCriteria = 3;
	for (size_t i = 0; i < evaluationMatrix.size(); i++) {
   		for (int j = 0; j < numCriteria; j++) {
			if (evaluationMatrix[i][j] < minValues[j])
				minValues[j] = evaluationMatrix[i][j];
			if (evaluationMatrix[i][j] > maxValues[j])
				maxValues[j] = evaluationMatrix[i][j];
		}
    }

	double currentScore;
	double c1, c2, c3;

	//printf("minValues: %f, %f, %f, maxValues: %f, %f, %f\n", minValues[0], minValues[1], minValues[2], maxValues[0], maxValues[1], maxValues[2]);

	for (size_t i = 0; i < evaluationMatrix.size(); i++) {
		c1 = minValues[0] == maxValues[0]
			? 1.0
			: (evaluationMatrix[i][0] - minValues[0]) / (maxValues[0] - minValues[0]);
		c2 = minValues[1] == maxValues[1]
			? 1.0
			: (maxValues[1] - evaluationMatrix[i][1]) / (maxValues[1] - minValues[1]);
		c3 = minValues[2] == maxValues[2]
			? 1.0
			: (maxValues[2] - evaluationMatrix[i][2]) / (maxValues[2] - minValues[2]);
		c4 = minValues[3] == maxValues[3]
			? 1.0
			: (maxValues[3] - evaluationMatrix[i][3]) / (maxValues[3] - minValues[3]);
		c5 = minValues[4] == maxValues[4]
			? 1.0
			: (evaluationMatrix[i][4] - minValues[4]) / (maxValues[4] - minValues[4]);
		c6 = minValues[5] == maxValues[5]
			? 1.0
			: (maxValues[5] - evaluationMatrix[i][5]) / (maxValues[5] - minValues[5]);
		c7 = minValues[6] == maxValues[6]
			? 1.0
			: (maxValues[6] - evaluationMatrix[i][6]) / (maxValues[6] - minValues[6]);
		c8 = minValues[7] == maxValues[7]
			? 1.0
			: (maxValues[7] - evaluationMatrix[i][7]) / (maxValues[7] - minValues[7]);

		currentScore = c1 * 0.2 + c2 * 0.5 + c3 * 0.3;

		if (m_node->GetId() == 1) {
			printf("normalização %i:  %f %f %f (%f) \n", ids[i], c1, c2, c3, currentScore);
		}

		if (currentScore > bestScore) {
			bestIndex = i;
			bestScore = currentScore;
		}	
	}

	PrintInLog("NODE " + std::to_string(ids[bestIndex]) + " IS THE NEXT RELAY WITH SCORE: " + std::to_string(bestScore));
	

	// TOPSIS Functions
	// Criteria Weights
    vector<double> weights = {0.2, 0.5, 0.3, 0.2, 1, 0.4, 0.5};

    // Normalization
    vector<vector<double>> normalizedMatrix = normalize(evaluationMatrix);

    // Multiply Weights
    vector<vector<double>> weightedMatrix = multiplyWeights(normalizedMatrix, weights);
    
    // ideal and anti-ideal solution
    vector<double> ideal = idealSolution(weightedMatrix, weights);
    vector<double> antiIdeal = negativeIdealSolution(weightedMatrix, weights);

    // Euclidean distance
    vector<double> euclideanDistanceIdeal = euclideanDistance(weightedMatrix, ideal);
    vector<double> euclideanDistanceAntiIdeal = euclideanDistance(weightedMatrix, antiIdeal);

    // Coefficients
    vector<double> coeffs = closenessCoefficients(euclideanDistanceIdeal, euclideanDistanceAntiIdeal);

    // Best Alternative
    bestIndex = bestAlternative(coeffs);

    cout << "O melhor é " << bestIndex << endl;

	// Start of collection of times that a vehicle became relay
	uint16_t vehicleID = ids[bestIndex]; // Get the ID of the best index

	// Use the vehicle ID as the key in the map
	auto it = countTimesRelay.find(vehicleID);

	if (it != countTimesRelay.end() && vehicleID != sameVehicleID) {
		it->second++; // Increment the count associated with the ID
	}
	else if (countTimesRelay[vehicleID] < 1) {
		countTimesRelay[vehicleID] = 1; // Initialize the count for this ID to 1
	}

	sameVehicleID = vehicleID;

	// Creation of log files 
	PrintInLogFCV("Relay node: " + std::to_string(ids[bestIndex]));
	PrintInLogFCV("Score: " + std::to_string(bestScore));

	for (const auto& entry : countTimesRelay)
		PrintInLogFCV("Node " + std::to_string(entry.first) + " was relay: " + std::to_string(entry.second));
		//PrintInLogFCV("Node " + std::to_string(entry.first) + " was relay: " + std::to_string(entry.second) + " | Vehicle age: " + std::to_string(year - criterios[0][entry.first]) + " | Multas: " + std::to_string(criterios[1][entry.first]) + " | Potência: " + std::to_string(criterios[3][entry.first]) + " | Idade motorista: " + std::to_string(criterios[4][entry.first]) + " | Peso: " + std::to_string(criterios[5][entry.first]) + " | Tempo de habilitação: " + std::to_string(criterios[6][entry.first]));
	PrintInLogFCV("");

	return ids[bestIndex];
	time(&end);
}
*/

uint16_t SOCIAL::RelayElectionSTR () {


	time_t ini, end;

	time(&ini);

	//NS_LOG_FUNCTION(this);
	//NS_LOG_INFO("ELECTING Relay!");

	vector<VehicleSOCIAL>::iterator closeNeighbor;

	float degreeScore = 0;
	float closenessScore = 0;
	float eigenScore = 0;
	float betweenScore = 0;
	float str = 0;

	float highestScore = 0;
	float highestDegreeScore;
	float highestClosenessScore;
	float highestEigenScore;
	float highestBetweenScore;
	uint16_t highestScoreId = NaN;

	highestBetweenScore = betweenScore;
	highestClosenessScore = closenessScore;
	highestDegreeScore = degreeScore;
	highestEigenScore = eigenScore;

	if (highestBetweenScore && highestClosenessScore && highestDegreeScore && highestEigenScore){
		;/*Empty*/
	}


	for (closeNeighbor = closeNeighbors.begin() ; closeNeighbor != closeNeighbors.end() ; closeNeighbor++) {
		str = 0;
		degreeScore = (*closeNeighbor).GetScore();
		closenessScore = (*closeNeighbor).GetClosenessScore();
		eigenScore = (*closeNeighbor).GetEigenScore();
		betweenScore = 0;

		for (uint32_t i = 0; i<m_neighborsList.size(); i++) {
			if((*closeNeighbor).GetId() == m_neighborsList[i][0]){
				betweenScore++;
			}
		}

		//betweenScore = (2*betweenScore) / ( pow(m_neighborsList.size(), 2) - (3*m_neighborsList.size()) + 2 );
		betweenScore = betweenScore / m_neighborsList.size();

		if(degreeScore == 0){
			eigenScore = 0;
		} else {
			eigenScore = eigenScore/degreeScore;
		}

		degreeScore = degreeScore/(MinuetConfig::GetTotalNodes()-1);
		if(degreeScore>1)
			degreeScore = 1;


		//NS_LOG_INFO("Nó " << (*closeNeighbor).GetId() << " degree: " << degreeScore << " closeness: " << closenessScore << " betweenness: " << betweenScore << " eigen: " <<  eigenScore);

		str =  (wBC * betweenScore) + (wCC * closenessScore) + (wDC * degreeScore) + (wEC * eigenScore);
		if(str > highestScore){
			highestBetweenScore = betweenScore;
			highestClosenessScore = closenessScore;
			highestDegreeScore = degreeScore;
			highestEigenScore = eigenScore;

			highestScore = str;
			highestScoreId = (*closeNeighbor).GetId();
		}
	}


	if(highestScoreId == 0){
		//NS_LOG_ERROR("CAN'T FIND THE NEXT RELAY NODE!");
		//cerr << "## ERROR: Can't find the next relay node!" << endl;
		//exit(1);
		return -1;
	}
	else{
		PrintInLog("NODE " + std::to_string(highestScoreId) + " IS THE NEXT RELAY WITH STR " + std::to_string(highestScore) + " DEGREE " + std::to_string(highestDegreeScore) + ", CLOSENESS " + std::to_string(highestClosenessScore) + ", BETWEENNESS: " + std::to_string(highestBetweenScore) + ", EIGENVECTOR: " +  std::to_string(highestEigenScore) + ". WITH QoS WEIGHT " + std::to_string(wBC*3) + " AND QoE WEIGHT " + std::to_string(wEC));
		//NS_LOG_INFO("NODE " << std::to_string(highestScoreId) << " IS THE NEXT RELAY WITH STR " << std::to_string(highestScore) << " DEGREE " << std::to_string(highestDegreeScore) << ", CLOSENESS " << std::to_string(highestClosenessScore) << ", BETWEENNESS: " << std::to_string(highestBetweenScore) << ", EIGENVECTOR: " <<  std::to_string(highestEigenScore) << ". WITH QoS WEIGHT " << std::to_string(wBC*3) << " AND QoE WEIGHT " << std::to_string(wEC));
		//NS_LOG_INFO("NODE " << highestScoreId << " IS THE NEXT RELAY WITH STR " << str << " DEGREE " << degreeScore << ", CLOSENESS " << closenessScore << ", BETWEENNESS: " << betweenScore << ", EIGENVECTOR: " <<  eigenScore);
		return highestScoreId;

	}

	time(&end);

	//double tempo = end - ini;

	//std::cout << "A função RelayElection foi executada em " << tempo << " segundos.\n";
}

uint16_t SOCIAL::RelayElectionAtivSocial() {
	vector<VehicleSOCIAL>::iterator closeNeighbor;

	uint16_t highestAtivSocial=0;
	uint16_t highestScoreId = NaN;

	for (closeNeighbor = closeNeighbors.begin() ; closeNeighbor != closeNeighbors.end() ; closeNeighbor++) {
		if((*closeNeighbor).GetAtividadeSocial() > highestAtivSocial){
			highestAtivSocial = (*closeNeighbor).GetAtividadeSocial();
			highestScoreId = (*closeNeighbor).GetId();
		}
	}

	if(highestScoreId == 0){
			//NS_LOG_ERROR("CAN'T FIND THE NEXT RELAY NODE!");
			//cerr << "## ERROR: Can't find the next relay node!" << endl;
			//exit(1);
			return -1;
	}
	else{
		PrintInLog("NODE " + std::to_string(highestScoreId) + " IS THE NEXT RELAY WITH SOCIAL ACTIVITY " + std::to_string(highestAtivSocial));
		//NS_LOG_INFO("NODE " << std::to_string(highestScoreId) << " IS THE NEXT RELAY WITH SOCIAL ACTIVITY " << std::to_string(highestAtivSocial));
		//NS_LOG_INFO("NODE " << highestScoreId << " IS THE NEXT RELAY WITH STR " << str << " DEGREE " << degreeScore << ", CLOSENESS " << closenessScore << ", BETWEENNESS: " << betweenScore << ", EIGENVECTOR: " <<  eigenScore);
		return highestScoreId;

	}

	//return highestScoreId;
}

void SOCIAL::RelayElectionAmizade() {
	/*vector<VehicleSOCIAL>::iterator closeNeighbor;

	APAGAR TUDO ABAIXO DO QUE ESTÁ COMENTADO
	ALTERAR O TIPO DA FUNÇÃO PARA uint16_t AQUI E NO .H

	uint16_t highestAmizade=0;
	uint16_t highestScoreId = NaN;

	for (closeNeighbor = closeNeighbors.begin() ; closeNeighbor != closeNeighbors.end() ; closeNeighbor++) {
		if(MinuetConfig::GetAmizade((m_node->GetId()-MinuetConfig::GetMenorId()), ((*closeNeighbor).GetId())-MinuetConfig::GetMenorId()) > highestAmizade){
			highestAmizade = MinuetConfig::GetAmizade((m_node->GetId()-MinuetConfig::GetMenorId()), ((*closeNeighbor).GetId())-MinuetConfig::GetMenorId());
			highestScoreId = (*closeNeighbor).GetId();
		}
	}

	if(highestScoreId == 0){
			//NS_LOG_ERROR("CAN'T FIND THE NEXT RELAY NODE!");
			//cerr << "## ERROR: Can't find the next relay node!" << endl;
			//exit(1);
			return -1;
	}
	else{
		PrintInLog("NODE " + std::to_string(highestScoreId) + " IS THE NEXT RELAY WITH FRIENDSHIP " + std::to_string(highestAmizade));
		//NS_LOG_INFO("NODE " << std::to_string(highestScoreId) << " IS THE NEXT RELAY WITH FRIENDSHIP " << std::to_string(highestAmizade));
		//NS_LOG_INFO("NODE " << highestScoreId << " IS THE NEXT RELAY WITH STR " << str << " DEGREE " << degreeScore << ", CLOSENESS " << closenessScore << ", BETWEENNESS: " << betweenScore << ", EIGENVECTOR: " <<  eigenScore);
		return highestScoreId;

	}*/

	time_t ini, end;

	time(&ini);

	vector<VehicleSOCIAL>::iterator closeNeighbor;

	float score = 0 ;
	float degreeScore = 0;
	float closenessScore = 0;
	float eigenScore = 0;
	float betweenScore = 0;
	float str = 0;
	float atividadeSocial = 0;
	float amizade = 0;

	float highestScore = 0;
	float highestDegreeScore;
	float highestClosenessScore;
	float highestEigenScore;
	float highestBetweenScore;
	float highestAtividadeSocial;
	float highestAmizade;
	float highestSTR;
	uint16_t highestScoreId = 0;

	highestBetweenScore = betweenScore;
	highestClosenessScore = closenessScore;
	highestDegreeScore = degreeScore;
	highestEigenScore = eigenScore;
	highestSTR = str;
	highestAtividadeSocial = atividadeSocial;
	highestAmizade = amizade;

	if (highestBetweenScore && highestClosenessScore && highestDegreeScore && highestEigenScore){
		;/*Empty*/
	}

	NS_LOG_DEBUG("INFORMAÇÕES DO NÓ: " << m_node->GetId());

	for (closeNeighbor = closeNeighbors.begin() ; closeNeighbor != closeNeighbors.end() ; closeNeighbor++) {
		str = 0;
		degreeScore = (*closeNeighbor).GetScore();
		closenessScore = (*closeNeighbor).GetClosenessScore();
		eigenScore = (*closeNeighbor).GetEigenScore();
		betweenScore = 0;

		for (uint32_t i = 0; i<m_neighborsList.size(); i++) {
			if((*closeNeighbor).GetId() == m_neighborsList[i][0]){
				betweenScore++;
			}
		}

		//betweenScore = (2*betweenScore) / ( pow(m_neighborsList.size(), 2) - (3*m_neighborsList.size()) + 2 );
		betweenScore = betweenScore / m_neighborsList.size();

		//Se o STR não for calculado, isto é, a lista de vizinhos não existir, BC e CC são NaN, fiz esse if para não dar esse problema.
		if(!m_neighborsList.size()){
			betweenScore = 0;
			closenessScore = 0;
		}

		if(degreeScore == 0){
			eigenScore = 0;
		} else {
			eigenScore = eigenScore/degreeScore;
		}

		degreeScore = degreeScore/(MinuetConfig::GetTotalNodes()-1);
		if(degreeScore>1)
			degreeScore = 1;


		NS_LOG_INFO("Nó " << (*closeNeighbor).GetId() << " degree: " << degreeScore << " closeness: " << closenessScore << " betweenness: " << betweenScore << " eigen: " <<  eigenScore);

		if (MinuetConfig::wSTR)
			str = float((wBC * betweenScore) + (wCC * closenessScore) + (wDC * degreeScore) + (wEC * eigenScore));
		else
			str = 0;

		amizade = (float) MinuetConfig::GetAmizade((m_node->GetId()-MinuetConfig::GetMenorId()), ((*closeNeighbor).GetId())-MinuetConfig::GetMenorId())/4;

		atividadeSocial = (float) (*closeNeighbor).GetAtividadeSocial()/(*closeNeighbor).GetScore();

		NS_LOG_DEBUG("Nó " << (*closeNeighbor).GetId() << " Str: " << str << " Atividade Social: " << atividadeSocial << " Amizade: " << amizade);
		PrintInLog("Nó " + std::to_string(m_node->GetId()) + " tem relacionamento com " + std::to_string((*closeNeighbor).GetId()) + " que possui Str: " + std::to_string(str) + " Atividade Social: " + std::to_string(atividadeSocial) + " Amizade: " + std::to_string(amizade));
		//wSTR = 1;
		//wAm = 1;
		//wAS = 1;

		score = ((MinuetConfig::wSTR * str) + (MinuetConfig::wAm * amizade) + (MinuetConfig::wAS * atividadeSocial));

	}


	if(highestScoreId == 0){
		//NS_LOG_ERROR("CAN'T FIND THE NEXT RELAY NODE!");
		//cerr << "## ERROR: Can't find the next relay node!" << endl;
		//exit(1);
		return;
	}
	else{
		//PrintInLog("NODE " + std::to_string(highestScoreId) + " IS THE NEXT RELAY WITH STR " + std::to_string(highestScore) + " DEGREE " + std::to_string(highestDegreeScore) + ", CLOSENESS " + std::to_string(highestClosenessScore) + ", BETWEENNESS: " + std::to_string(highestBetweenScore) + ", EIGENVECTOR: " +  std::to_string(highestEigenScore) + ". WITH QoS WEIGHT " + std::to_string(wBC*3) + " AND QoE WEIGHT " + std::to_string(wEC));
		PrintInLog("NODE " + std::to_string(highestScoreId) + " IS THE NEXT RELAY WITH SCORE: " + std::to_string(highestScore) + " STR: " + std::to_string((MinuetConfig::wSTR * highestSTR)) + " SOCIAL ACTIVITY: " + std::to_string((MinuetConfig::wAS * highestAtividadeSocial)) + " AND FRIENDSHIP: " + std::to_string((MinuetConfig::wAm * highestAmizade)));
		//NS_LOG_INFO("NODE " << std::to_string(highestScoreId) << " IS THE NEXT RELAY WITH SCORE " << std::to_string(highestScore) << " STR " << std::to_string(highestSTR) << ", ATIVIDADE SOCIAL " << std::to_string(highestAtividadeSocial) << ", AMIZADE: " << std::to_string(highestAmizade));
		//NS_LOG_INFO("NODE " << (*closeNeighbor).GetId() << " IS THE NEXT RELAY WITH STR: " << (MinuetConfig::wSTR * str) << " SOCIAL ACTIVITY: " << (MinuetConfig::wAS * atividadeSocial) << " AND FRIENDSHIP: " << (MinuetConfig::wAm * amizade));
		//NS_LOG_INFO("NODE " << highestScoreId << " IS THE NEXT RELAY WITH STR " << str << " DEGREE " << degreeScore << ", CLOSENESS " << closenessScore << ", BETWEENNESS: " << betweenScore << ", EIGENVECTOR: " <<  eigenScore);
		return;

	}

	time(&end);

}

void SOCIAL::Purge (uint32_t vehicleId, vector<VehicleSOCIAL>& vehicleVector) {
	//NS_LOG_FUNCTION(this);

	for(vector<VehicleSOCIAL>::iterator vehicle = vehicleVector.begin() ; vehicle < vehicleVector.end() ; vehicle++) {
		if ((*vehicle).GetId() == vehicleId) {
			vehicle = vehicleVector.erase(vehicle);
			PrintInLog("Purge Node #" + std::to_string(vehicleId) + " from List");
		}
	}
}

void SOCIAL::CleanUp () {
	//NS_LOG_FUNCTION(this);

	time_t ini, end;

	time(&ini);

	for(vector<VehicleSOCIAL>::iterator vehicle = closeNeighbors.begin() ; vehicle < closeNeighbors.end() ; vehicle++) {
		if ((*vehicle).GetTimeExpire() <= Simulator::Now().GetSeconds()) {
			//NS_LOG_INFO("CLEANING UP VEHICLE: " << (*vehicle).GetId() << " FROM VEHICLE "<< m_node->GetId() <<  " CLOSENEIGHBORS");
			for(uint32_t i = 0; i<novosVizinhos.size(); i++){
				if((*vehicle).GetId() == novosVizinhos[i][0]){
					novosVizinhos.erase(novosVizinhos.begin() + i);
					break;
				}
			}
			cout << "cleanup " << vehicle->GetId() << " m_node: "<< m_node->GetId() << endl;
			vehicle = closeNeighbors.erase(vehicle);
		}
	}

	/////////////////////STR////////////////////////////
	//Calcula o eigenvector após o CleanUp
	m_eigenScore = 0;
	for(vector<VehicleSOCIAL>::iterator vehicle = closeNeighbors.begin() ; vehicle < closeNeighbors.end() ; vehicle++) {
		if((*vehicle).IsGateway() == true){
			m_eigenScore++;
			//NS_LOG_INFO("COUNTING EIGENSCORE FROM VEHICLE " << m_node->GetId() << " SCORE IS " << m_eigenScore);
		}
	}

	//Limpa os nós que eram alcançados por nós que não são mais closeNeighbor
	for(uint32_t i = 0; i<m_neighborsList.size(); i++) {
		bool isOnCloseNeighborList = false;
		for(vector<VehicleSOCIAL>::iterator vehicle = closeNeighbors.begin() ; vehicle < closeNeighbors.end() ; vehicle++){
			if( m_neighborsList[i][0] == (*vehicle).GetId() ){
				isOnCloseNeighborList = true;
				break;
			}
		}

		if(isOnCloseNeighborList == false){
			m_neighborsList.erase(m_neighborsList.begin() + i);
			i--;
		}
	}

	float closenessScore = 0;
	//Calcula closenessScore do nó atual após o CleanUp
	for(uint32_t i = 0; i<m_neighborsList.size(); i++){
		closenessScore = closenessScore + m_neighborsList[i][1];
	}

	closenessScore = m_neighborsList.size()/closenessScore;

	m_score = closeNeighbors.size();
	m_closenessScore = closenessScore;
	///////////////END OF STR///////////////////////////


	/////////////////////Atividade Social////////////////////////////
	for(uint32_t i = 0; i<novosVizinhos.size(); i++){
		if((uint16_t) Simulator::Now().GetSeconds() - novosVizinhos[i][1] > 3){
			novosVizinhos.erase(novosVizinhos.begin() + i);
			i--;
		}
	}

	m_atividadeSocial = novosVizinhos.size();
	///////////////END OF Atividade Social///////////////////////////

	m_cleanEvent = Simulator::Schedule (Seconds(SOCIALUtils::CLEAN_UP_TIMER), &SOCIAL::CleanUp, this);

	time(&end);

	//double tempo = end - ini;

	//std::cout << "A função CleanUp foi executada em " << tempo << " segundos.\n";
}

void SOCIAL::VerifyRelay (uint32_t vehicleId) {
	//NS_LOG_FUNCTION(this);

}

void SOCIAL::HasHigherScore (uint32_t vehicleId) {
	//NS_LOG_FUNCTION(this);

}

void SOCIAL::CheckIncreaseByThreshold (VehicleSOCIAL vehicle2, double_t separation, double_t angleDiff) {
	NS_LOG_FUNCTION(this);

	double_t separationThreshold = separation + (separation * SOCIALUtils::THRESHOLD);
	double_t angleDiffThreshold = angleDiff + (angleDiff * SOCIALUtils::THRESHOLD);

	double_t newSeparation = CalculateDistance(m_mobilityModel->GetPosition(), vehicle2.GetPosition());
	double_t newAngleDiff = GetAngleDiff(m_mobilityModel->GetVelocity(), vehicle2.GetDirection());

	vehicle2.SetTimeExpire(Simulator::Now().GetSeconds() + SOCIALUtils::EXPIRY_TIME_INTERVAL_3);

	//NS_LOG_INFO("SETTING NODE "<< vehicle2.GetId() <<" TIMEEXPIRE TO: " << vehicle2.GetTimeExpire());

	if ((newSeparation <= separationThreshold) && (newAngleDiff <= angleDiffThreshold)) {
		//neighbors.push_back(vehicle2);

		if (separation < SOCIALUtils::MAX_SEPARATION_CLOSENEIGHBORS) {
			//NS_LOG_INFO("CLUSTERING NODE "<< vehicle2.GetId() <<" INTO "<< m_node->GetId() <<" CLOSENEIGHBORS WITH SEPARATION: " << separation);
			closeNeighbors.push_back(vehicle2);
		}
	}
}

void SOCIAL::CheckIncreaseByThresholdInAngleAbove45(VehicleSOCIAL vehicle2, double_t separation, double_t angleDiff) {
	//NS_LOG_FUNCTION(this);

	double_t separationThreshold = separation + (separation * SOCIALUtils::THRESHOLD);
	double_t angleDiffThreshold = angleDiff + (angleDiff * SOCIALUtils::THRESHOLD);

	double_t newSeparation = CalculateDistance(m_mobilityModel->GetPosition(), vehicle2.GetPosition());
	double_t newAngleDiff = GetAngleDiff(m_mobilityModel->GetVelocity(), vehicle2.GetDirection());

	if((newSeparation <= separationThreshold) && (newAngleDiff <= angleDiffThreshold)) {

		/*for(vector<VehicleSOCIAL>::iterator neighbor = neighbors.begin() ; neighbor < neighbors.end() ; neighbor++) {
			if((*neighbor).GetId() == vehicle2.GetId())
				(*neighbor).SetTimeExpire((*neighbor).GetTimeExpire() + SOCIALUtils::EXPIRY_TIME_INTERVAL_5);
		}*/

	} else {

		PrintInLog("Taking Node: " + std::to_string(vehicle2.GetId()) + " out of Cluster");
		//Purge(vehicle2.GetId(), neighbors);

	}

	for(auto vehicle : closeNeighbors) {
		if(vehicle2.GetId() == vehicle.GetId())
			Purge(vehicle2.GetId(), closeNeighbors);
	}

	//NS_LOG_INFO("SETTING NODE "<< vehicle2.GetId() <<" TIMEEXPIRE TO: " << vehicle2.GetTimeExpire());
}

void SOCIAL::SendBeacon () {
	//NS_LOG_FUNCTION(this);

	time_t ini, end;

	time(&ini);

	PrintInLog("Sending Beacon Message TO " + std::to_string(m_score) + " VIZINHOS.");

	BeaconHeader::VehicleInfo info;
	//BeaconHeader::VehicleInfo info2;

	info.id = (uint16_t)m_node->GetId();

	/////////STR/////////
	info.score = m_score;
	info.eigenScore = m_eigenScore;
	info.closenessScore = m_closenessScore;

	info.atividadeSocial = m_atividadeSocial;
	//info.amizade = m_amizade;
	//info.isRelay = m_isRelay;
	info.relayId = m_relayId;
	info.isPerfilSocial = m_isPerfilSocial;
	info.isGateway = m_isGateway;
	info.position = m_mobilityModel->GetPosition();
	info.direction = m_mobilityModel->GetVelocity();

	cout << m_node->GetId() << ": " << m_mobilityModel->GetVelocity().x << " " << m_mobilityModel->GetVelocity().y << " " << m_mobilityModel->GetVelocity().z << " " << endl;

	/*if(m_neighborsList.size() > 30){
		BeaconHeader header;
		BeaconHeader header2;

		vector<vector<uint32_t>> neighborsList;
		vector<vector<uint32_t>> neighborsList2;


		header.SetSeq(m_sentCounter++);
		//header2.SetSeq(m_sentCounter);

		header.SetVehicleInfo(info);
		header2.SetVehicleInfo(info2);

		for (uint32_t i = 0; i < m_neighborsList.size(); i++){
			if(i<30)
				neighborsList.push_back(m_neighborsList[i]);
			else
				neighborsList2.push_back(m_neighborsList[i]);
		}

		header.SetNeighborsList(neighborsList);
		header2.SetNeighborsList(neighborsList2);

		SendMessage(header);
		SendMessage(header2);
	} else{

		BeaconHeader header;
		header.SetSeq(m_sentCounter++);
		header.SetVehicleInfo(info);
		header.SetNeighborsList(m_neighborsList);

		SendMessage(header);
	}*/

	BeaconHeader header;
	header.SetSeq(m_sentCounter++);
	header.SetLenNeighborsList(m_neighborsList.size());
	header.SetVehicleInfo(info);
	header.SetNeighborsList(m_neighborsList);

	SendMessage(header);

	m_sendEvent = Simulator::Schedule (Seconds(SOCIALUtils::BEACON_MESSAGE_TIMER), &SOCIAL::SendBeacon, this);

	time(&end);

	//double tempo = end - ini;

	//std::cout << "A função SendBeacon foi executada em " << tempo << " segundos.\n";
	//NS_LOG_INFO("SENDING THIS INFO: ID: " << info.id << " SCORE: " << info.score << " POSITION: " << info.position);

	/*if((int) Simulator::Now().GetSeconds() == 29000){
		NS_LOG_DEBUG("Total de veículos na comunidade -> " << MinuetConfig::GetTotalNodes());
		RelayElectionAmizade();
	}*/
}

void SOCIAL::SendMessage (BeaconHeader header) {
	//NS_LOG_FUNCTION(this);

	time_t ini, end;

	time(&ini);

	//NS_LOG_DEBUG("NODE: "<< m_node->GetId() <<" SENDING BEACON MESSAGE IN BROADCAST!");

	/*if(header.GetNeighborsList().size() > 80){
		BeaconHeader headerAux;
		BeaconHeader headerAux2;

		headerAux.SetSeq(header.GetSeq());
		headerAux2.SetSeq(m_sentCounter++);

		headerAux.SetLenNeighborsList(header.GetLenNeighborsList());
		headerAux2.SetLenNeighborsList(header.GetLenNeighborsList());

		headerAux.SetVehicleInfo(header.GetVehicleInfo());
		headerAux2.SetVehicleInfo(header.GetVehicleInfo());

		vector<vector<uint16_t>> neighborsList;
		vector<vector<uint16_t>> neighborsList2;

		for(uint32_t i=0; i<header.GetNeighborsList().size(); i++){
			if(i<(header.GetNeighborsList().size()/2))
				neighborsList.push_back(header.GetNeighborsList()[i]);
			else
				neighborsList2.push_back(header.GetNeighborsList()[i]);
		}

		headerAux.SetNeighborsList(neighborsList);
		headerAux2.SetNeighborsList(neighborsList2);


		Ptr<Packet> packet = Create<Packet>();
		Ptr<Packet> packet2 = Create<Packet>();

		packet->AddHeader(headerAux);
		packet2->AddHeader(headerAux2);

		m_communicationLayer->SendControlMenssage(packet);
		m_communicationLayer->SendControlMenssage(packet2);
	} else{*/

	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(header);


	//NS_LOG_DEBUG("NODE: "<< m_node->GetId() << " TEM " << m_score << " VIZINHOS, FAZ PARTE DO PERFIL SOCIAL " << m_isPerfilSocial << " E É RETRANSMISSOR " << m_isRelay);
	m_communicationLayer->SendControlMenssage(packet);
	//}

	time(&end);

	//double tempo = end - ini;

	//std::cout << "A função SendMessage foi executada em " << tempo << " segundos.\n";
}

double_t SOCIAL::GetAngleDiff (Vector v1, Vector v2) {
	NS_LOG_FUNCTION(this);

	double_t diff = 0.0;

	double l1 = v1.GetLength();
	double l2 = v2.GetLength();

	if ((l1 != 0) && (l2 != 0)) {
		diff = acos(((v1.x*v2.x) + (v1.y*v2.y)) / (l1*l2));
	}

	return (diff * 180) / PI;   //Convert from Radius to Degrees
}

bool SOCIAL::IsRelay () {
	//NS_LOG_FUNCTION(this);

	return m_isRelay && (m_relayId == m_node->GetId());
}

bool SOCIAL::IsGateway () {
	//NS_LOG_FUNCTION(this);

	return m_isGateway;
}

void SOCIAL::SetIsGateway(bool isGateway) {
	m_isGateway = isGateway;
}

bool SOCIAL::IsPerfilSocial () {
	//NS_LOG_FUNCTION(this);

	return m_isPerfilSocial;
}
bool SOCIAL::IsClusterMember () {
	NS_LOG_FUNCTION(this);

	return !m_isRelay && (m_relayId != m_node->GetId());
}

bool SOCIAL::IsIsolated () {
	NS_LOG_FUNCTION(this);

	return false;
}

uint16_t SOCIAL::GetRelayId(uint16_t op) {
	//NS_LOG_FUNCTION(this);

	/*if(IsRelay() || IsClusterMember()) {
		return m_relayId;
	} else {
		cerr << "## ERROR: In GetClusterId method node is not Cluster Head or Member Cluster" << endl;
		exit(1);
	}*/

	time_t ini, end;

	time(&ini);

	switch(op){
	case 0:
		wBC = wCC = wDC = 0.9/3;
		wEC = 0.1;
		break;
	case 1:
		wBC = wCC = wDC = 0.9/3;
		wEC = 0.1;
		break;
	case 2:
		wBC = wCC = wDC = 0.75/3;
		wEC = 0.25;
		break;
	case 3:
		wBC = wCC = wDC = 0.66/3;
		wEC = 0.34;
		break;
	case 4:
		wBC = wCC = wDC = 0.5/3;
		wEC = 0.5;
		break;
	case 5:
		wBC = wCC = wDC = 0.34/3;
		wEC = 0.66;
		break;
	case 6:
		wBC = wCC = wDC = 0.25/3;
		wEC = 0.75;
		break;
	case 7:
		wBC = wCC = wDC = 0.1/3;
		wEC = 0.9;
		break;
	default:
		wBC = wCC = wDC = 0.1/3;
		wEC = 0.9;
		break;
	}

	time(&end);

	//double tempo = end - ini;

	//std::cout << "A função GetRelayId foi executada em " << tempo << " segundos.\n";

	return RelayElection();
	//return RelayElectionSTR();
	//return RelayElectionAtivSocial();
	//return RelayElectionAmizade();
}

bool SOCIAL::IsStarted() {
	return m_clusteringStarted;
}

void SOCIAL::EventsListMaintenance () {
	//NS_LOG_FUNCTION(this);

	for(vector<EventId>::iterator eventIdIt =  scheduledCheckEvents.begin() ; eventIdIt < scheduledCheckEvents.end() ; eventIdIt++) {
		if((*eventIdIt).IsExpired()) {
			eventIdIt = scheduledCheckEvents.erase(eventIdIt);
		}
	}

	m_maintenanceEvent = Simulator::Schedule (Seconds(SOCIALUtils::EVENTS_LIST_MAINTENANCE_TIMER), &SOCIAL::EventsListMaintenance, this);
}

void SOCIAL::PrintInLog(string message) {
	time_t ini, end;

	time(&ini);
	ofstream os;
	os.open (MinuetConfig::LOG_FILE_CLUSTERING_ALGORITHM.c_str(), ofstream::out | ofstream::app);
	os << Simulator::Now().GetNanoSeconds() << "ns - SOCIAL - Node #" << m_node->GetId() << " Pos (" << m_mobilityModel->GetPosition() << ") GroupId #" << m_relayId << " : " << message << endl;
	os.close();
	time(&end);

	//double tempo = end - ini;

	//std::cout << "A função PrintInLog (Social) foi executada em " << tempo << " segundos.\n";
}

void SOCIAL::PrintInLogFCV(string message) {
	time_t ini, end;

	time(&ini);
	ofstream os;
	os.open (MinuetConfig::LOG_FILE_FCV.c_str(), ofstream::out | ofstream::app);
	
	/*
	os << Simulator::Now().GetNanoSeconds() << "ns - SOCIAL - Node #" << m_node->GetId() << " Pos (" << m_mobilityModel->GetPosition() << ") GroupId #" << m_relayId << " : " << message << endl;
	os.close();
	time(&end);
	*/

	os << message << endl;
	os.close();

	//double tempo = end - ini;

	//std::cout << "A função PrintInLog (Social) foi executada em " << tempo << " segundos.\n";
}

void SOCIAL::PrintInGetEdgesOnAnInstant(string message) {
	time_t ini, end;

	time(&ini);
	ofstream os;
	string path = "ns-3.28/src/minuet/utils/log/EdgesOnAnInstant-AD.csv";
	os.open (path.c_str(), ofstream::out | ofstream::app);
	os << message << endl;
	os.close();
	time(&end);

	//double tempo = end - ini;

	//std::cout << "A função PrintInLog (Social) foi executada em " << tempo << " segundos.\n";
}
}

