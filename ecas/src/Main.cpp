#ifndef __MAIN__CPP__
#define __MAIN__CPP__

// ##########################################################################
// Includes                                                               ###
// ##########################################################################

#include <iostream>
#include "Tools.h"
#include "Instance.h"
#include <fstream>
#include "ECAS.h"

using namespace std;

// ##########################################################################
// Prototypes                                                             ###
// ##########################################################################

void parse_args(int argc, char *argv[], string &infile);
void usage(char *argv[]);

// ##########################################################################
// Main                                                                   ###
// ##########################################################################

int main(int argc, char *argv[]) {
  // parse arguments
  string infile;
  parse_args(argc, argv, infile);

  // create problem instance from JSON file
  Instance i(infile);

  // create CP Optimizer model
  ECAS *model = new ECAS(i);

  // solve CP Optimizer model
  model->solveModel();
	
  // delete CP Optimizer model
  delete model;
	return 0;
} // main

// ##########################################################################
// Functions                                                              ###
// ##########################################################################

void parse_args(int argc, char *argv[], string &infile){
  if(argc != 2)
    usage(argv);
  infile = string(argv[1]);
}

void usage(char *argv[]) {
	cout << "USAGE:\t\t" << argv[0] << " <instance_file>\n";
	cout << "EXAMPLE:\t" << argv[0] << " ./data/sample01.json\n\n";
	exit( 1 );
} // usage

#endif // __MAIN__CPP__
