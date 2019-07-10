#include "ECAS.h"

// ##########################################################################
// Constructors & Destructors                                             ###
// ##########################################################################

ECAS::ECAS( ) : epInt( 0.0 ), epOpt( 0.0 ) {
  this->initCPLEX();
  this->initModel();
}

ECAS::~ECAS() {
	// free CPLEX resources
	cplex.end();
	model.end();
	env.end();
}

// ##########################################################################
// Private ethods                                                         ###
// ##########################################################################

void ECAS::initCPLEX() {
	cout << "initialize CPLEX ... ";
	try {
		env = IloEnv();
		model = IloModel( env );
	}
	catch( IloException& e ) {
		cerr << "ECAS: exception " << e.getMessage();
	}
	catch( ... ) {
		cerr << "ECAS: unknown exception.\n";
	}
	cout << "done.\n";
}

void ECAS::initModel() {
  cout << "initialize Model ... ";

  //TODO: Implement variables
  // initialize node variables
	/*for(u_int i=0; i<n; i++){
		stringstream name_node;
		name_node << "z_" << i;
    x[i] = IloBoolVar(env, name_node.str().c_str());
    y[i] = IloBoolVar(env, name_node.str().c_str());
		z[i] = IloBoolVar(env, name_node.str().c_str());
	}*/
  x1 = IloNumVar(env, "x1");
  x2 = IloNumVar(env, "x2");
  x3 = IloNumVar(env, "x3");

  //TODO: Implement constrains
  IloExpr c1(env);
  IloExpr c2(env);
  c1 += x1*x2;
  c2 += x3;
  model.add(0 <= c1 <= 20);
  model.add(0 <= c2 <= 30);
	c1.end();
  c2.end();

  //TODO: Implement objective function
  IloExpr objt(env);
  objt += x1;
  objt += x2;
  objt += x3;

  // add objective function to model
	model.add(IloMaximize(env, objt));
	objt.end();

  cout << "done.\n";
}

// ##########################################################################
// Public Methods                                                         ###
// ##########################################################################

stats_t ECAS::solveModel(){
  stats_t stats;
  try {
		// build model
		cplex = IloCplex( model );
		cplex.exportModel( "model.lp" );

		// set parameters
		epInt = cplex.getParam( IloCplex::EpInt );
		epOpt = cplex.getParam( IloCplex::EpOpt );

		// set number of threads
		cplex.setParam( IloCplex::Threads, 8 );

		// solve model
		cout << "Calling CPLEX solve ...\n";
		cplex.solve();
		cout << "CPLEX finished.\n\n";
		cout << "CPLEX status: " << cplex.getStatus() << "\n";
		cout << "Branch-and-Bound nodes: " << cplex.getNnodes() << "\n";
		cout << "Objective value: " << cplex.getObjValue() << "\n";
		cout << "CPU time: " << Tools::CPUtime() << "\n\n";

		// get variable values for edge decision variable x
		//values = IloNumArray(env, m);
		//cplex.getValues(values, x);

    // return statistics
		stats.cpu_time = Tools::CPUtime();
		stats.bnb_nodes = cplex.getNnodes();

	}
	catch( IloException& e ) {
		cerr << "ECAS: exception " << e.getMessage();
		exit( -1 );
	}
	catch( ... ) {
		cerr << "ECAS: unknown exception.\n";
		exit( -1 );
	}
	return stats;
}

