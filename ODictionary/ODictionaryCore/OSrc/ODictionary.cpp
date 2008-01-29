#include<ODictionary.h>
#include<cstdlib>
#include<iostream>
#include<cstdlib>
#include<ctime>

extern "C"
{
#include<unistd.h>
}

using namespace std;
ODictionary::ODictionary(char *FileName)
{
	/** Digits "parte Numerica"*/
ODigits[0]="0";
ODigits[1]="1";
ODigits[2]="2";
ODigits[3]="3";
ODigits[4]="4";
ODigits[5]="5";
ODigits[6]="6";
ODigits[7]="7";
ODigits[8]="8";
ODigits[9]="9";

/**Small Letter */
OAlphaSmallLetter[0]="a";
OAlphaSmallLetter[1]="b";
OAlphaSmallLetter[2]="c";
OAlphaSmallLetter[3]="d";
OAlphaSmallLetter[4]="e";
OAlphaSmallLetter[5]="f";
OAlphaSmallLetter[6]="g";
OAlphaSmallLetter[7]="h";
OAlphaSmallLetter[8]="i";
OAlphaSmallLetter[9]="j";
OAlphaSmallLetter[10]="k";
OAlphaSmallLetter[11]="l";
OAlphaSmallLetter[12]="m";
OAlphaSmallLetter[13]="n";
OAlphaSmallLetter[14]=OAlphaSmallLetter[14].setUnicode(OCharSpecial=new QChar(0xF1),1);//ñ unicode hex
OAlphaSmallLetter[15]="o";
OAlphaSmallLetter[16]="p";
OAlphaSmallLetter[17]="q";
OAlphaSmallLetter[18]="r";
OAlphaSmallLetter[19]="s";
OAlphaSmallLetter[20]="t";
OAlphaSmallLetter[21]="u";
OAlphaSmallLetter[22]="v";
OAlphaSmallLetter[23]="w";
OAlphaSmallLetter[24]="x";
OAlphaSmallLetter[25]="y";
OAlphaSmallLetter[26]="z";
//OAlphaSmallLetter[27]="";

/** Capital Letter*/
OAlphaCapitalLetter[0]="A";
OAlphaCapitalLetter[1]="B";
OAlphaCapitalLetter[2]="C";
OAlphaCapitalLetter[3]="D";
OAlphaCapitalLetter[4]="E";
OAlphaCapitalLetter[5]="F";
OAlphaCapitalLetter[6]="G";
OAlphaCapitalLetter[7]="H";
OAlphaCapitalLetter[8]="I";
OAlphaCapitalLetter[9]="J";
OAlphaCapitalLetter[10]="K";
OAlphaCapitalLetter[11]="L";
OAlphaCapitalLetter[12]="M";
OAlphaCapitalLetter[13]="N";
OAlphaCapitalLetter[14]=OAlphaCapitalLetter[14].setUnicode(OCharSpecial=new QChar(0xD1),1);//Ñ unicode hex
OAlphaCapitalLetter[15]="O";
OAlphaCapitalLetter[16]="P";
OAlphaCapitalLetter[17]="Q";
OAlphaCapitalLetter[18]="R";
OAlphaCapitalLetter[19]="S";
OAlphaCapitalLetter[20]="T";
OAlphaCapitalLetter[21]="U";
OAlphaCapitalLetter[22]="V";
OAlphaCapitalLetter[23]="W";
OAlphaCapitalLetter[24]="X";
OAlphaCapitalLetter[25]="Y";
OAlphaCapitalLetter[26]="Z";

//OAlphaSpecialLetter[].setUnicode(&QChar(0x),1);
OAlphaSpecialLetter[0]=" ";
OAlphaSpecialLetter[1]="!";
OAlphaSpecialLetter[2]="\"";
OAlphaSpecialLetter[3]="#";
OAlphaSpecialLetter[4]="$";
OAlphaSpecialLetter[5]="%";
OAlphaSpecialLetter[6]="&";
OAlphaSpecialLetter[7]="'";
OAlphaSpecialLetter[8]="(";
OAlphaSpecialLetter[9]=")";
OAlphaSpecialLetter[10]="*";
OAlphaSpecialLetter[11]="+";
OAlphaSpecialLetter[12]=",";
OAlphaSpecialLetter[13]=".";
OAlphaSpecialLetter[14]="/";
OAlphaSpecialLetter[15]=":";
OAlphaSpecialLetter[16]=";";
OAlphaSpecialLetter[17]="<";
OAlphaSpecialLetter[18]="=";
OAlphaSpecialLetter[19]=">";
OAlphaSpecialLetter[20]="?";
OAlphaSpecialLetter[21]="@";
OAlphaSpecialLetter[22]="[";
OAlphaSpecialLetter[23]="\\";
OAlphaSpecialLetter[24]="]";
OAlphaSpecialLetter[25]="^";
OAlphaSpecialLetter[26]="_";
OAlphaSpecialLetter[27]="`";
OAlphaSpecialLetter[28]="{";
OAlphaSpecialLetter[29]="|";
OAlphaSpecialLetter[30]="}";
OAlphaSpecialLetter[31]="~";
/*No standar Tomado de Unicode */
OAlphaSpecialLetter[32]=OAlphaSpecialLetter[32].setUnicode(OCharSpecial=new QChar(0xA0),1);//" "
OAlphaSpecialLetter[33]=OAlphaSpecialLetter[33].setUnicode(OCharSpecial=new QChar(0xA1),1);//¡
OAlphaSpecialLetter[34]=OAlphaSpecialLetter[34].setUnicode(OCharSpecial=new QChar(0xA2),1);//¢
OAlphaSpecialLetter[35]=OAlphaSpecialLetter[35].setUnicode(OCharSpecial=new QChar(0xA3),1);//£
OAlphaSpecialLetter[36]=OAlphaSpecialLetter[36].setUnicode(OCharSpecial=new QChar(0xA4),1);//¤
OAlphaSpecialLetter[37]=OAlphaSpecialLetter[37].setUnicode(OCharSpecial=new QChar(0xA5),1);//¥
OAlphaSpecialLetter[38]=OAlphaSpecialLetter[38].setUnicode(OCharSpecial=new QChar(0xA6),1);//¦
OAlphaSpecialLetter[39]=OAlphaSpecialLetter[39].setUnicode(OCharSpecial=new QChar(0xA7),1);//§
OAlphaSpecialLetter[40]=OAlphaSpecialLetter[40].setUnicode(OCharSpecial=new QChar(0xA8),1);//¨
OAlphaSpecialLetter[41]=OAlphaSpecialLetter[41].setUnicode(OCharSpecial=new QChar(0xA9),1);//©
OAlphaSpecialLetter[42]=OAlphaSpecialLetter[42].setUnicode(OCharSpecial=new QChar(0xAA),1);//ª
OAlphaSpecialLetter[43]=OAlphaSpecialLetter[43].setUnicode(OCharSpecial=new QChar(0xAB),1);//«
OAlphaSpecialLetter[44]=OAlphaSpecialLetter[44].setUnicode(OCharSpecial=new QChar(0xAC),1);//¬
OAlphaSpecialLetter[45]=OAlphaSpecialLetter[45].setUnicode(OCharSpecial=new QChar(0xAD),1);//­
OAlphaSpecialLetter[46]=OAlphaSpecialLetter[46].setUnicode(OCharSpecial=new QChar(0xAE),1);//®
OAlphaSpecialLetter[47]=OAlphaSpecialLetter[47].setUnicode(OCharSpecial=new QChar(0xAF),1);//¯
OAlphaSpecialLetter[48]=OAlphaSpecialLetter[48].setUnicode(OCharSpecial=new QChar(0xB0),1);//°
OAlphaSpecialLetter[49]=OAlphaSpecialLetter[49].setUnicode(OCharSpecial=new QChar(0xB1),1);//±
OAlphaSpecialLetter[50]=OAlphaSpecialLetter[50].setUnicode(OCharSpecial=new QChar(0xB2),1);//²
OAlphaSpecialLetter[51]=OAlphaSpecialLetter[51].setUnicode(OCharSpecial=new QChar(0xB3),1);//³
OAlphaSpecialLetter[52]=OAlphaSpecialLetter[52].setUnicode(OCharSpecial=new QChar(0xB4),1);//´
OAlphaSpecialLetter[53]=OAlphaSpecialLetter[53].setUnicode(OCharSpecial=new QChar(0xB5),1);//µ
OAlphaSpecialLetter[54]=OAlphaSpecialLetter[54].setUnicode(OCharSpecial=new QChar(0xB6),1);//¶
OAlphaSpecialLetter[55]=OAlphaSpecialLetter[55].setUnicode(OCharSpecial=new QChar(0xB7),1);//·
OAlphaSpecialLetter[56]=OAlphaSpecialLetter[56].setUnicode(OCharSpecial=new QChar(0xB8),1);//¸
OAlphaSpecialLetter[57]=OAlphaSpecialLetter[57].setUnicode(OCharSpecial=new QChar(0xB9),1);//¹
OAlphaSpecialLetter[58]=OAlphaSpecialLetter[58].setUnicode(OCharSpecial=new QChar(0xBA),1);//º
OAlphaSpecialLetter[59]=OAlphaSpecialLetter[59].setUnicode(OCharSpecial=new QChar(0xBB),1);//»
OAlphaSpecialLetter[60]=OAlphaSpecialLetter[60].setUnicode(OCharSpecial=new QChar(0xBC),1);//¼
OAlphaSpecialLetter[61]=OAlphaSpecialLetter[61].setUnicode(OCharSpecial=new QChar(0xBD),1);//½
OAlphaSpecialLetter[62]=OAlphaSpecialLetter[62].setUnicode(OCharSpecial=new QChar(0xBE),1);//¾
OAlphaSpecialLetter[63]=OAlphaSpecialLetter[63].setUnicode(OCharSpecial=new QChar(0xBF),1);//¿
OAlphaSpecialLetter[64]=OAlphaSpecialLetter[64].setUnicode(OCharSpecial=new QChar(0xC0),1);//À
OAlphaSpecialLetter[65]=OAlphaSpecialLetter[65].setUnicode(OCharSpecial=new QChar(0xC1),1);//Á
OAlphaSpecialLetter[66]=OAlphaSpecialLetter[66].setUnicode(OCharSpecial=new QChar(0xC2),1);//Â
OAlphaSpecialLetter[67]=OAlphaSpecialLetter[67].setUnicode(OCharSpecial=new QChar(0xC3),1);//Ã
OAlphaSpecialLetter[68]=OAlphaSpecialLetter[68].setUnicode(OCharSpecial=new QChar(0xC4),1);//Ä
OAlphaSpecialLetter[69]=OAlphaSpecialLetter[69].setUnicode(OCharSpecial=new QChar(0xC5),1);//Å
OAlphaSpecialLetter[70]=OAlphaSpecialLetter[70].setUnicode(OCharSpecial=new QChar(0xC6),1);//Æ
OAlphaSpecialLetter[71]=OAlphaSpecialLetter[71].setUnicode(OCharSpecial=new QChar(0xC7),1);//Ç
OAlphaSpecialLetter[72]=OAlphaSpecialLetter[72].setUnicode(OCharSpecial=new QChar(0xC8),1);//È
OAlphaSpecialLetter[73]=OAlphaSpecialLetter[73].setUnicode(OCharSpecial=new QChar(0xC9),1);//É
OAlphaSpecialLetter[74]=OAlphaSpecialLetter[74].setUnicode(OCharSpecial=new QChar(0xCA),1);//Ê
OAlphaSpecialLetter[75]=OAlphaSpecialLetter[75].setUnicode(OCharSpecial=new QChar(0xCB),1);//Ë
OAlphaSpecialLetter[76]=OAlphaSpecialLetter[76].setUnicode(OCharSpecial=new QChar(0xCC),1);//Ì
OAlphaSpecialLetter[77]=OAlphaSpecialLetter[77].setUnicode(OCharSpecial=new QChar(0xCD),1);//Í
OAlphaSpecialLetter[78]=OAlphaSpecialLetter[78].setUnicode(OCharSpecial=new QChar(0xCE),1);//Î
OAlphaSpecialLetter[79]=OAlphaSpecialLetter[79].setUnicode(OCharSpecial=new QChar(0xCF),1);//Ï
OAlphaSpecialLetter[80]=OAlphaSpecialLetter[80].setUnicode(OCharSpecial=new QChar(0xD0),1);//Ð
//Aqui va la Ñ que esta arriba por eso salta de 0xD0 a OxD2
OAlphaSpecialLetter[81]=OAlphaSpecialLetter[81].setUnicode(OCharSpecial=new QChar(0xD2),1);//Ò
OAlphaSpecialLetter[82]=OAlphaSpecialLetter[82].setUnicode(OCharSpecial=new QChar(0xD3),1);//Ó
OAlphaSpecialLetter[83]=OAlphaSpecialLetter[83].setUnicode(OCharSpecial=new QChar(0xD4),1);//Ô
OAlphaSpecialLetter[84]=OAlphaSpecialLetter[84].setUnicode(OCharSpecial=new QChar(0xD5),1);//Õ
OAlphaSpecialLetter[85]=OAlphaSpecialLetter[85].setUnicode(OCharSpecial=new QChar(0xD6),1);//Ö
OAlphaSpecialLetter[86]=OAlphaSpecialLetter[86].setUnicode(OCharSpecial=new QChar(0xD7),1);//×
OAlphaSpecialLetter[87]=OAlphaSpecialLetter[87].setUnicode(OCharSpecial=new QChar(0xD8),1);//Ø
OAlphaSpecialLetter[88]=OAlphaSpecialLetter[88].setUnicode(OCharSpecial=new QChar(0xD9),1);//Ù
OAlphaSpecialLetter[89]=OAlphaSpecialLetter[89].setUnicode(OCharSpecial=new QChar(0xDA),1);//Ú
OAlphaSpecialLetter[90]=OAlphaSpecialLetter[90].setUnicode(OCharSpecial=new QChar(0xDB),1);//Û
OAlphaSpecialLetter[91]=OAlphaSpecialLetter[91].setUnicode(OCharSpecial=new QChar(0xDC),1);//Ü
OAlphaSpecialLetter[92]=OAlphaSpecialLetter[92].setUnicode(OCharSpecial=new QChar(0xDD),1);//Ý
OAlphaSpecialLetter[93]=OAlphaSpecialLetter[93].setUnicode(OCharSpecial=new QChar(0xDE),1);//Þ
OAlphaSpecialLetter[94]=OAlphaSpecialLetter[94].setUnicode(OCharSpecial=new QChar(0xDF),1);//ß
OAlphaSpecialLetter[95]=OAlphaSpecialLetter[95].setUnicode(OCharSpecial=new QChar(0xE0),1);//à
OAlphaSpecialLetter[96]=OAlphaSpecialLetter[96].setUnicode(OCharSpecial=new QChar(0xE1),1);//á
OAlphaSpecialLetter[97]=OAlphaSpecialLetter[97].setUnicode(OCharSpecial=new QChar(0xE2),1);//â
OAlphaSpecialLetter[98]=OAlphaSpecialLetter[98].setUnicode(OCharSpecial=new QChar(0xE3),1);//ã
OAlphaSpecialLetter[99]=OAlphaSpecialLetter[99].setUnicode(OCharSpecial=new QChar(0xE4),1);//ä
OAlphaSpecialLetter[100]=OAlphaSpecialLetter[100].setUnicode(OCharSpecial=new QChar(0xE5),1);//å
OAlphaSpecialLetter[101]=OAlphaSpecialLetter[101].setUnicode(OCharSpecial=new QChar(0xE6),1);//æ
OAlphaSpecialLetter[102]=OAlphaSpecialLetter[102].setUnicode(OCharSpecial=new QChar(0xE7),1);//ç
OAlphaSpecialLetter[103]=OAlphaSpecialLetter[103].setUnicode(OCharSpecial=new QChar(0xE8),1);//è
OAlphaSpecialLetter[104]=OAlphaSpecialLetter[104].setUnicode(OCharSpecial=new QChar(0xE9),1);//é
OAlphaSpecialLetter[105]=OAlphaSpecialLetter[105].setUnicode(OCharSpecial=new QChar(0xEA),1);//ê
OAlphaSpecialLetter[106]=OAlphaSpecialLetter[106].setUnicode(OCharSpecial=new QChar(0xEB),1);//ë
OAlphaSpecialLetter[107]=OAlphaSpecialLetter[107].setUnicode(OCharSpecial=new QChar(0xEC),1);//ì
OAlphaSpecialLetter[108]=OAlphaSpecialLetter[108].setUnicode(OCharSpecial=new QChar(0xED),1);//í
OAlphaSpecialLetter[109]=OAlphaSpecialLetter[109].setUnicode(OCharSpecial=new QChar(0xEE),1);//î
OAlphaSpecialLetter[110]=OAlphaSpecialLetter[110].setUnicode(OCharSpecial=new QChar(0xEF),1);//ï
OAlphaSpecialLetter[111]=OAlphaSpecialLetter[111].setUnicode(OCharSpecial=new QChar(0xF0),1);//ð
//aqui va la ñ minuscula que esta arriba
OAlphaSpecialLetter[112]=OAlphaSpecialLetter[112].setUnicode(OCharSpecial=new QChar(0xF2),1);//ò
OAlphaSpecialLetter[113]=OAlphaSpecialLetter[113].setUnicode(OCharSpecial=new QChar(0xF3),1);//ó
OAlphaSpecialLetter[114]=OAlphaSpecialLetter[114].setUnicode(OCharSpecial=new QChar(0xF4),1);//ô
OAlphaSpecialLetter[115]=OAlphaSpecialLetter[115].setUnicode(OCharSpecial=new QChar(0xF5),1);//õ
OAlphaSpecialLetter[116]=OAlphaSpecialLetter[116].setUnicode(OCharSpecial=new QChar(0xF6),1);//ö
OAlphaSpecialLetter[117]=OAlphaSpecialLetter[117].setUnicode(OCharSpecial=new QChar(0xF7),1);//÷
OAlphaSpecialLetter[118]=OAlphaSpecialLetter[118].setUnicode(OCharSpecial=new QChar(0xF8),1);//ø
OAlphaSpecialLetter[119]=OAlphaSpecialLetter[119].setUnicode(OCharSpecial=new QChar(0xF9),1);//ù
OAlphaSpecialLetter[120]=OAlphaSpecialLetter[120].setUnicode(OCharSpecial=new QChar(0xFA),1);//ú
OAlphaSpecialLetter[121]=OAlphaSpecialLetter[121].setUnicode(OCharSpecial=new QChar(0xFB),1);//û
OAlphaSpecialLetter[122]=OAlphaSpecialLetter[122].setUnicode(OCharSpecial=new QChar(0xFC),1);//ü
OAlphaSpecialLetter[123]=OAlphaSpecialLetter[123].setUnicode(OCharSpecial=new QChar(0xFD),1);//ý
OAlphaSpecialLetter[124]=OAlphaSpecialLetter[124].setUnicode(OCharSpecial=new QChar(0xFE),1);//þ
OAlphaSpecialLetter[125]=OAlphaSpecialLetter[125].setUnicode(OCharSpecial=new QChar(0xFF),1);//ÿ

/** ********************************************/
/** Iicializacio de Otras Variables de la clase*/
/** ********************************************/

Dictionary=new QFile(FileName);
Dictionary->open(QIODevice::WriteOnly | QIODevice::Text);
FileOut=new QTextStream(Dictionary);
FileOut->setCodec("UTF-8");
StdOut=new QTextStream(stdout);
StdOut->setCodec("UTF-8");
ORandDigitsType=RandInRange;

}

ODictionary::~ODictionary()
{
delete OCharSpecial;
delete Dictionary;
delete FileOut;
delete StdOut;
}
/** ******************************/
/** Random Dictionaries functions*/
/** ******************************/

/** pide el numero de lineas a ganarar y el rango de la longitud de la palabra */
bool ODictionary::ORandDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;	
if(MinLong<=MaxLong)
{
        unsigned int LineCounter=0;
	unsigned int LimitCunter=0;
	srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
	/**#if defined(ODEBUG)
	#endif*/
	if(ORandDigitsType==OneOfOne)
	{
	while(LineCounter<Lines)
	{
		/** Mira el limite de longitud maximo de la palabra*/
		if(LimitCunter==(MaxLong-(MinLong-1)))
		{
			LimitCunter=0;
		}
	for(register unsigned int i=0;i<(MinLong+LimitCunter);i++)
	{
		int ORandLatter=rand()%10;
		#if defined(ODEBUG)
		//*StdOut<<""ORandLatter<<endl;
		*StdOut<<ODigits[ORandLatter];
		#endif
		*FileOut<<ODigits[ORandLatter];
	}
	#if defined(ODEBUG)
	*StdOut<<endl;
	#endif
	*FileOut<<endl;
	LineCounter++;
	LimitCunter++;
	}
	}
	if(ORandDigitsType==RandInRange)
	{
		LineCounter=0;
		while(LineCounter<Lines)
		{
			LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
			/** Mira el limite de longitud maximo de la palabra*/
			if(LimitCunter==(MaxLong-(MinLong-1)))
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));
			}
			
			for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
			{
			int ORandLatter=rand()%10;
			#if defined(ODEBUG)
			//*StdOut<<""ORandLatter<<endl;
			*StdOut<<ODigits[ORandLatter];
			#endif
			*FileOut<<ODigits[ORandLatter];
			}
			#if defined(ODEBUG)
			*StdOut<<endl;
			#endif
			*FileOut<<endl;
			LineCounter++;
			LimitCunter++;
		}
	}
	Status=true;
}
else
{
	*StdOut<<"Error MinLong > MaxLong "<<endl;
	Status=false;
}
return Status;
}


void ODictionary::ORandSetType(int Type)
{
	ORandDigitsType=Type;
}





bool ODictionary::ORandAlphaSmall(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{	
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
			/** Mira el limite de longitud maximo de la palabra*/
			if(LimitCunter==(MaxLong-(MinLong-1)))
			{
				LimitCunter=0;
			}
			for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
			{
			int ORandLatter=rand()%27;
			#if defined(ODEBUG)
			//*StdOut<<""ORandLatter<<endl;
			*StdOut<<OAlphaSmallLetter[ORandLatter];
			#endif
			
			*FileOut<<OAlphaSmallLetter[ORandLatter];
			}
			#if defined(ODEBUG)
			*StdOut<<endl;
			#endif
			*FileOut<<endl;
			LineCounter++;
			LimitCunter++;
			}
		
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
				int ORandLatter=rand()%27;
				#if defined(ODEBUG)
				//*StdOut<<""ORandLatter<<endl;
				*StdOut<<OAlphaSmallLetter[ORandLatter];
				#endif
				*FileOut<<OAlphaSmallLetter[ORandLatter];
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
	*StdOut<<"Error MinLong > MaxLong "<<endl;
	Status=false;
	}
return Status;
}


bool ODictionary::ORandAlphaCapital(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{	
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
			/** Mira el limite de longitud maximo de la palabra*/
			if(LimitCunter==(MaxLong-(MinLong-1)))
			{
				LimitCunter=0;
			}
			
			for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
			{
			int ORandLatter=rand()%27;
			#if defined(ODEBUG)
			//*StdOut<<""ORandLatter<<endl;
			*StdOut<<OAlphaCapitalLetter[ORandLatter];
			#endif
			
			*FileOut<<OAlphaCapitalLetter[ORandLatter];
			}
			#if defined(ODEBUG)
			*StdOut<<endl;
			#endif
			*FileOut<<endl;
			
			LineCounter++;
			LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
				int ORandLatter=rand()%27;
				#if defined(ODEBUG)
				//*StdOut<<""ORandLatter<<endl;
				*StdOut<<OAlphaCapitalLetter[ORandLatter];
				#endif
			
				*FileOut<<OAlphaCapitalLetter[ORandLatter];
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
		
				*FileOut<<endl;
		
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
	*StdOut<<"Error MinLong > MaxLong "<<endl;
	Status=false;
	}
return Status;
}

bool ODictionary::ORandAlphaSpecial(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{	
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
			/** Mira el limite de longitud maximo de la palabra*/
			if(LimitCunter==(MaxLong-(MinLong-1)))
			{
				LimitCunter=0;
			}
			
			for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
			{
			int ORandLatter=rand()%126;
			#if defined(ODEBUG)
			//*StdOut<<""ORandLatter<<endl;
			*StdOut<<OAlphaSpecialLetter[ORandLatter];
			#endif
			
			*FileOut<<OAlphaSpecialLetter[ORandLatter];
			}
			#if defined(ODEBUG)
			*StdOut<<endl;
			#endif
		
			*FileOut<<endl;
			
			LineCounter++;
			LimitCunter++;
			}
		
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
				int ORandLatter=rand()%126;
				#if defined(ODEBUG)
				//*StdOut<<""ORandLatter<<endl;
				*StdOut<<OAlphaSpecialLetter[ORandLatter];
				#endif
			
				*FileOut<<OAlphaSpecialLetter[ORandLatter];
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
		
				*FileOut<<endl;
		
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
	*StdOut<<"Error MinLong > MaxLong "<<endl;
	Status=false;
	}
return Status;
}

bool ODictionary::ORandAlphaSmallAndSpecial(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%126;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSpecialLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%126;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSpecialLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}


bool ODictionary::ORandAlphaSmallAndCapital(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%27;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaCapitalLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaCapitalLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}

bool ODictionary::ORandAlphaSmallAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%10;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<ODigits[ORandLatter];
					#endif
					*FileOut<<ODigits[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}

/** CapitalAnd* contruccion*/

bool ODictionary::ORandAlpheCapitalAndSpecial(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%27;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaCapitalLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%126;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSpecialLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaCapitalLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSpecialLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}

bool ODictionary::ORandAlphaCapitalAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%27;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaCapitalLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaCapitalLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}

bool ODictionary::ORandAlphaSpecialAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%126;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSpecialLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%126;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSpecialLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}

/** construcciones con tres tipos de caracteres*/
bool ODictionary::ORandAlphaSmallAndSpecialAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%3;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%126;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSpecialLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}else
					if(ORandAlphaType==1)
					{
					int ORandLatter=rand()%27;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSmallLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%126;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSpecialLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}else
					if(ORandAlphaType==1)
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}

bool ODictionary::ORandAlphaSmallAndSpecialAndCapital(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%3;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%126;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSpecialLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}else
					if(ORandAlphaType==1)
					{
					int ORandLatter=rand()%27;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSmallLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaCapitalLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%126;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSpecialLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}else
					if(ORandAlphaType==1)
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaCapitalLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}

bool ODictionary::ORandAlphaSmallAndCapitalAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%3;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%27;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaCapitalLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}else
					if(ORandAlphaType==1)
					{
					int ORandLatter=rand()%27;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSmallLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaCapitalLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}else
					if(ORandAlphaType==1)
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}

bool ODictionary::ORandAlphaSpecialAndCapitalAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%3;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%27;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaCapitalLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}else if(ORandAlphaType==1)
					{
					int ORandLatter=rand()%126;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSpecialLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%2;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaCapitalLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}else if(ORandAlphaType==1)
					{
						int ORandLatter=rand()%126;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSpecialLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}
					else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}

bool ODictionary::ORandAlphaAll(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
bool Status;
	if(MinLong<=MaxLong)
	{
		unsigned int LineCounter=0;
		unsigned int LimitCunter=0;
		srandom((getuid( ) << 7) ^ (getpid( ) << 4) ^ time(0)^rand());
		/**#if defined(ODEBUG)
		#endif*/
		if(ORandDigitsType==OneOfOne)
		{
			while(LineCounter<Lines)
			{
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=0;
				}
			
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%4;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
					int ORandLatter=rand()%126;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSpecialLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}else
					if(ORandAlphaType==1)
					{
					int ORandLatter=rand()%27;
					#if defined(ODEBUG)
					//*StdOut<<""ORandLatter<<endl;
					*StdOut<<OAlphaSmallLetter[ORandLatter];
					#endif
					*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
					else if(ORandAlphaType==2)
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaCapitalLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					}
				
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
		if(ORandDigitsType==RandInRange)
		{
			LineCounter=0;
			while(LineCounter<Lines)
			{
				LimitCunter=rand()%(MaxLong-(MinLong-1));/**busca numero aleatorio en el rango permitido*/
				/** Mira el limite de longitud maximo de la palabra*/
				if(LimitCunter==(MaxLong-(MinLong-1)))
				{
					LimitCunter=rand()%(MaxLong-(MinLong-1));
				}
				for(unsigned int i=0;i<(MinLong+LimitCunter);i++)
				{
					int ORandAlphaType=rand()%3;
					if(ORandAlphaType==0)/** Si da ccero tira para special siNo para Small*/
					{
						int ORandLatter=rand()%126;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSpecialLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSpecialLetter[ORandLatter];
					}else
					if(ORandAlphaType==1)
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaSmallLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaSmallLetter[ORandLatter];
					}
					else if(ORandAlphaType==2)
					{
						int ORandLatter=rand()%27;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<OAlphaCapitalLetter[ORandLatter];
						#endif
						*FileOut<<OAlphaCapitalLetter[ORandLatter];
					}else
					{
						int ORandLatter=rand()%10;
						#if defined(ODEBUG)
						//*StdOut<<""ORandLatter<<endl;
						*StdOut<<ODigits[ORandLatter];
						#endif
						*FileOut<<ODigits[ORandLatter];
					
					}
				}
				#if defined(ODEBUG)
				*StdOut<<endl;
				#endif
				*FileOut<<endl;
				LineCounter++;
				LimitCunter++;
			}
		}
	Status=true;
	}
	else
	{
		*StdOut<<"Error MinLong > MaxLong "<<endl;
		Status=false;
	}
return Status;
}


QString* ODictionary::OInvertList(QString List[])
{
int Long=List->size(),i=0;
QString Word[Long];
Long=Long-1;/** para que se pare exactamente en la ultima casillad de memoria*/
while(List[i]!=NULL)
{
Word[i]=List[Long];/** Mientres i avanza long retrocede cambiando el orden*/
i++;
Long--;
}
return Word;

}







// QString ODictionary::OCurlNestle(QString Word,int Counter)
// {
// QString String=Word;
// if(Counter==1)
// {
// String=Word;
// }
// for(int i=0;i<Counter;i++)
// {
// 
// String+=OBucleNestle(Word,Counter-1);
// }
// 
// return String;
// }

bool ODictionary::OSmartGenerateDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong)
{
QString List[10];
OInvertList(ODigits);
for(int i=0;i<10;i++)
{
*StdOut<<List[i]<<endl;
}
}






