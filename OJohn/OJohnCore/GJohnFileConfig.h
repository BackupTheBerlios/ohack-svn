#ifndef GJOHNFILECONFIG_H
#define GJOHNFILECONFIG_H

#include<fstream>
#include<string>
#include<iostream>
using std::ifstream;
using std::ofstream;
using std::string;

namespace Gerar
{
/*esta funcion sabe que archivo cojer por defecto*/
bool GJohnFileConfig(string condition, string neword);
//uses ChWSpaces
//check condition and changes the word two words ahead by neword in file

//void ChWSpaces(ifstream& inf,ofstream& outf);
//checks if there's a space, count how many and write'em in outf
}
#endif

