#include <stdio.h> /*an input-output library*/
#include <stdlib.h> /*other functions*/
#include <math.h> /*math library*/
#include <string> /*String Function Library*/
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <fstream>
#define MAXAA 50
using namespace std;


// Classes
class Gromacs {

	private:
	int i;

	public:

		float x1,y1,z1; /*Centroid*/
		float cumsum;
		float VEC1[MAXAA],VEC2[MAXAA],VEC3[MAXAA];


	// Calculate Weighted Average
	void Weighted_Ave(int Natoms,float O1X[MAXAA]) {		
		for(int i=0;i<Natoms;i++){
	   		cumsum+=O1X[i];
		}
		cumsum/=Natoms;	
	}
		
	// Calculate Center of Mass
	void COM(int Natoms, float O1X[MAXAA],float O1Y[MAXAA],float O1Z[MAXAA]){
	x1=y1=z1=0.0;	
		for(i=0;i<Natoms;i++){
	   		x1+=O1X[i];
			y1+=O1Y[i];
			z1+=O1Z[i];
		}

		x1/=Natoms;
		y1/=Natoms;
		z1/=Natoms;
		for(i=0;i<Natoms;i++){
			VEC1[i] = O1X[i]-x1;
			VEC2[i] = O1Y[i]-y1;
			VEC3[i] = O1Z[i]-z1;	
		}

	}

};

int main() {
  

// Read in Gromacs Coordinates
// 	Into
//	an	
//	Array
string line;
float O1X[MAXAA],O1Y[MAXAA],O1Z[MAXAA]; /*Atom Cartesian Coordinates*/
float x1,y1,z1; /*Centroid*/
char Atom_type[2],ResID[8];
int Natoms=26;
int AtomID;
  ifstream myfile ("trp.gro");
  if (myfile.is_open())
  {
  
  int i = 0;
  getline (myfile,line);
  getline (myfile,line);
    while ( i<Natoms)
    {
	 myfile >> ResID >> Atom_type >> AtomID >> O1X[i] >> O1Y[i] >> O1Z[i];
	 i++;
    }
    myfile.close();
  }

  else cout << "Unable to open file"; 

//
//
// Calculate Center of Mass

Gromacs mac;
mac.COM(Natoms, O1X, O1Y, O1Z);


}


