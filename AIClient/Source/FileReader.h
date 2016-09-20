#pragma once
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

enum inputType { I_INT, I_DOUBLE, I_STRING };

struct InputParametre {
	inputType type;
	double d;
	int i;
	string s;
};


class FileReader
{
private:
	map<string, InputParametre> _config;
	bool _good;
public:
	FileReader(string path = "config.ini");
	int count(string);
	double getDouble(string);
	int getInt(string);
	string getString(string);
	inputType getType(string);
	~FileReader();
	bool good();
	
};

