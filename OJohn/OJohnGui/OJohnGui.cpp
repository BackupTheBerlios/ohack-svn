/**Mis Cabeceras*/
#include<OJohnGui.h>
/**Cabeceras de qt4*/
#include<QtCore> 
#include<string>
#include<kcpuproc.h>
extern "C"
{
#include<stdlib.h>
}

/**Constructor con rencia*/
OJohnGui::OJohnGui(QWidget *parent):QMainWindow(parent)
{
/**Iicializacion del "User Interface"*/
setupUi(this);

/**Inicializacion de las propieades de los elementos de la User Interface*/
/**Panel de Mensages log de modo solo lectura mostrando todo lo que pasa en el programa*/
OLogPanelTextEdit->setReadOnly(true);
//OLoadFilesButton->setDisabled(true);
//ORunPushButton->setDisabled(true);
OLogPanelTextEdit->append(tr("Carga o Crea Una Sesion"));


/**Inicializacion de variables de la clase*/
ODialogFiles=new OLoadFilesDialog(this);
ODialogDictionary=new OLoadDictionaryDialog(this);
OSessionName=new OSessionNameDialog(this);
OAbout=new OAboutDialog(this);
OJohn=new  QProcess(parent);/**Inicializacion del proceso de ejecucion del john*/
OJohn->setProcessChannelMode(QProcess::MergedChannels);
OSession="john";/**Sesion por defecto*/
ORestore=false;

/** ****************************************/
/** Algunas cosas no implementadas or ahora*/
OExternalCheckBox->hide();
OExternalModeComboBox->hide();




/** ****************************************/
/**Mirar La existencia de la configuracion*/
QString TmpDir=getenv("HOME");
	TmpDir.append("/.OHack/OJohn/");
/*mira la de la terminal*/
QDir HomeDir(TmpDir);
//if(!HomeDir.exists())
//{
	this->OInitConfig();
//}


/** *****************************************/



/**Asignacion de eventos A los elementos de la User Interface*/
/**Botones*/
connect(OLoadFilesButton,SIGNAL(clicked()),this,SLOT(OCallLoadFilesDialog()));
connect(ORunPushButton,SIGNAL(clicked()),this,SLOT(ORunCrack()));
connect(OActionQuit,SIGNAL(activated()),this,SLOT(OClose()));
connect(OLoadDictionaryButton,SIGNAL(clicked()),this,SLOT(OCallLoadDictinaryDialog()));
connect(OShowPushButton,SIGNAL(clicked()),this,SLOT(OShowPasswords()));
connect(ODataPushButton,SIGNAL(clicked()),this,SLOT(OShowData()));
connect(OStopPushButton,SIGNAL(clicked()),this,SLOT(OStop()));



/**Opciones basicas o checkbox*/
connect(OSingularCheckBox,SIGNAL(toggled(bool)),this,SLOT(OSingularCheckBoxState(bool)));
connect(OIncrementalCheckBox,SIGNAL(toggled(bool)),this,SLOT(OIncrementalCheckBoxState(bool)));
connect(OExternalCheckBox,SIGNAL(toggled(bool)),this,SLOT(OExternalCheckBoxStates(bool)));

/**acciones del menu*/
connect(OActionNewSession,SIGNAL(activated()),this,SLOT(ONewSession()));
connect(OActionCredits,SIGNAL(activated()),this,SLOT(OCallAbout()));
connect(OActionHelp,SIGNAL(activated()),this,SLOT(OCallHelp()));

/**process*/
connect(OJohn,SIGNAL(finished ( int,QProcess::ExitStatus)),this,SLOT(OJohnEnd(int,QProcess::ExitStatus)));
connect(OJohn,SIGNAL(readyReadStandardOutput()),this,SLOT(OJohnReadStdOut()));


QTimer::singleShot(20, this,SLOT(OCpuStat()));

}
/**Destructor que limpia la memoria de las variables de la clase incializadas en el constructor*/
OJohnGui::~OJohnGui()
{
 delete ODialogFiles;
 delete ODialogDictionary;
 delete OJohn;
 delete OSessionName;
 delete OAbout;
}

/** **************************************************/
/**Seccion de construccion de slots                  */
/** **************************************************/

void OJohnGui::OCpuStat()
{
	readLoad();
	//OCpuUse=all.totalPercent();
	OCpuProgressBar->setValue(all.totalPercent());
	OCpuUserProgressBar->setValue(all.userPercent());
	OCpuSystemProgressBar->setValue(all.systemPercent());
	QTimer::singleShot(200, this,SLOT(OCpuStat()));
}

void OJohnGui::OCallHelp()
{
system("$OHACK/OBin/ODoc OJohnDoc &");
}

void OJohnGui::OCallAbout()
{
	OAbout->exec();
}

void OJohnGui::ONewSession()
{
OSessionName->exec();
OSession=OSessionName->OSessionName.trimmed();
if((OJohn->state()==QProcess::Running)||(OJohn->state()==QProcess::Starting))
{
Message.OShow(tr("En la Seccion Actual \nJohn se esta ejecutando\n detangalo antes de crear una nueva "));
OSessionName->close();
}
else
{
OLoadFilesButton->setEnabled(true);
ORunPushButton->setText(tr("Ejecutar"));
OShadowFile.clear();
OLabelSession->setText(OSession);
}
//OLoadFilesButton->setEnabled(true);
//ORunPushButton->setEnabled(true);

}

/** ****************************/
/**SLOT de el proceso principal*/
/** ****************************/
void OJohnGui::OJohnEnd(int exitCode,QProcess::ExitStatus exitStatus)
{
ORunPushButton->setEnabled(true);
OUserPriorityHorizontalSlider->setEnabled(true);
OLoadDictionaryButton->setEnabled(true);
OSaltsComboBox->setEnabled(true);
OSaveMemoryHorizontalSlider->setEnabled(true);
}

void OJohnGui::OJohnReadStdOut()
{
OStandardOutput=OJohn->readAll();
OLogPanelTextEdit->append(OStandardOutput);
}

/** *************************************************/
/**Construccion de los checkbox de opciones basicas */
/** *************************************************/

/*Mira el Status de OExternalCheckBoxStates  para inhabilitar o no  OsigularCheckbox */
void OJohnGui::OExternalCheckBoxStates(bool State)
{
if(State) 
{
OSingularCheckBox->setDisabled(true);
OIncrementalCheckBox->setDisabled(true);
OIncrementalModeComboBox->setDisabled(true);
OLoadDictionaryButton->setDisabled(true);
}
if(!State)
{
 OSingularCheckBox->setEnabled(true);
 OIncrementalCheckBox->setEnabled(true);
 OIncrementalModeComboBox->setEnabled(true);
 OLoadDictionaryButton->setEnabled(true);
}
}

/*********************************************************************************/
/*Mira el Status de OIncrementalCheckBox para inhabilitar o no  OsigularCheckbox */
/*********************************************************************************/
void OJohnGui::OIncrementalCheckBoxState(bool State)
{
if(State) 
{
OSingularCheckBox->setDisabled(true);
OExternalCheckBox->setDisabled(true);
OExternalModeComboBox->setDisabled(true);
OLoadDictionaryButton->setDisabled(true);
}
if(!State)
{
 OSingularCheckBox->setEnabled(true);
 OExternalCheckBox->setEnabled(true);
 OExternalModeComboBox->setEnabled(true);
 OLoadDictionaryButton->setEnabled(true);
}
}


/** *******************************************************************************/
/**Mira el Status de OsigularCheckbox para inhabilitar o no OIncrementalCheckBox  */
/** *******************************************************************************/
void OJohnGui::OSingularCheckBoxState(bool State)
{
if(State)
{ 
OIncrementalCheckBox->setDisabled(true);
OIncrementalModeComboBox->setDisabled(true);
OExternalCheckBox->setDisabled(true);
OExternalModeComboBox->setDisabled(true);
}
if(!State)
{
OIncrementalCheckBox->setEnabled(true); 
OIncrementalModeComboBox->setEnabled(true);
OExternalCheckBox->setEnabled(true);
OExternalModeComboBox->setEnabled(true);
}
}

/** ***********************************/
/**slot de la barra de menu item salir*/
/** ***********************************/
void OJohnGui::OClose()
{
QVariant Pid(OPid);

/*debug code*/
/*QString KillJohn="kill -9 ";
	KillJohn+=Pid.toString();
	KillJohn+=" > /dev/null ";
std::string KillCommand=KillJohn.toStdString();
system(KillCommand.c_str());*/
OJohn->terminate();
close();
}
/** ***************************************************/
/**Construccion de los Botones que son dialogos       */
/** ***************************************************/

/** **********************************************************/
/**Llama el dialogo para cargar los archivo shadow y password*/
/** **********************************************************/
void OJohnGui::OCallLoadFilesDialog()
{
	ODialogFiles->exec();//La clave es poner exec() y no show para que pase el argumento
	//connect(OGetFileToolButton
	if(ODialogFiles->OLoadFilesDialogStatus)
	{
	OShadowFile=ODialogFiles->OShadowLineEdit->text().trimmed();//trimmed remueve los espacion en blanco
	//connect(ODialogFiles->OToolButton,SIGNAL(clecked()),this,SLOT(OGetSshadow()));
	ODialogFiles->OShadowLineEdit->clear();
	OLoadFilesButton->setDisabled(true);
	}
}

void OJohnGui::OCallLoadDictinaryDialog()
{
/** Cargar lista de diccionarios*/
ODialogDictionary->ODictionaryComboBox->clear();
QString Path=getenv("HOME");
	Path+="/.OHack/ODictionary/ODictionaries";
QDir DicPath(Path);
QStringList  Files;
Files<<"*.dic";
Files=DicPath.entryList(Files);
ODialogDictionary->ODictionaryComboBox->addItems(Files);
ODialogDictionary->exec();
ODictionaryFile=ODialogDictionary->ODictionaryFile.trimmed();/*trimmed borra los espacios en blanco*/


}



/** ****************************************************/
/**construccion de los botones que no son dialogos     */
/** ****************************************************/

void OJohnGui::OStop()
{
	
OLogPanelTextEdit->append(tr("Proceso Esta Detenido !!"));
OJohn->terminate();//etse no funciona para los procesos hijos; el de abajo lo mata por que lo mata
ORunPushButton->setEnabled(true);
OLoadDictionaryButton->setEnabled(true);
OSaltsComboBox->setEnabled(true);
OSaveMemoryHorizontalSlider->setEnabled(true);
/*debug code*/
/*QVariant Pid(OPid);
QString KillJohn="kill -9 ";
	KillJohn+=Pid.toString();
	KillJohn+=" &> /dev/null ";
std::string KillCommand=KillJohn.toStdString();
system(KillCommand.c_str());//este lo mata por que lo mata*/
}

void OJohnGui::OShowData()
{
QString Shadow="Shadow = ",Mode=tr("Modo = "),Nice=tr("Prioridad \"nice\" = ");
QString Crypt=tr("Encriptacion = ");
QVariant  OVar(OUserPriorityHorizontalSlider->value());
OLogPanelTextEdit->append(tr(" Datos Cargados"));	
if(OShadowFile.count()==0)
{
Shadow+=tr("No Cargado !!!");
OLogPanelTextEdit->append(Shadow);
}
else
{
Shadow+=OShadowFile;
OLogPanelTextEdit->append(Shadow);
}
if(OSingularCheckBox->checkState())
{
Mode+=tr("Singular");
OLogPanelTextEdit->append(Mode);
}
else if(OIncrementalCheckBox->checkState())
{
Mode+="Incremental";
OLogPanelTextEdit->append(Mode);
}
else if(OExternalCheckBox->checkState())
{
Mode+=tr("Externo");
OLogPanelTextEdit->append(Mode);
}
else
{
Mode+=tr("No seleccionado aun");
OLogPanelTextEdit->append(Mode);
}
Nice+=OVar.toString();
OLogPanelTextEdit->append(Nice);
Crypt+=OSaltsComboBox->currentText();
OLogPanelTextEdit->append(Crypt);
}

void OJohnGui::OShowPasswords()
{
QProcess *OJohnShow=new  QProcess(this);
OJohnShow->setProcessChannelMode(QProcess::MergedChannels);
//QString Session="--session=";
	//Session+=OSession;
QStringList OArgumentsShow;
if(OShadowFile.count()==0)
{
OLogPanelTextEdit->append(tr("Debes Cargar primero el shadow"));
}
else
{
	OArgumentsShow<<"--show"<<OShadowFile;//<<Session;
	OJohnShow->start(OJohnPath,OArgumentsShow);
	if(!OJohnShow->waitForStarted())
	{
	OLogPanelTextEdit->append(tr("John no esta corriendo no se \npudieron mostrar los passwords  !!!"));
	OStandardOutput=OJohnShow->readAll();
	OLogPanelTextEdit->append(OStandardOutput);
	}
	
	if(OJohnShow->waitForFinished())
	{
	OLogPanelTextEdit->append(tr("Passwors Hallados"));
	OStandardOutput=OJohnShow->readAll();
	OLogPanelTextEdit->append(OStandardOutput);
	}
	
}
delete OJohnShow;
}


/** *****************************/
/**Ejecucucion total del proceso*/
/** *****************************/
	
void OJohnGui::ORunCrack()
{
/** ***************************/
/**ONice Prioridad del sistema*/	
/** ***************************/
	QString ONiceFile=getenv("HOME");
	ONiceFile+="/.OHack/OJohn/nice.conf";
	QFile ONFile(ONiceFile);
	if(!ONFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
	{
	OLogPanelTextEdit->append(tr("No se pudo escribir en el archivo: ")+ONiceFile);
	OLogPanelTextEdit->append(tr("Verifique que tiene permisos de escritura"));
	}
	else
	{
	QTextStream ONiceWrite(&ONFile);
	ONiceWrite<<OUserPriorityHorizontalSlider->value();
	ONiceWrite.atEnd();
	}
	ONFile.atEnd();
/****************************************/
/*Finaliza la parte Del Nice Del Systema*/
/****************************************/

 QTextStream *out=new QTextStream(stdout);
if(OShadowFile.count()==0)
{
//Aqui poner el dialogo de mensage para llenar primero los archivos
OLogPanelTextEdit->append(tr("Debe introducir primero los archivos a Crackear \nEn el boton cargar archivo Ok"));
}
if(OShadowFile.count()!=0)
{
	if(OIncrementalCheckBox->isChecked()||OSingularCheckBox->isChecked())
	{
		
		//  delete out;
	
		//OStandardOutput=new QByteArray;
		// QProcess *OJohn=new  QProcess(0);
	// 	QStringList *OArguments=new QStringList;
		QStringList OArguments;
		//OArguments<<"-hold"<<"-e"<<OJohnPath;
		if(OSingularCheckBox->isChecked())
		{
		QString DicFile="--wordlist=";
			DicFile+=getenv("HOME");
			DicFile+="/.OHack/ODictionary/ODictionaries/";
			DicFile+=ODictionaryFile;
		OArguments<<DicFile;
		}
		if(OIncrementalCheckBox->isChecked())
		{
		QString *TmpArgs=new QString;
		*TmpArgs="-i=";
		TmpArgs->append(OIncrementalModeComboBox->currentText());
		OArguments<<*TmpArgs;
		delete TmpArgs;
		}
		if(OSaltsComboBox->currentText()!=tr("Cualquiera"))
		{
		QString TmpArgs = "--format=";
			TmpArgs+= OSaltsComboBox->currentText();
		OArguments<<TmpArgs;
		}
		//OArguments<<OShadowFile;
		if(!ORestore)
		{
		OLogPanelTextEdit->append(tr("Lanzado .."));
		ORestore=true;
		}
		else
		{
		OLogPanelTextEdit->append(tr("Continuando .."));
		}
		//OJohn->moveToThread(OJohn->thread());/*para correrlo con hilos*/
		
		OJohn->start(OJohnPath,OArguments<<OShadowFile);
		qDebug()<<OArguments;

		/** *****************************************************/
		/** Deasbilitar elementos del wigdet mientras se ejecuta*/
		
		ORunPushButton->setText(tr("Continuar"));
		ORunPushButton->setDisabled(true);
		OUserPriorityHorizontalSlider->setDisabled(true);
		OLoadDictionaryButton->setDisabled(true);
		OSaltsComboBox->setDisabled(true);
		OSaveMemoryHorizontalSlider->setDisabled(true);

		//OPid=OJohn->pid();//obtener el pid
		//*out<<"OPid = "<<OPid;
		//*out<<"OArguments = "<<&OArguments<<endl;
		OJohn->closeWriteChannel();
		if(!OJohn->waitForStarted(30))
		{
		OLogPanelTextEdit->append(tr("John no esta corriendo !!!"));
		OStandardOutput=OJohn->readAll();
		OLogPanelTextEdit->append(OStandardOutput);
		}else
		if(OJohn->waitForFinished(30))
		{
		OLogPanelTextEdit->append(tr("\nMensages del john :"));
		OStandardOutput=OJohn->readAll();
		OLogPanelTextEdit->append(OStandardOutput);
		}
 		else if(OJohn->waitForReadyRead(30))
		{
		OLogPanelTextEdit->append(tr("\nMensages del john :"));
		OStandardOutput=OJohn->readAll();
		OLogPanelTextEdit->append(OStandardOutput);
		}
		//ORunPushButton->setEnabled(true);
	}
	else
	{
	OLogPanelTextEdit->append(tr("Debe elegir  primero na opcion basica  \nSingular , Incremental o Externo"));
	}
}
delete out;
}


/*mirar si existe la configuracion inicial o si no crearla*/
void OJohnGui::OInitConfig()
{

	

system("mkdir -p ~/.OHack;mkdir -p ~/.OHack/OJohn;mkdir -p ~/.OHack/OJohn/OModes;mkdir -p ~/.OHack/OJohn/ODictionary;mkdir -p ~/.OHack/OJohn/OLog;mkdir -p ~/.OHack/OJohn/OSessions;mkdir -p ~/.OHack/ODictionary/ODictionaries");


/*falta mirar la existencia de los archivos*/
/*eso se hace en el principal*/
system("cp -rf $OHACK/OJohn/OJohnCore/OJohnConfig/*.conf ~/.OHack/OJohn/");
system("cp -rf $OHACK/OJohn/OJohnCore/OJohnConfig/*.chr ~/.OHack/OJohn/OModes/");
system("cp -rf $OHACK/OJohn/OJohnCore/OJohnConfig/password.dic ~/.OHack/OJohn/ODictionary/");
system("cp -rf $OHACK/OJohn/OJohnCore/OJohnConfig/*.dic ~/.OHack/ODictionary/ODictionaries/");

}



