#include<cstdlib>
#include<fstream>
#include<string>
#include<GJohnFileConfig.h>


using std::ifstream;
using std::ofstream;
using std::string;

namespace Gerar
{

void ChWSpaces(ifstream& inf,ofstream& outf)
		//Checks if there's  a space character count how many and put'em in outf
{
  char next;
  int spacecount=0;

  inf.get(next);
  while(next==' ')
  {
  inf.get(next);
  spacecount++;
  }
  if(! isspace(next)) inf.putback(next); //if the char is diferent form space or newline
  for(int i=0;i<spacecount;i++) outf<<" ";//writes the spaces
  if(next=='\n'||next=='\t') outf<<next;//after spaces have finished and is \n or tab... write ir!
}


/*es la misma funcion solo que editada para john.conf*/
bool GJohnFileConfig(string condition, string neword)
{
string JohnFile=getenv("HOME"),TmpFile=getenv("HOME");
JohnFile += "/.OHack/OJohn/john.conf";
TmpFile += "/.OHack/GOut.tmp";
ifstream infile(JohnFile.c_str());
ofstream ofile(TmpFile.c_str());
if(!infile)
{
return false;
}
if(!ofile)
{
return false;
}
string nextstr;

while(!infile.eof()) //while infile has not reached the end
    {
      ChWSpaces(infile,ofile); //checks the indentation
      infile>>nextstr;
      ofile << nextstr;

      if(nextstr==condition)
	{
	  ChWSpaces(infile,ofile);
	  infile >> nextstr;
	  
	  ofile << nextstr;//writes the next word(=) before the one that is gonna change
	  ChWSpaces(infile,ofile);
	  ofile << neword;//writes the new word
	  infile >> nextstr;//reads to discard
	  ChWSpaces(infile,ofile);
	}

    }

  infile.close();
  ofile.close();

return system("cp ~/.OHack/GOut.tmp ~/.OHack/OJohn/john.conf;rm ~/.OHack/GOut.tmp");//BUG HERE!!!


}

}
