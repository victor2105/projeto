#include "FileWriter.h"


FileWriter::FileWriter()
{
}


FileWriter::~FileWriter()
{
}

void FileWriter::begin(string path){
	_foutput.open(path.c_str(), ios::app);
}

void FileWriter::end(){
	_foutput << std::endl;
	_foutput.close();
}

void FileWriter::writeInt(int i){
	if (_foutput)
		_foutput << i;
}

void FileWriter::writeString(string s){
	if (_foutput)
		_foutput << s;
}

void FileWriter::space(){
	if (_foutput)
		_foutput << "\t";
}

void FileWriter::endl()
{
	_foutput << std::endl;
}

void FileWriter::writeDouble(double d){
	if (_foutput)
		_foutput << d;
}