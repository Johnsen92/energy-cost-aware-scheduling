#ifndef __MACHINE__H__
#define __MACHINE__H__

#include "Tools.h"

using namespace std;

class Machine {
  public:
    int id;
    int idle_consumption;
    float power_up_cost;
    float power_down_cost;
    vector<int> resource_capacities;

    int cpu_cap();
    int memory_cap();
    int io_cap();

    Machine();
    Machine(string json);

    friend std::ostream &operator<<(std::ostream &str, const Machine &m);
};

#endif // __MACHINE__H__