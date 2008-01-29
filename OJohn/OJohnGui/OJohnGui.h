#ifndef OJOHNGUI_H
#define OJOHNGUI_H
#include<ui_OJohnMainWindow.h>
#include<OLoadFilesDialog.h>
#include<OLoadDictionaryDialog.h>
#include<OSessionNameDialog.h>
#include<OMessage.h>
#include<OAboutDialog.h>
#include<QtCore>
#include<kcpuproc.h>
#define OJohnPath "/home/andresete/WorkSpace/OHackSuite/OBin/john"

class OLoadFilesDialog;
class OSessionNameDialog;
class OLoadDictionaryDialog;
class OJohnGui:public QMainWindow,public Ui::OJohnMainWindow,public KCPUProc
{
Q_OBJECT
Q_CLASSINFO("Autor","Omar Andrés Zapata Mesa")
Q_CLASSINFO("URL", "http://ohack.berlios.de")
Q_CLASSINFO("URL", "http://he1.udea.edu.co/ohack")
	public:
	OJohnGui(QWidget *parent=0);
	~OJohnGui();
	//void OInitTerminal();
	
	public slots:
	/**llmada a los dialogos*/
	void OCallLoadFilesDialog();
	void OCallLoadDictinaryDialog();

	/**Botones no dialogos*/
	void ORunCrack();
	void OClose();
	void OStop();
	void OShowPasswords();
	void OShowData();
	
	
	/**checkbox de opciones basicas*/
	void OSingularCheckBoxState(bool State);
	void OIncrementalCheckBoxState(bool State);
	void OExternalCheckBoxStates(bool State);
	
	/**acciones del menu*/
	void ONewSession();
	void OCallAbout();
	void OCallHelp();

	/**Procesos*/
	void OJohnEnd(int exitCode,QProcess::ExitStatus exitStatus);
	void OJohnReadStdOut();
	void OCpuStat();
	private:

	/*Variables Para Generar procesos*/
	QProcess *OJohn;
	QByteArray OStandardOutput;
	int OPid;
	bool ORestore;
	//int OCpuUse;//Uso de la cpu
	//QByteArray OStandardError;
	/*Dialogos*/
	OLoadFilesDialog *ODialogFiles;
	OLoadDictionaryDialog *ODialogDictionary;
	OSessionNameDialog *OSessionName;
	OAboutDialog *OAbout;
	
	/*Variables Obtenidas de los dialogos*/
	QString OSession;
	QString OShadowFile;
	QString OPasswordFile;
	QString ODictionaryFile;
	QString UnshadowFile;
	
	OMessage Message;
	
	void OInitConfig();
// 	signals:
// 	void valueChanged(int Status);
};


#endif






