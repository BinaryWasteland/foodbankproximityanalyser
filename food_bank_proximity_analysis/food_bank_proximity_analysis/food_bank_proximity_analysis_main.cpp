#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <mpi.h>
#include <math.h>
#include <algorithm>
#include "food_bank_proximity_analysis_helper.hpp"
using namespace std;

// Global constants
const int MSG_TAG_DATA = 0, MSG_TAG_DONE = 1;
ifstream inHomes("Residences.dat");
ifstream inFoodBanks("FoodBanks.dat");
vector<coordinate> foodBanks;

MPI_Datatype type() {
	int blocklen[] = {5, 5};
	MPI_Aint indices[2];
	indices[0] = 0;
	MPI_Type_extent(MPI_INT, &indices[1]);
	indices[1] *= 5;
	MPI_Datatype oldtype[] = {MPI_INT, MPI_DOUBLE};
	MPI_Datatype newType;
	MPI_Type_create_struct(2, blocklen, indices, oldtype, &newType);
	MPI_Type_commit(&newType);

	return newType;
}

void readInFoodBanks() {
	int count = 0;
	coordinate coords;
	while(inFoodBanks >> coords.x_ >> coords.y_) {
		foodBanks.push_back(coords);
	}
	inFoodBanks.close();
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

		cout << "Proximity of Residential Addresses to Foodbanks in Toronto" << endl;
		cout << "----------------------------------------------------------" << endl << endl;

		int alternateCount = 0;
		coordinate coord;
		while(inHomes >> coord.x_ >> coord.y_) {
			if(alternateCount++ % numProcs == rank) {
				double shortestDis = 1000;
				for(unsigned int j = 0; j < foodBanks.size(); j++) {
					double dis = calcDis(coord, foodBanks[j]);
			
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
		}
		inHomes.close();

		count.percentCount1 = (static_cast<double>(count.count1)/static_cast<double>(count.countAddr))*100;
		count.percentCount2 = (static_cast<double>(count.count2)/static_cast<double>(count.countAddr))*100;
		count.percentCount3 = (static_cast<double>(count.count3)/static_cast<double>(count.countAddr))*100;
		count.percentCount4 = (static_cast<double>(count.count4)/static_cast<double>(count.countAddr))*100;
		
		MPI_Datatype newType = type();
		
		counts* recv = new counts[numProcs];

		MPI_Gather(&count, 1, newType, recv, 1, newType, 0, MPI_COMM_WORLD);

		startTime = MPI_Wtime() - startTime;

		cout << "Number of Processes:\t\t" << numProcs << endl;
		cout << "Elapsed Time:\t\t\t" << startTime << " seconds" << endl << endl;
		counts agg;
		agg.count1 = 0;
		agg.count2 = 0;
		agg.count3 = 0;
		agg.count4 = 0;
		agg.countAddr = 0;

		for(int i = 0; i < numProcs; ++i) {
			cout << "Process " << i + 1 << " results for " << recv[i].countAddr << " addresses..." << endl;
			cout << "Nearest Foodbank(km)" << setw(28) << "# of Addresses" << setw(28) << "% of Addresses" << endl;
			cout << "--------------------" << setw(28) << "--------------" << setw(28) << "--------------" << endl;
			cout << "0 - 1" << setw(40) << right << recv[i].count1 << setw(28) << right << recv[i].percentCount1 << endl;
			cout << "1 - 2" << setw(40) << right << recv[i].count2 << setw(28) << right << recv[i].percentCount2 << endl;
			cout << "2 - 5" << setw(40) << right << recv[i].count3 << setw(28) << right << recv[i].percentCount3 << endl;
			cout << "  > 5" << setw(40) << right << recv[i].count4 << setw(28) << right << recv[i].percentCount4 << endl << endl;

			//Aggregate results
			agg.countAddr += recv[i].countAddr;
			agg.count1 += recv[i].count1;
			agg.count2 += recv[i].count2;
			agg.count3 += recv[i].count3;
			agg.count4 += recv[i].count4;
		}

		if(numProcs > 1) {
			agg.percentCount1 = (static_cast<double>(agg.count1)/static_cast<double>(agg.countAddr))*100;
			agg.percentCount2 = (static_cast<double>(agg.count2)/static_cast<double>(agg.countAddr))*100;
			agg.percentCount3 = (static_cast<double>(agg.count3)/static_cast<double>(agg.countAddr))*100;
			agg.percentCount4 = (static_cast<double>(agg.count4)/static_cast<double>(agg.countAddr))*100;

			cout << "Aggregate results for all " << agg.countAddr << " addresses..." << endl;
			cout << "Nearest Foodbank(km)" << setw(28) << "# of Addresses" << setw(28) << "% of Addresses" << endl;
			cout << "--------------------" << setw(28) << "--------------" << setw(28) << "--------------" << endl;
			cout << "0 - 1" << setw(40) << right << agg.count1 << setw(28) << right << agg.percentCount1 << endl;
			cout << "1 - 2" << setw(40) << right << agg.count2 << setw(28) << right << agg.percentCount2 << endl;
			cout << "2 - 5" << setw(40) << right << agg.count3 << setw(28) << right << agg.percentCount3 << endl;
			cout << "  > 5" << setw(40) << right << agg.count4 << setw(28) << right << agg.percentCount4 << endl;
		}

		MPI_Type_free(&newType);
		delete [] recv;
	} catch( exception ex ) {
		cerr << ex.what() << endl;
	} catch(MPI::Exception ex) {
		cerr << ex.Get_error_string() << endl;
	}
}

void processSlave(int rank, int numProcs) {
	try {
		counts count;
		count.count1 = 0;
		count.count2 = 0;
		count.count3 = 0;
		count.count4 = 0;
		count.countAddr = 0;

		int alternateCount = 0;
		coordinate coord;
		while(inHomes >> coord.x_ >> coord.y_) {
			if(alternateCount++ % numProcs == rank) {
				double shortestDis = 1000;
				for(unsigned int j = 0; j < foodBanks.size(); j++) {
					double dis = calcDis(coord, foodBanks[j]);
			
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
		}
		inHomes.close();

		count.percentCount1 = (static_cast<double>(count.count1)/static_cast<double>(count.countAddr))*100;
		count.percentCount2 = (static_cast<double>(count.count2)/static_cast<double>(count.countAddr))*100;
		count.percentCount3 = (static_cast<double>(count.count3)/static_cast<double>(count.countAddr))*100;
		count.percentCount4 = (static_cast<double>(count.count4)/static_cast<double>(count.countAddr))*100;

		MPI_Datatype newType = type();
		
		counts* recv = new counts[numProcs];

		MPI_Gather(&count, 1, newType, recv, 1, newType, 0, MPI_COMM_WORLD);
	} catch( exception ex ) {
		cerr << ex.what() << endl;
	}
}

int main( int argc, char* argv[] ) {
	if(MPI_Init(&argc, &argv) == MPI_SUCCESS) {		
		int numProcs, rank;
		MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		readInFoodBanks();

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