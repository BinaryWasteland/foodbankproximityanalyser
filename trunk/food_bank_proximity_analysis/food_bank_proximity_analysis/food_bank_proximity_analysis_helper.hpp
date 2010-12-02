#ifndef GUARD_food_bank_proximity_analysis_helper_hpp20101121_
#define GUARD_food_bank_proximity_analysis_helper_hpp20101121_

/** @file: food_bank_proximity_analysis_helper.hpp
@author Greg R. Jacobs and Garrett Wilson
	@author greg.r.jacobs@gmail.com and wierdworld77@hotmail.com
	@author http://gregrjacobs.com
	@date 2010-11-21
	@version 1.0.1
	@note Developed for Visual C++ 10.0
	@brief ...
	*/


/*=============================================================
Copyright

The copyright to the computer program(s) herein
is the property of Greg R. Jacobs and Garrett Wilson 
of Canada.  The program(s) may be used and/or copied 
only with the written permission of Greg R. Jacobs
or Garrett Wilson in accordance with the terms and conditions
stipulated in the agreement/contract under which
the program(s) have been supplied.
=============================================================*/

typedef struct {
	int countAddr, count1, count2, count3, count4;
	double percentCount1, percentCount2, percentCount3, percentCount4, holder;
} counts;

// Struct to hold variables
struct coordinate {
	long double x_;
	long double y_;
};

// Take in vectors of coordinates
long double calcDis(coordinate home, coordinate foodBank) {
	double dis = (sqrt(pow(home.x_ - foodBank.x_, 2) + pow(home.y_ - foodBank.y_, 2))/1000);
	return (sqrt(pow(home.x_ - foodBank.x_, 2) + pow(home.y_ - foodBank.y_, 2))/1000);
}

#endif // GUARD_food_bank_proximity_analysis_helper_hpp20101121_