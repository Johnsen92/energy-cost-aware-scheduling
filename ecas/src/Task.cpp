#include "Task.h"

// ##########################################################################
// Constructors & Destructors                                             ###
// ##########################################################################

Task::Task() 
  : id(0), earliest_start_time(0), latest_end_time(0), duration(0), power_consumption(0.0), resource_usage() {}

Task::Task(string json) : Task() {
  istringstream ss(json);
  string line;
  while(getline(ss, line)){
    if(regex_match(line, regex("^.*earliest_start_time.*$")))
      this->earliest_start_time = Tools::getIntFromLine(line);
    else if(regex_match(line, regex("^.*id.*$")))
      this->id = Tools::getIntFromLine(line);
    else if(regex_match(line, regex("^.*latest_end_time.*$")))
      this->latest_end_time = Tools::getIntFromLine(line);
    else if(regex_match(line, regex("^.*duration.*$")))
      this->duration = Tools::getIntFromLine(line);
    else if(regex_match(line, regex("^.*power_consumption.*$")))
      this->power_consumption = Tools::getFloatFromLine(line);
    else if(regex_match(line, regex("^.*resource_usage.*$"))){
      while(getline(ss, line)){
        this->resource_usage.push_back(Tools::getIntFromLine(line));
        if(!regex_match(line, regex("^.*,$")))
          break;
      }
    }
  }
  this->name = "Task_" + to_string(this->id);
}

// ##########################################################################
// Methods                                                                ###
// ##########################################################################

int Task::cpu_usage(){
  if(this->resource_usage.size() != 3){
    cout << "Error: resource usages uninitialized" << endl;
    return 0;
  }
  return this->resource_usage[0];
}

int Task::memory_usage(){
  if(this->resource_usage.size() != 3){
    cout << "Error: resource usages uninitialized" << endl;
    return 0;
  }
  return this->resource_usage[1];
}

int Task::io_usage(){
  if(this->resource_usage.size() != 3){
    cout << "Error: resource usages uninitialized" << endl;
    return 0;
  }
  return this->resource_usage[2];
}

// ##########################################################################
// Operators                                                              ###
// ##########################################################################

ostream &operator<<(std::ostream &str, const Task &t) {
  str << "id: " << t.id << endl;
  str << "earliest_start_time: " << t.earliest_start_time << endl;
  str << "latest_end_time: " << t.latest_end_time << endl;
  str << "duration: " << t.duration << endl;
  str << "power_consumption: " << t.power_consumption << endl;
  str << "resource_capacities: " << endl;
  for(int i: t.resource_usage)
    str << "\t" << i << endl;
  return str;
}
