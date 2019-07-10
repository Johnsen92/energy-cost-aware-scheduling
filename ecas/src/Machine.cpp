#include "Machine.h"

// ##########################################################################
// Constructors & Destructors                                             ###
// ##########################################################################

Machine::Machine() : id(0), idle_consumption(0), power_up_cost(0.0), power_down_cost(0.0), resource_capacities() {}

Machine::Machine(string json) {
  istringstream ss(json);
  string line;
  while(getline(ss, line)){
    if(regex_match(line, regex("^.*idle_consumption.*$")))
      this->idle_consumption = Tools::getIntFromLine(line);
    else if(regex_match(line, regex("^.*id.*$")))
      this->id = Tools::getIntFromLine(line);
    else if(regex_match(line, regex("^.*power_up_cost.*$")))
      this->power_up_cost = Tools::getFloatFromLine(line);
    else if(regex_match(line, regex("^.*power_down_cost.*$")))
      this->power_down_cost = Tools::getFloatFromLine(line);
    else if(regex_match(line, regex("^.*resource_capacities.*$"))){
      while(getline(ss, line)){
        this->resource_capacities.push_back(Tools::getIntFromLine(line));
        if(!regex_match(line, regex("^.*,$")))
          break;
      }
    }
  }
}

// ##########################################################################
// Methods                                                                ###
// ##########################################################################

int Machine::cpu_cap(){
  if(this->resource_capacities.size() != 3){
    cout << "Error: resource capactities uninitialized" << endl;
    return 0;
  }
  return this->resource_capacities[0];
}

int Machine::memory_cap(){
  if(this->resource_capacities.size() != 3){
    cout << "Error: resource capactities uninitialized" << endl;
    return 0;
  }
  return this->resource_capacities[1];
}

int Machine::io_cap(){
  if(this->resource_capacities.size() != 3){
    cout << "Error: resource capactities uninitialized" << endl;
    return 0;
  }
  return this->resource_capacities[2];
}

// ##########################################################################
// Operators                                                              ###
// ##########################################################################

ostream &operator<<(std::ostream &str, const Machine &m) {
  str << "id: " << m.id << endl;
  str << "idle_consumption: " << m.idle_consumption << endl;
  str << "power_up_cost: " << m.power_up_cost << endl;
  str << "power_down_cost: " << m.power_down_cost << endl;
  str << "resource_capacities: " << endl;
  for(int i: m.resource_capacities)
    str << "\t" << i << endl;
  return str;
}