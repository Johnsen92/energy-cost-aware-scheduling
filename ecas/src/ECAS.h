#ifndef __ECAS__H__
#define __ECAS__H__

#include <ilcplex/ilocplex.h>
#include "Tools.h"

using namespace std;

ILOSTLBEGIN

typedef struct
{
  double cpu_time;
  u_int bnb_nodes;
} stats_t;

class ECAS {
private:
  // CPLEX Variables
	IloEnv env;
	IloModel model;
	IloCplex cplex;

  IloNumVar x1;
  IloNumVar x2;
  IloNumVar x3;

  // Model Variables
  /*IloBoolVarArray x; // edge selection variables
	IloBoolVarArray z; // node section variables
	IloBoolVarArray y; // arc selection variables
	IloNumArray values; // to store result values of x*/

  // Statistic Variables
	double epInt, epOpt;
  void initCPLEX();
  void initModel();
  
public:
	ECAS();
	~ECAS();
  
	stats_t solveModel();
};

#endif //__ECAS__H__