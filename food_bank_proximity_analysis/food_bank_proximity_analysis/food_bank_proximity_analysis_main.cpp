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
	vector<coordinate> homes;
	vector<coordinate> foodBanks;
	ifstream inHomes;
	ifstream inFoodBanks;
	inHomes.open("Residences.dat");
	inFoodBanks.open("FoodBanks.dat");
	double countAddr = 0;
	double countDis1 = 0;
	double countDis2 = 0;
	double countDis3 = 0;
	double countDis4 = 0;

	cout << "Proximity of Residential Addresses to Foodbanks in Toronto" << endl;
	cout << "----------------------------------------------------------" << endl << endl;

	double startTime = MPI_Wtime();

	while(!inHomes.eof()) {
		countAddr++;
		string line;
		string coord;
		coordinate coords;
		getline(inHomes, line, '\n');
		stringstream ss(line);
		int count = 0;
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
	inHomes.close();
	inFoodBanks.close();

	//calcDis(homes[0], foodBanks[0]);

	// Get the shortest distance to a food bank for each home
	for(int i = 0; i < homes.size(); i++) {
		double dis = 0;
		double shortestDis = 1000;
		for(int j = 0; j < foodBanks.size(); j++) {
			dis = calcDis(homes[i], foodBanks[j]);
			
			if(dis < shortestDis)
				shortestDis = dis;
		}


		if(shortestDis <= 1)
			countDis1++;
		else if(shortestDis <= 2)
			countDis2++;
		else if(shortestDis <= 5)
			countDis3++;
		else if(shortestDis > 5)
			countDis4++;
	}

	startTime = MPI_Wtime() - startTime;

	cout << "Number of Processes:\t\t" << numProcs << endl;
	cout << "Elapsed Time:\t\t\t" << startTime << " seconds" << endl << endl;
	cout << "Process #1 results for " << countAddr << " addresses..." << endl;
	cout << "Nearest Foodbank(km)" << setw(28) << "# of Addresses" << setw(28) << "% of Addresses" << endl;
	cout << "--------------------" << setw(28) << "--------------" << setw(28) << "--------------" << endl;
	cout << "0 - 1" << setw(40) << right << countDis1 << setw(28) << right << (countDis1/countAddr)*100 << endl;
	cout << "1 - 2" << setw(40) << right << countDis2 << setw(28) << right << (countDis2/countAddr)*100 << endl;
	cout << "2 - 5" << setw(40) << right << countDis3 << setw(28) << right << (countDis3/countAddr)*100 << endl;
	cout << "  > 5" << setw(40) << right << countDis4 << setw(28) << right << (countDis4/countAddr)*100 << endl;
}

void processSlave(int rank, int numProcs) {

}
int main( int argc, char* argv[] ) {
	if(MPI_Init(&argc, &argv) == MPI_SUCCESS) {
		// Get the number of processes
		int numProcs;
		MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
		
		// Get the rank of the current process
		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		//processMaster(rank, numProcs);
		try {
			if( rank == 0 )
				processMaster(rank, numProcs);
			else
				processSlave(rank, numProcs);
		}
		catch(exception ex) {
			cerr << ex.what() << endl;
		}
		
		MPI_Finalize();
	}
}