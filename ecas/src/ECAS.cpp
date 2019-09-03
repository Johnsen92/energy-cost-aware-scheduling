#include "ECAS.h"

// ##########################################################################
// Constructors & Destructors                                             ###
// ##########################################################################

ECAS::ECAS(Instance instance) {
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
	int nTaskFragments = 0;

	// Calculate number of task fragments
	for(Task t: instance.tasks){
		nTaskFragments += t.duration;
	}

	IloIntExprArray ends(env);
	IloNumExpr energy_costs(env);

	// Stepwise linear function for energy prices
	IloNumToNumStepFunction energy_prices(env);
	for(int i=0; i<time_slots; i++){
		energy_prices.setValue(i,i+1,instance.energy_prices[i]);
	}

	// Tasks
	IloIntervalVarArray tasks(env, nTaskFragments);
	int c = 0;
	for(Task t:instance.tasks){
		for(int i=0; i<t.duration; i++){
			tasks[c] = IloIntervalVar(env, 1);
			tasks[c].setStartMin(t.earliest_start_time + i);
			tasks[c].setEndMax(t.latest_end_time - t.duration + i + 1);
			if(i != 0)
				model.add(IloStartAtEnd(env, tasks[c], tasks[c-1], 0));
			//ends.add(IloEndOf(tasks[c+i]));
			energy_costs += IloStartEval(tasks[c], energy_prices, 0)*t.duration*t.power_consumption;
			c++;
		}
	}

	// Task to Machine matching
	IloIntervalVarArray2 taskOnMachine(env, nTaskFragments);
	c = 0;
	for(Task t:instance.tasks){
		for(int i=0; i<t.duration; i++){
			taskOnMachine[c] = IloIntervalVarArray(env, nMach);
			for(Machine m:instance.machines) {
				taskOnMachine[c][m.id] = IloIntervalVar(env, 1);
				taskOnMachine[c][m.id].setOptional();
			}
			//model.add(IloAlternative(env, tasks[c], taskOnMachine[c]));
			c++;
		}
		model.add(IloAlternative(env, tasks[c-t.duration], taskOnMachine[c-t.duration]));
	}

	// Machine state constraint
	IloStateFunctionArray machineStateFunctions(env, nMach);
	for(Machine m: instance.machines){
		string name = "Machine_state_" + m.id;
		machineStateFunctions[m.id] = IloStateFunction(env, name.c_str());
		model.add(IloAlwaysEqual(env, machineStateFunctions[m.id], 0, 0, MACHINE_OFF, true, true));
		model.add(IloAlwaysEqual(env, machineStateFunctions[m.id], time_slots, time_slots, MACHINE_OFF, true, true));
	}

	// Machine to Task matching
	IloIntervalVarArray2 machineHasTask(env, nMach);
	for(Machine m:instance.machines){
		c = 0;
		machineHasTask[m.id] = IloIntervalVarArray(env, nTaskFragments);
		for(Task t:instance.tasks){
			for(int i=0; i<t.duration; i++){
				machineHasTask[m.id][c] = taskOnMachine[c][m.id];
				if(i != 0)
					model.add(IloIfThen(env, IloPresenceOf(env, machineHasTask[m.id][c-1]), IloPresenceOf(env, machineHasTask[m.id][c])));
				model.add(IloAlwaysIn(env, machineStateFunctions[m.id], machineHasTask[m.id][c], MACHINE_ON, MACHINE_ON));
				c++;
			}
		}
	}

	// Add resource constraints
	IloCumulFunctionExprArray machine_cpu_res(env, nMach);
	IloCumulFunctionExprArray machine_mem_res(env, nMach);
	IloCumulFunctionExprArray machine_io_res(env, nMach);
	for(Machine m: instance.machines){
		machine_cpu_res[m.id] = IloCumulFunctionExpr(env);
		machine_mem_res[m.id] = IloCumulFunctionExpr(env);
		machine_io_res[m.id] = IloCumulFunctionExpr(env);
		c = 0;
		for(Task t: instance.tasks){
			for(int i = 0; i<t.duration; i++){
				machine_cpu_res[m.id] += IloPulse(machineHasTask[m.id][c],t.cpu_usage());
				machine_mem_res[m.id] += IloPulse(machineHasTask[m.id][c],t.memory_usage());
				machine_io_res[m.id] += IloPulse(machineHasTask[m.id][c],t.io_usage());
				c++;
			}
		}
		model.add(machine_cpu_res[m.id] <= m.cpu_cap());
		model.add(machine_mem_res[m.id] <= m.memory_cap());
		model.add(machine_io_res[m.id] <= m.io_cap());
	}

  // add objective function to model
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

