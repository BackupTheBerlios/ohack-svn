#ifndef OBRUTEFORCE_H
#define OBRUTEFORCE_H
#include<ui_OBruteForceGui.h>
#include<OMachinesDialog.h>
#include<OUsersDialog.h>
#include<QtGui>
#include<QtCore>
#include<OAboutDialog.h>
class OMachinesDialog;

class OBruteForceGui:public QMainWindow,Ui::OBruteForceGui
{

	Q_OBJECT
public:
OBruteForceGui(QWidget *parent=0);
~OBruteForceGui();
	public slots:
		/** ********************/
		/**Primer tab "General"*/
		/** ********************/
		/** Lamadas a los dialogos*/
		void OCallMachinesDialog();
		void OCallUsersDialog();
		void OCallAboutDialog();
		
		/** Lectura de usuarios y maquina y dictionarios*/
		void OMachinesButtonState(int State);
		void OMachinesState(int State);
		void OUsersButtonState(int State);
		void OLoadDictionary();
		void ODictionaryComboBoxState(int State);
		
		/** Ejecucion de procesos*/
		void ORunBruteForce();
		
		/** ***********************/
		/**Segundo Tab "Protocols"*/
		/** ***********************/
		
		/** Database Groupbox events*/
		void OMySqlCheckBoxState(int State);
		void OPostGresSqlCheckBoxState(int State);
		void OMsSqlCheckBoxState(int State);
		
		/** Repositories Groupbox events*/
		void OCvsCheckBoxState(int State);
		void OSvnCheckBoxState(int State);
		
		/** EMail Groupbox Eevents*/
		void OPop3CheckBoxState(int State);
		void OSnmpCheckBoxState(int State);
		void OImapCheckBoxState(int State);
		
		/** ORemoteConnectionGroupBox events*/
		void OSshCheckBoxState(int State);
		void OTelnetCheckBoxState(int State);
		void ORshCheckBoxState(int State);
		void ORloginCheckBoxState(int State);
		void OVncCheckBoxState(int State);
		void OFtpCheckBoxState(int State);
	
	private:
	/** Procesos*/
	QProcess *OMedusa;
	QByteArray OStandardOutput;
	
	/** Argumentosd del proceso*/
	QString OUser;
	QString OMachine;
	QString ODictionary;
	QStringList *OArguments;
	QStringList OProtocolArguments;
	QStandardItemModel *OUserSIM;/** SIM => Standard Item Model*/
	QStandardItemModel *OMachineSIM;
	QList<QStandardItem *> *UsersList;/** lista de usuarios Obtenidos de l modelo*/
	QList<QStandardItem *> *MachinesList;/** Lista de maquinas Obtenida del modelo*/
	
	/** Dialogos*/
	OMachinesDialog *MachinesDialog;
	OUsersDialog *UsersDialog;
	OAboutDialog *OAbout;
	QString OHackPath;
};

#endif
 
