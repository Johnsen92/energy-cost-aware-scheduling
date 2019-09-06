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

	// Constants for model parameters
	const int time_slots = 24*60/instance.time_resolution;
	const int nMach = instance.machines.size();
	const int nTasks = instance.tasks.size();


	// Objective function expression
	IloNumExpr energyCosts(env);
	IloNumArray energyPricesArray(env);

	// Stepwise linear function for energy prices
	IloNumToNumStepFunction energyPrices(env);
	for(int i=0; i<time_slots; i++){
		energyPrices.setValue(i,i+1,instance.energy_prices[i]);
		energyPricesArray.add(instance.energy_prices[i]);
	}

	// Initialize task intervals with min start time and max end time
	for(Task t:instance.tasks){
		tasks[t.id] = IloIntervalVar(env, t.duration, t.name.c_str());
		tasks[t.id].setStartMin(t.earliest_start_time);
		tasks[t.id].setEndMax(t.latest_end_time);

		// Add energy costs of tasks to objective function
		for(int i=0; i<t.duration; i++){
			energyCosts += energyPricesArray[IloStartOf(tasks[t.id]) + i]*t.power_consumption;
		}
			
	}

	// Task to machine matching
	IloIntervalVarArray2 taskOnMachine(env, nTasks);
	for(Task t:instance.tasks){
		taskOnMachine[t.id] = IloIntervalVarArray(env, nMach);
		for(Machine m:instance.machines) {
			taskOnMachine[t.id][m.id] = IloIntervalVar(env, t.duration, t.name.c_str());
			taskOnMachine[t.id][m.id].setOptional();
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
	IloCumulFunctionExprArray machineCPURes(env, nMach);
	IloCumulFunctionExprArray machineMemRes(env, nMach);
	IloCumulFunctionExprArray machineIORes(env, nMach);
	for(Machine m: instance.machines){
		machineCPURes[m.id] = IloCumulFunctionExpr(env);
		machineMemRes[m.id] = IloCumulFunctionExpr(env);
		machineIORes[m.id] = IloCumulFunctionExpr(env);
		for(Task t: instance.tasks){
			machineCPURes[m.id] += IloPulse(machineHasTask[m.id][t.id],t.cpu_usage());
			machineMemRes[m.id] += IloPulse(machineHasTask[m.id][t.id],t.memory_usage());
			machineIORes[m.id] += IloPulse(machineHasTask[m.id][t.id],t.io_usage());
		}
		model.add(machineCPURes[m.id] <= m.cpu_cap());
		model.add(machineMemRes[m.id] <= m.memory_cap());
		model.add(machineIORes[m.id] <= m.io_cap());
	}

	// Add machinePowerOn cumul function and constraints
	IloCumulFunctionExprArray machinePowerOn(env, nMach);
	IloIntervalVarArray2 machinePowerOnIntervals(env, nMach);
	for(Machine m: instance.machines){
		machinePowerOn[m.id] = IloCumulFunctionExpr(env);
		machinePowerOnIntervals[m.id] = IloIntervalVarArray(env, nTasks + 1);
		for(int i=0; i<nTasks + 1; i++){
			machinePowerOnIntervals[m.id][i] = IloIntervalVar(env);
			machinePowerOnIntervals[m.id][i].setOptional();
			machinePowerOnIntervals[m.id][i].setSizeMin(0);
			machinePowerOnIntervals[m.id][i].setSizeMax(288);
		}
		for(int i=0; i<nTasks + 1; i++){
			if(i != 0){
				model.add(IloIfThen(env, IloPresenceOf(env, machinePowerOnIntervals[m.id][i]), IloPresenceOf(env, machinePowerOnIntervals[m.id][i-1])));
				model.add(IloEndBeforeStart(env, machinePowerOnIntervals[m.id][i-1], machinePowerOnIntervals[m.id][i], 1));
			}
			machinePowerOn[m.id] += IloPulse(machinePowerOnIntervals[m.id][i],1);

			// Add the machine idle consumption and power up/down costs to objective function
			energyCosts += IloStartEval(machinePowerOnIntervals[m.id][i], energyPrices, 0)*IloSizeOf(machinePowerOnIntervals[m.id][i])*m.idle_consumption/2;
			energyCosts += IloEndEval(machinePowerOnIntervals[m.id][i], energyPrices, 0)*IloSizeOf(machinePowerOnIntervals[m.id][i])*m.idle_consumption/2;
			energyCosts += IloPresenceOf(env, machinePowerOnIntervals[m.id][i])*(m.power_up_cost + m.power_down_cost);
		}
		for(Task t: instance.tasks){
			model.add(IloAlwaysIn(env, machinePowerOn[m.id], machineHasTask[m.id][t.id], 1, 1));
		}
	}


  // Add objective function to model
	IloObjective objective = IloMinimize(env, energyCosts);
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

