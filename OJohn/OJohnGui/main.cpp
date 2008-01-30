#include<OJohnGui.h>
#include<QtCore>

#include<OJohn.h>



int main(int argc,char *argv[])
{
	QString OShadowFile;
	QApplication app(argc,argv);
	QString OSplash=getenv("OHACK");
		if(OSplash.count()==0)
		{
		qDebug()<<"No tienes la varable de entor OHACK"<<endl;
		qDebug()<<"export OHACK=/path/to/OHackSuite/"<<endl;
		qDebug()<<"sin esta variable el programa no funcionara"<<endl;
		exit(1);
		}
		OSplash+="OImages/OJohn/OJohnSplash.png";

	/**Inicializacion del splash*/	
	QPixmap o(OSplash);
	QSplashScreen osplash(o);
	OJohnGui OGui;
	osplash.show();
	
	app.processEvents();
	osplash.show();
	/**
	*************************/
// sleep(2);
osplash.show();
// sleep(2);
OGui.show();
//OGui.OInitTerminal();

osplash.finish(&OGui);
return app.exec();
}
