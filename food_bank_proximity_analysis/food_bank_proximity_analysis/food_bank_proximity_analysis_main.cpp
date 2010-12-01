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
const int HOMES_BYTES = 11;
ifstream inHomes("Residences.dat");
ifstream inFoodBanks("FoodBanks.dat");
vector<coordinate> foodBanks;

void processMaster(int rank, int numProcs) 
{
	try	{
		vector<coordinate> homes;
		double countAddr = 0;
		double countDis1 = 0;
		double countDis2 = 0;
		double countDis3 = 0;
		double countDis4 = 0;

		cout << "Proximity of Residential Addresses to Foodbanks in Toronto" << endl;
		cout << "----------------------------------------------------------" << endl << endl;

		double startTime = MPI_Wtime();

		// Determine the number of SINs per process (not exact since the
		// number of processes may not divide evenly into the number of SINs)
		inHomes.seekg(0, ios::end); // move pointer to end of file
		int fBytes = static_cast<int>(inHomes.tellg()); // get file size in bytes
		int numHomesTotal = fBytes / HOMES_BYTES;
		int homesPerProcess = numHomesTotal / numProcs;
		
		// Move the file pointer to the apropriate starting position
		inHomes.seekg( rank * homesPerProcess * HOMES_BYTES, ios::beg);

		// Determine the exact number of SINs to be validated by THIS process
		int numHomesMine = homesPerProcess;
		if( rank == numProcs - 1 )
			// Add the unassigned SINs to the last slave's workload
			numHomesMine += numHomesTotal % numProcs;

		/* Obsolete for Parallel Computing
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

		inHomes.close();*/
		

		// Get the shortest distance to a food bank for each home
		for(unsigned int i = 0; i < homes.size(); i++) {
			double dis = 0;
			double shortestDis = 1000;
			for(unsigned int j = 0; j < foodBanks.size(); j++) {
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
		
		vector<int> counts;
		//MPI_Gather(&counts,);

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
	} catch( exception ex ) {
		cerr << ex.what() << endl;
	}
}

void processSlave(int rank, int numProcs) 
{
	try {
		// Determine the number of SINs per process (not exact since the
		// number of processes may not divide evenly into the number of SINs)
		inHomes.seekg(0, ios::end); // move pointer to end of file
		int fBytes = static_cast<int>(inHomes.tellg()); // get file size in bytes
		int numHomesTotal = fBytes / HOMES_BYTES;
		int homesPerProcess = numHomesTotal / numProcs;
		
		// Move the file pointer to the apropriate starting position
		inHomes.seekg( rank * homesPerProcess * HOMES_BYTES, ios::beg);

		// Determine the exact number of SINs to be validated by THIS process
		int numHomesMine = homesPerProcess;
		if( rank == numProcs - 1 )
			// Add the unassigned SINs to the last slave's workload
			numHomesMine += numHomesTotal % numProcs;
	} catch( exception ex ) {
		cerr << ex.what() << endl;
	}
}

int main( int argc, char* argv[] ) 
{
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
	inFoodBanks.close();

	if(MPI_Init(&argc, &argv) == MPI_SUCCESS) {
		// Get the number of processes
		int numProcs;
		MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
		
		// Get the rank of the current process
		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		try {
			if( rank == 0 )
				processMaster(rank, numProcs);
			else
				processSlave(rank, numProcs);
		} catch(exception ex) {
			cerr << ex.what() << endl;
		}
		
		MPI_Finalize();
	}
}