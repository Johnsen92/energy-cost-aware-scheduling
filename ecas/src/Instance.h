#ifndef __INSTANCE__H__
#define __INSTANCE__H__

#include "Machine.h"
#include "Task.h"
#include "Tools.h"
#include <fstream>

using namespace std;

class Instance {
  public:
    int time_resolution;
    int resources;
    vector<Machine> machines;
    vector<Task> tasks;
    vector<float> energy_prices;

    Instance();
    Instance(string filename);
};

#endif // __INSTANCE__H__
