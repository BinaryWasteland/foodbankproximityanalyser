#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <mpi.h>
#include "food_bank_proximity_analysis_helper.hpp"
using namespace std;

// Global constants
const int MSG_TAG_DATA = 0, MSG_TAG_DONE = 1;

void processMaster(int rank, int numProcs) {
	double startTime = MPI_Wtime();
	vector<coordinate> homes;
	vector<coordinate> foodBanks;
	ifstream inHomes;
	ifstream inFoodBanks;
	inHomes.open("Residences.dat");
	inFoodBanks.open("FoodBanks.dat");

	cout << "Proximity of Residential Addresses to Foodbanks in Toronto" << endl;
	cout << "----------------------------------------------------------" << endl << endl;

	while(!inHomes.eof()) {
		string line;
		string coord;
		coordinate coords;
		int count = 0;
		getline(inHomes, line, '\n');
		stringstream ss(line);
		while(getline(ss, coord, ' ')) {
			if(count == 0)
				coords.x_ = atof(coord.c_str());
			else
				coords.y_ = atof(coord.c_str());
			count++;
		}

		homes.push_back(coords);
	}

	while(!inFoodBanks.eof()) {
		string line;
		string coord;
		coordinate coords;
		int count = 0;
		getline(inFoodBanks, line, '\n');
		stringstream ss(line);
		while(getline(ss, coord, ' ')) {
			if(count == 0)
				coords.x_ = atof(coord.c_str());
			else
				coords.y_ = atof(coord.c_str());
			count++;
		}

		foodBanks.push_back(coords);
	}

	//calcDis(homes[0], foodBanks[0]);

	startTime = MPI_Wtime() - startTime;

	cout << "Number of Processes:\t\t" << numProcs << endl;
	cout << "Elapsed Time:\t\t\t" << startTime << " seconds" << endl;
}

int main( int argc, char* argv[] ) {
	if(MPI_Init(&argc, &argv) == MPI_SUCCESS) {
		// Get the number of processes
		int numProcs;
		MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
		
		// Get the rank of the current process
		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		processMaster(rank, numProcs);
		/*try {
			if( rank == 0 )
				processMaster(rank, numProcs);
			else
				processSlave(rank, numProcs);
		}
		catch(exception ex) {
			cerr << ex.what() << endl;
		}*/
		
		MPI_Finalize();
	}
}