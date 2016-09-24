#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

class FileWriter
{
private:
	ofstream _foutput;
public:
	FileWriter();
	~FileWriter();
	void begin(string path);
	void writeInt(int);
	void writeString(string);
	void writeDouble(double);
	void space();
	void endl();

	void end();
};

