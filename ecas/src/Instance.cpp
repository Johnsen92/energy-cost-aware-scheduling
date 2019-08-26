#include "Instance.h"

// ##########################################################################
// Constructors & Destructors                                             ###
// ##########################################################################

Instance::Instance() 
  : time_resolution(0), resources(0), machines(), tasks(), energy_prices() {}

Instance::Instance(string filename) : Instance() {
  cout << "Parsing instance ... ";
  string json_machine = "";
  string json_task = ""; 
  string line;
  ifstream infile(filename);
  while (getline(infile, line)){
    if(regex_match(line, regex("^.*time_resolution.*$")))
      this->time_resolution = Tools::getIntFromLine(line);
    else if(regex_match(line, regex("^.*resources.*$")))
      this->resources = Tools::getIntFromLine(line);
    else if(regex_match(line, regex("^.*machines.*$"))){
      while(getline(infile, line)){
        json_machine += line + "\n";
        if(regex_match(line, regex("^.*\\},$"))){
          this->machines.push_back(Machine(json_machine));
          json_machine = "";
        }
        else if(regex_match(line, regex("^.*\\}$"))){
          this->machines.push_back(Machine(json_machine));
          break;
        }
      }
    }
    else if(regex_match(line, regex("^.*tasks.*$"))){
      while(getline(infile, line)){
        json_task += line + "\n";
        if(regex_match(line, regex("^.*\\},$"))){
          this->tasks.push_back(Task(json_task));
          json_task = "";
        }
        else if(regex_match(line, regex("^.*\\}$"))){
          this->tasks.push_back(Task(json_task));
          break;
        }
      }
    }
    else if(regex_match(line, regex("^.*energy_prices.*$"))) {
      while(getline(infile, line)){
        this->energy_prices.push_back(Tools::getFloatFromLine(line));
        if(!regex_match(line, regex("^.*,$")))
          break;
      }
    }
  }  
  cout << "done" << endl;
}