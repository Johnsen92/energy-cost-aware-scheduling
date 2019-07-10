#ifndef __TOOLS__CPP__
#define __TOOLS__CPP__

#include "Tools.h"

string Tools::indicesToString( string prefix, int i, int j, int v )
{
	stringstream ss;
	ss << prefix << "(" << i;
	if( j >= 0 ) ss << ',' << j;
	if( v >= 0 ) ss << ',' << v;
	ss << ')';
	return ss.str();
}

double Tools::CPUtime()
{
	tms t;
	times( &t );
	double ct = sysconf( _SC_CLK_TCK );
	return t.tms_utime / ct;
}

int Tools::getIntFromLine(string line){
  regex r("[0-9]+");
  smatch m;
  regex_search(line, m, r);
  if(m.size() == 0){
    cout << "Error: no integer found in line" << endl;
    return 0;
  }
  return stoi(m[0]);
}

float Tools::getFloatFromLine(string line){
  regex r("[0-9]+\\.[0-9]+");
  smatch m;
  regex_search(line, m, r);
  if(m.size() == 0){
    cout << "Error: no float found in line" << endl;
    return 0;
  }
  return stof(m[0]);
}

#endif // __TOOLS__CPP__
