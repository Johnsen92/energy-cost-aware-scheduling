#ifndef __TASK__H__
#define __TASK__H__

#include "Tools.h"

using namespace::std;

class Task {
  public:
    int id;
    int earliest_start_time;
    int latest_end_time;
    int duration;
    float power_consumption;
    vector<int> resource_usage;

    int cpu_usage();
    int memory_usage();
    int io_usage();

    Task();
    Task(string json);

    friend std::ostream &operator<<(std::ostream &str, const Task &t);
};

#endif // __TASK__H__