#include "FileReader.h"

using namespace std;

FileReader::FileReader(string path)
{
	ifstream input(path.c_str());
	std::string type;
	std::string name;
	double d;
	int i;
	std::string s;
	if (input){
		while (input.good())
		{
			input >> type >> name;
			InputParametre t;
			if (type == "double"){
				t.type = I_DOUBLE;
				input >> d;
				t.d = d;
				_config[name] = t;
			}
			else if (type == "int"){
				t.type = I_INT;
				input >> i;
				t.i = i;
				_config[name] = t;
			}
			else if (type == "string"){
				t.type = I_STRING;
				input >> s;
				t.s = s;
				_config[name] = t;
			}
			else{
			}
		}
		_good = true;
	}
	else{
		cout << "ERROR: file not found: "<< path << endl;
		_good = false;
	}

	input.close();
}



FileReader::~FileReader()
{
}



int FileReader::count(string s){
	return _config.count(s);
}



inputType FileReader::getType(string s){
	return _config[s].type;
}


double FileReader::getDouble(string s){
	return _config[s].d;
}



int FileReader::getInt(string s){
	return _config[s].i;
}



string FileReader::getString(string s){
	return _config[s].s;
}


bool FileReader::good(){
	return _good;
}