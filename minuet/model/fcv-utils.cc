#include "fcv-utils.h"

#include <cmath>
#include <ctime>
#include <chrono>

#include "ns3/simulator.h"
#include "ns3/vector.h"

#include <fstream>
#include <string>
#include <vector>
#include <map>

void LogAllScores(const std::string& methodName, 
                  const std::vector<int>& ids, 
                  const std::vector<std::vector<double>>& criteriaMatrix, 
                  const std::vector<double>& finalScores, 
                  uint64_t tempo) 
{
    const std::string folderPath = "/root/ns3/ns-allinone-3.29/ns-3.29/src/minuet/utils/log/";
    const std::string filePath = folderPath + "score_history_" + methodName + ".csv";

    std::ofstream logFile(filePath, std::ios::app);
    
    if (!logFile.is_open()) {
        std::cerr << "ERRO: Não foi possível abrir o arquivo de log para escrita: " << filePath << std::endl;
        return;
    }
    
    logFile.seekp(0, std::ios::end);
    if (logFile.tellp() == 0) {
        logFile << "ns,ID,0,C,D,N,V,A,E,I,T,M,ICR\n";
    }
    
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i >= criteriaMatrix.size() || i >= finalScores.size()) continue;
        logFile << tempo << "," << ids[i];
        for (const double& criterionValue : criteriaMatrix[i]) {
            logFile << "," << criterionValue;
        }
        logFile << "," << finalScores[i] << "\n";
    }

    logFile.close();
}

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

void printMatrix(vector<vector<double>> &matrix) {
	for (const auto &row : matrix) {
		for (double val : row)
			cout << val << " ";
		cout << endl;
	}
}

bool createDirectory(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        if (mkdir(path.c_str(), 0777) != 0) {
            return false;
        }
    }
    return true;
}

double VectorDistance(const ns3::Vector &v1, const ns3::Vector &v2) {
	return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}

double CalculateAgeVsPotencyScore(int pv, int im, int lp) {
    if (!lp) { // Veículos Leves
        // Itera sobre as linhas de potência
        for (int i = 1; i < 20; i++) {
            if (pv == faixa_matrix_leves[i][0]) {
                // Itera sobre as faixas etárias
                for (int j = 1; j < 7; j++) {
                    if (im >= faixa_matrix_leves[0][j] && im < faixa_matrix_leves[0][j + 1]) {
                        return faixa_matrix_leves[i][j];
                    }
                }
                // Trata a última faixa etária (80+)
                if (im >= faixa_matrix_leves[0][7]) {
                    return faixa_matrix_leves[i][7];
                }
            }
        }
    } else { // Veículos Pesados
        // Itera sobre as faixas etárias
        for (int i = 0; i < 6; i++) {
            if (im >= faixa_matrix_pesados[0][i] && im < faixa_matrix_pesados[0][i + 1]) {
                return faixa_matrix_pesados[1][i];
            }
        }
        // Trata a última faixa etária (70+)
        if (im >= faixa_matrix_pesados[0][6]) {
            return faixa_matrix_pesados[1][6];
        }
    }
    return 0.0;
}

double CalculateVehicleAgeScore(int manufactureYear) {
    int currentYear = 2025;

    int vehicleAge = std::max(0, currentYear - manufactureYear);

    if (vehicleAge <= 10) {return 1.00;}
    else if (vehicleAge <= 20) {return 0.75;} 
    else if (vehicleAge <= 40) {return 0.50;}
    else {return 0.25;}
}

double CalculateTimeTraveledScore(int tt) {
    if (tt > 0 && tt <= 2) {return 1.00;} 
    else if (tt <= 4) {return 0.75;} 
    else if (tt <= 8) {return 0.50;} 
    else {return 0.25;}
    return 0.0;
}

double CalculateTimeLicensedScore(int tc) {
    if (tc > 10) return 1.0;
    if (tc > 5) return 0.75;
    if (tc > 2) return 0.5;
    if (tc >= 1) return 0.25;
    return 0.0;
}

double CalculateFuelEfficiencyScore(int ec) {
    switch (ec) {
        case 0: return 1.0;
        case 1: return 0.8;
        case 2: return 0.7;
        case 3: return 0.4;
        default: return 0.2;
    }
}

double CalculateAverageSpeedScore(double magnitudeVelocity_kmh) {
    if (magnitudeVelocity_kmh >= 80.0) {return 1.00;}
    else if (magnitudeVelocity_kmh >= 60.0) {return 0.75;} 
    else if (magnitudeVelocity_kmh >= 40.0) {return 0.50;}
    else if (magnitudeVelocity_kmh > 0.0) {return 0.25;} 
    return 0.00;
}

double CalculateClusterVelocityScore(double vehicle_kmh, double cluster_avg_kmh) {
    if (cluster_avg_kmh <= 1.0) {return (vehicle_kmh <= 1.0) ? 1.0 : 0.0;}

    double ratio = vehicle_kmh / cluster_avg_kmh;

    if (ratio >= 0.8 && ratio < 1.5) {return 1.00;}
    else if ((ratio >= 0.6 && ratio < 0.8) || (ratio >= 1.5 && ratio < 2.0)) {return 0.75;}
    else if ((ratio >= 0.4 && ratio < 0.6) || (ratio >= 2.0 && ratio < 2.5)) {return 0.50;}
    else if ((ratio > 0.0 && ratio < 0.4) || (ratio >= 2.5 && ratio < 3.0)) {return 0.25;}
    else {return 0.00;}
}

double CalculateVehicleTypeScore(int vt) {
    switch (vt) {
        case 0: // Emergência
            return 1.00;
        case 1: // Transporte Público
            return 0.75;
        case 2: // Comercial (táxi, entrega, etc.)
            return 0.50;
        case 3: // Particular
            return 0.25;
        default:
            return 0.25;
    }
}
