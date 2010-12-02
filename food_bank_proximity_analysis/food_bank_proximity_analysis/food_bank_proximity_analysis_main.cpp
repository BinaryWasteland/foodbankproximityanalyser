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
ifstream inHomes("Residences.dat");
ifstream inFoodBanks("FoodBanks.dat");
vector<coordinate> foodBanks;
vector<coordinate> homes;

void readInFiles() {
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

	while(!inHomes.eof()) {
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
	inHomes.close();
}

void processMaster(int rank, int numProcs) {
	try	{
		double startTime = MPI_Wtime();
		counts count;
		count.count1 = 0;
		count.count2 = 0;
		count.count3 = 0;
		count.count4 = 0;
		count.countAddr = 0;
		vector<counts> recv;

		cout << "Proximity of Residential Addresses to Foodbanks in Toronto" << endl;
		cout << "----------------------------------------------------------" << endl << endl;

		readInFiles();

		int homesPerProcess = homes.size() / numProcs;
		if( rank == numProcs - 1 )
			homesPerProcess += homes.size() % numProcs;

		// Get the shortest distance to a food bank for each home
		for(int i = (rank * homesPerProcess); i < ((rank + 1) * homesPerProcess); i++) {
			double dis = 0;
			double shortestDis = 1000;
			for(unsigned int j = 0; j < foodBanks.size(); j++) {
				dis = calcDis(homes[i], foodBanks[j]);
			
				if(dis < shortestDis)
					shortestDis = dis;
			}

			if(shortestDis <= 1)
				count.count1++;
			else if(shortestDis <= 2)
				count.count2++;
			else if(shortestDis <= 5)
				count.count3++;
			else if(shortestDis > 5)
				count.count4++;
			count.countAddr++;
		}
		
		int blocklen[] = {5};
		int indices[1];
		indices[0] = 0;
		MPI_Datatype oldtype[] = {MPI_INT};
		MPI_Datatype newType;
		MPI_Type_create_struct(1, blocklen, indices, oldtype, &newType);
		MPI_Type_commit(&newType);
		
		MPI_Gather(&count, 1, newType, &recv, 1, newType, 0, MPI_COMM_WORLD);

		MPI_Type_free(&newType);

		startTime = MPI_Wtime() - startTime;

		cout << "Number of Processes:\t\t" << numProcs << endl;
		cout << "Elapsed Time:\t\t\t" << startTime << " seconds" << endl << endl;
		cout << "Process #1 results for " << homesPerProcess << " addresses..." << endl;
		cout << "Nearest Foodbank(km)" << setw(28) << "# of Addresses" << setw(28) << "% of Addresses" << endl;
		cout << "--------------------" << setw(28) << "--------------" << setw(28) << "--------------" << endl;
		cout << "0 - 1" << setw(40) << right << count.count1 << setw(28) << right << (count.count1/homesPerProcess)*100 << endl;
		cout << "1 - 2" << setw(40) << right << count.count2 << setw(28) << right << (count.count2/homesPerProcess)*100 << endl;
		cout << "2 - 5" << setw(40) << right << count.count3 << setw(28) << right << (count.count3/homesPerProcess)*100 << endl;
		cout << "  > 5" << setw(40) << right << count.count4 << setw(28) << right << (count.count4/homesPerProcess)*100 << endl;
	} catch( exception ex ) {
		cerr << ex.what() << endl;
	} catch(MPI::Exception ex) {
		cerr << ex.Get_error_string() << endl;
	}
}

void processSlave(int rank, int numProcs) {
	try {
		counts count;
		int homesPerProcess = homes.size() / numProcs;
		if( rank == numProcs - 1 )
			homesPerProcess += homes.size() % numProcs;

		// Get the shortest distance to a food bank for each home
		for(int i = (rank * homesPerProcess); i < ((rank + 1) * homesPerProcess); i++) {
			double dis = 0;
			double shortestDis = 1000;
			for(unsigned int j = 0; j < foodBanks.size(); j++) {
				dis = calcDis(homes[i], foodBanks[j]);
			
				if(dis < shortestDis)
					shortestDis = dis;
			}

			if(shortestDis <= 1)
				count.count1++;
			else if(shortestDis <= 2)
				count.count2++;
			else if(shortestDis <= 5)
				count.count3++;
			else if(shortestDis > 5)
				count.count4++;
			count.countAddr++;
		}
	} catch( exception ex ) {
		cerr << ex.what() << endl;
	}
}

int main( int argc, char* argv[] ) {
	if(MPI_Init(&argc, &argv) == MPI_SUCCESS) {		
		int numProcs, rank;
		MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
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