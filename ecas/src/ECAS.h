#ifndef __ECAS__H__
#define __ECAS__H__

#include <ilcp/cp.h>
#include "Tools.h"
#include "Instance.h"

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
	IloCP cp;

  IloIntervalVarArray tasks;
  IloIntervalVarArray2 machines;


  // Statistic Variables
  void initCP();
  void initModel(Instance instance);
  
public:
	ECAS(Instance instance);
	~ECAS();
  
	stats_t solveModel();
};

#endif //__ECAS__H__
