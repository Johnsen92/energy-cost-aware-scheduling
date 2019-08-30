#include "ECAS.h"

// ##########################################################################
// Constructors & Destructors                                             ###
// ##########################################################################

ECAS::ECAS(Instance instance) {
	this->tasks = IloIntervalVarArray(env, instance.tasks.size());
	this->machines = IloIntervalVarArray2(env, instance.machines.size());
  this->initCP();
  this->initModel(instance);
}

ECAS::~ECAS() {
	// free CP resources
	cp.end();
	model.end();
	env.end();
}

// ##########################################################################
// Private ethods                                                         ###
// ##########################################################################

void ECAS::initCP() {
	cout << "initialize CP ... ";
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

void ECAS::initModel(Instance instance) {
  cout << "initialize Model ... ";

	int time_slots = 24*60/instance.time_resolution;
	int nMach = instance.machines.size();
	int nTasks = instance.tasks.size();

	IloIntExprArray ends(env);
	IloNumExpr energy_costs(env);

	// Stepwise linear function for energy prices
	IloNumToNumStepFunction energy_prices(env);
	for(int i=0; i<time_slots; i++){
		energy_prices.setValue(i,i+1,instance.energy_prices[i]);
	}

	// Tasks
	for(Task t:instance.tasks){
		tasks[t.id] = IloIntervalVar(env, t.duration, t.name.c_str());
		tasks[t.id].setStartMin(t.earliest_start_time);
		tasks[t.id].setEndMax(t.latest_end_time);
		//tasks[t.id].setIntensity(energy_prices,max_energy_price);
		ends.add(IloEndOf(tasks[t.id]));
		energy_costs += IloStartEval(tasks[t.id], energy_prices, 0)*t.power_consumption;
		//energy_costs += energy_prices.getValue(tasks[t.id]);
	}

	// Task to Machine matching
	IloIntervalVarArray2 taskOnMachine(env, nTasks);
	for(Task t:instance.tasks){
		taskOnMachine[t.id] = IloIntervalVarArray(env, nMach);
		for(Machine m:instance.machines) {
			taskOnMachine[t.id][m.id] = IloIntervalVar(env, t.duration, t.name.c_str());
			taskOnMachine[t.id][m.id].setOptional();
			//taskOnMachine[t.id][m.id].setIntensity(energy_prices,max_energy_price);
		}
		model.add(IloAlternative(env, tasks[t.id], taskOnMachine[t.id]));
	}

	// Machine to Task matching
	IloIntervalVarArray2 machineHasTask(env, nMach);
	for(Machine m:instance.machines){
		machineHasTask[m.id] = IloIntervalVarArray(env, nTasks);
		for(Task t:instance.tasks){
			machineHasTask[m.id][t.id] = taskOnMachine[t.id][m.id];
		}
	}

	// Add resource constraints
	IloCumulFunctionExprArray machine_cpu_res(env, nMach);
	IloCumulFunctionExprArray machine_mem_res(env, nMach);
	IloCumulFunctionExprArray machine_io_res(env, nMach);
	IloCumulFunctionExpr power_usage(env);
	for(Machine m: instance.machines){
		machine_cpu_res[m.id] = IloCumulFunctionExpr(env);
		machine_mem_res[m.id] = IloCumulFunctionExpr(env);
		machine_io_res[m.id] = IloCumulFunctionExpr(env);
		for(Task t: instance.tasks){
			power_usage += IloPulse(machineHasTask[m.id][t.id],t.power_consumption+m.idle_consumption);
			machine_cpu_res[m.id] += IloPulse(machineHasTask[m.id][t.id],t.cpu_usage());
			machine_mem_res[m.id] += IloPulse(machineHasTask[m.id][t.id],t.memory_usage());
			machine_io_res[m.id] += IloPulse(machineHasTask[m.id][t.id],t.io_usage());
		}
		model.add(machine_cpu_res[m.id] <= m.cpu_cap());
		model.add(machine_mem_res[m.id] <= m.memory_cap());
		model.add(machine_io_res[m.id] <= m.io_cap());
	}

  // add objective function to model
	//IloObjective objective = IloMinimize(env, IloMax(ends));
	IloObjective objective = IloMinimize(env, energy_costs);
  model.add(objective);

  cout << "done.\n";
}

// ##########################################################################
// Public Methods                                                         ###
// ##########################################################################

stats_t ECAS::solveModel(){

	cout << "solving Model ... ";
  stats_t stats;
  try {
		// build model
		cp = IloCP( model );
		cp.exportModel( "model.cpo" );

		// solve model
		cout << "Calling CP solve ...\n";
		cp.solve();
		cout << "done." << endl;
		cout << "CP finished.\n\n";
		cout << "CP status: " << cp.getStatus() << "\n";
		cout << "Objective value: " << cp.getObjValue() << "\n";
		cout << "CPU time: " << Tools::CPUtime() << "\n\n";

		// get variable values for edge decision variable x
		//values = IloNumArray(env, m);
		//cplex.getValues(values, x);

    // return statistics
		stats.cpu_time = Tools::CPUtime();
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

