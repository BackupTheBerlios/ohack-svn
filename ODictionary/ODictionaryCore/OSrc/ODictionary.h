#ifndef ODICTIONARY_H
#define ODICTIONARY_H
#include<QtCore>

/** Notas*/
/** por defecto ORandDigits utiliza ODigitsType RandInRange*/
#define OneOfOne 1
#define RandInRange 2
class ODictionary
{
	public:
		ODictionary(char *FileName);/**Inicializa las semillas para la parte de palabras aleatorias*/
		~ODictionary();/** para que me borre toda esa chachara*/
		
		/** *************************************/
		/** Generacion Aleatoria de Diccionarios*/
		/** *************************************/	

		/** Digits Metodos*/
		bool ORandDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		void ORandSetType(int Type);

		/** AlphaSmall Capital Amd Special Metodos*/
		bool ORandAlphaSmall(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		bool ORandAlphaSmallAndSpecial(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		bool ORandAlphaSmallAndCapital(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		bool ORandAlphaSmallAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		
		bool ORandAlphaCapital(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		bool ORandAlpheCapitalAndSpecial(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		bool ORandAlphaCapitalAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		
		bool ORandAlphaSpecial(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		bool ORandAlphaSpecialAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		
		bool ORandAlphaSmallAndSpecialAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		bool ORandAlphaSmallAndSpecialAndCapital(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		bool ORandAlphaSmallAndCapitalAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		bool ORandAlphaSpecialAndCapitalAndDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		

		bool ORandAlphaAll(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);

		/** ***************************************/
		/** Generacion Inteligente De diccionarios*/
		/** ***************************************/


		bool OSmartGenerateDigits(unsigned int Lines,unsigned int MinLong,unsigned int MaxLong);
		
		/** *******************************/
		/**Busqueda de repeticiones       */
		/** *******************************/
		


	private:
		QString ODigits[10];
		QString OAlphaSmallLetter[27];
		QString OAlphaCapitalLetter[27];
		QString OAlphaSpecialLetter[126];
		int ORandDigitsType;
	QFile *Dictionary;
	QTextStream *FileOut;
	QTextStream *StdOut;
	QChar *OCharSpecial;

	/** *******************/
	/** Funciones privadas*/
	/** *******************/
	QString OCurlNestle(QString Word,int Counter);
	QString* OInvertList(QString List[]);
	
};

#endif
 
