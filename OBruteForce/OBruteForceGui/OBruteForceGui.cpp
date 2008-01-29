#include<OBruteForceGui.h>
extern "C"
{
#include<stdlib.h>
}

OBruteForceGui::OBruteForceGui(QWidget *parent):QMainWindow(parent)
{
setupUi(this);
/** Inicializacion de Dialogos*/
MachinesDialog=new OMachinesDialog(parent);
UsersDialog=new OUsersDialog(parent);
OAbout=new OAboutDialog(parent);
UsersList=new QList<QStandardItem *>;
MachinesList=new QList<QStandardItem *>;
OHackPath=getenv("OHACK");

/** Inicializacion de variables de procesos*/
OMedusa=new QProcess(parent);
OMedusa->setProcessChannelMode(QProcess::MergedChannels);


/** Dialogs events*/

connect(OMachinesPushButton,SIGNAL(clicked()),this,SLOT(OCallMachinesDialog()));
connect(OUsersPushButton,SIGNAL(clicked()),this,SLOT(OCallUsersDialog()));
connect(OMachinesCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OMachinesButtonState(int)));
connect(OUsersCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OUsersButtonState(int)));
connect(ODictionaryToolButton,SIGNAL(clicked()),this,SLOT(OLoadDictionary()));
connect(ODictionaryCheckBox,SIGNAL(stateChanged(int)),this,SLOT(ODictionaryComboBoxState(int)));
connect(OMachinesNameCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OMachinesState(int)));
connect(OActionAbout,SIGNAL(activated()),this,SLOT(OCallAboutDialog()));

/**Database GruopBox Events*/
connect(OMySqlCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OMySqlCheckBoxState(int)));
connect(OPostGresSqlCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OPostGresSqlCheckBoxState(int)));
connect(OMsSqlCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OMsSqlCheckBoxState(int)));

/**Repositories Events*/
connect(OCvsCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OCvsCheckBoxState(int)));
connect(OSvnCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OSvnCheckBoxState(int)));

/**EMail Events */
connect(OPop3CheckBox,SIGNAL(stateChanged(int)),this,SLOT(OPop3CheckBoxState(int)));
connect(OSnmpCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OSnmpCheckBoxState(int)));
connect(OImapCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OImapCheckBoxState(int)));

/**RemoteConnection Events*/
connect(OSshCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OSshCheckBoxState(int)));
connect(OTelnetCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OTelnetCheckBoxState(int)));
connect(ORshCheckBox,SIGNAL(stateChanged(int)),this,SLOT(ORshCheckBoxState(int)));
connect(ORloginCheckBox,SIGNAL(stateChanged(int)),this,SLOT(ORloginCheckBoxState(int)));
connect(OVncCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OVncCheckBoxState(int)));
connect(OFtpCheckBox,SIGNAL(stateChanged(int)),this,SLOT(OFtpCheckBoxState(int)));

/** Process Events*/
connect(ORunPushButton,SIGNAL(clicked()),this,SLOT(ORunBruteForce()));
}

OBruteForceGui::~OBruteForceGui()
{
	delete MachinesDialog;
	delete UsersDialog;
	delete OAbout;
	delete UsersList;
	delete MachinesList;
}


/** ****************/
/** Process events */
/** ****************/

void OBruteForceGui::ORunBruteForce()
{

OArguments=new QStringList();
/** *********************************************************/
/** Manejo de errores del Tag General de la interfaz grafica*/
/** *********************************************************/


bool UsersStatus=false;
bool MachinesStatus=false;
bool DictionaryStatus=false;
OLogPanelTextEdit->append(tr("----------------------------OHack Log----------------------------------"));
QString MedusaPath=OHackPath;
	MedusaPath+="OBin/medusa";
QStringList OArguments;
/** texto de los lineedit*/
OUser=OUsersLineEdit->text();
ODictionary=ODictionaryLineEdit->text();
OMachine=OMachinesLineEdit->text();

/** ***********************************/
/** Manejo de los argumentos y errores*/
/** ***********************************/

/** Parte de ingresar usuario*/

if((OUser.count()==0)&&(!OUsersCheckBox->isChecked()))/** mira si tiene un usuario en el lineedit o si*/
{						      /**el button de multiples usuarios esta activado*/
OLogPanelTextEdit->append(tr("Ingrese Un Usuario"));
UsersStatus=false;
}else if(OUsersCheckBox->isChecked())
	{
	OUserSIM=UsersDialog->UsersModel;     /** OUsersSIM => SIM => Standard Item Model */
	*UsersList=OUserSIM->takeColumn(0);
	UsersDialog->UsersModel->clear();     /** Limpia my abajo reasigna la lista ya que takeColumn la borra*/
	UsersDialog->UsersModel->insertColumn(0,*UsersList);
	if(UsersList->isEmpty())
		{
		OLogPanelTextEdit->append(tr("Ingrese Usuarios en el Dialogo"));
		UsersStatus=false;
		}
	else
	{
	UsersStatus=true;
	}
	}

if((OUser.count()!=0)&&(!OUsersCheckBox->isChecked()))
{
UsersStatus=true;
}

/** Parte de ingresar Diccionario(s)*/

if((ODictionary.count()==0)&&(!ODictionaryCheckBox->isChecked()))/**mira si tiene un diccionario en el lineedit o si tiene el checkbox*/
{							       /**selecionado para  escogerlo de los predeterminados	            */ 
OLogPanelTextEdit->append(tr("Ingrese Un Diccionario"));
DictionaryStatus=false;
}else if(ODictionaryCheckBox->isChecked())
	{
	DictionaryStatus=true;
	}

if((ODictionary.count()!=0)&&(!ODictionaryCheckBox->isChecked()))
{
DictionaryStatus=true;
}

/** Parte de ingresar Maquina(s)*/
if(!OMachinesNameCheckBox->isChecked()&&!OMachinesCheckBox->isChecked())
{
OLogPanelTextEdit->append(tr("Utilizando Ip"));
MachinesStatus=true;
}

if(OMachinesNameCheckBox->isChecked()&&(OMachine.count()==0))/**Activa la Opcion de introducir manualmente la maquina */
{							   /** y mira si el lineedit tiene un nombre o esta vacio*/
OLogPanelTextEdit->append(tr("Ingrese Una Maquina"));
MachinesStatus=false;
}else if(OMachinesCheckBox->isChecked())
	{
	OMachineSIM=MachinesDialog->MachinesModel;
	*MachinesList=OMachineSIM->takeColumn(0);
	MachinesDialog->MachinesModel->clear();
	MachinesDialog->MachinesModel->insertColumn(0,*MachinesList);
	if(MachinesList->isEmpty())
	{
	OLogPanelTextEdit->append(tr("Ingrese Maquinas en el Dialogo"));
	MachinesStatus=false;
	}else
	{
	MachinesStatus=true;
	//OLogPanelTextEdit->append(tr("falata verificacion del protocolo"));
	}
		
	}
if(OMachinesNameCheckBox->isChecked()&&(OMachine.count()!=0))
{
MachinesStatus=true;
}
/** ***********************************************************/
/** Menjo de errores del tab Protocolos de la interfaz grafica*/
/** ***********************************************************/
OProtocolArguments<<"-M";
bool ProtocolStatus=false;
/** ***************/
/** bases de datos*/
/** ***************/
if(OMySqlCheckBox->isChecked())
{
OProtocolArguments<<"mysql";
ProtocolStatus=true;
}else if(OPostGresSqlCheckBox->isChecked())
{
OProtocolArguments<<"postgres";
ProtocolStatus=true;
}else if(OMsSqlCheckBox->isChecked())
{
OProtocolArguments<<"mssql";
ProtocolStatus=true;
}

/** ******/
/** Email*/
/** ******/
if(OPop3CheckBox->isChecked())
{
OProtocolArguments<<"pop3";
ProtocolStatus=true;
}else if(OSnmpCheckBox->isChecked())
{
OProtocolArguments<<"snmp";
ProtocolStatus=true;
}else if(OImapCheckBox->isChecked())
{
OProtocolArguments<<"imap";
ProtocolStatus=true;
}

/** ********************/
/** Conecciones Remotas*/
/** ********************/
if(OSshCheckBox->isChecked())
{
OProtocolArguments<<"ssh";
ProtocolStatus=true;
}else if(OTelnetCheckBox->isChecked())
{
OProtocolArguments<<"telnet";
ProtocolStatus=true;
}else if(ORshCheckBox->isChecked())
{
OProtocolArguments<<"rsh";
ProtocolStatus=true;
}else if(ORloginCheckBox->isChecked())
{
OProtocolArguments<<"rlogin";
ProtocolStatus=true;
}else if(OVncCheckBox->isChecked())
{
OProtocolArguments<<"vnc";
ProtocolStatus=true;
}else if(OFtpCheckBox->isChecked())
{
OProtocolArguments<<"ftp";
ProtocolStatus=true;
}

/** *************/
/** Repositorios*/
/** *************/

if(OCvsCheckBox->isChecked())
{
OProtocolArguments<<"cvs";
ProtocolStatus=true;
}else if(OSvnCheckBox->isChecked())
{
OProtocolArguments<<"svn";
ProtocolStatus=true;
}

if(ProtocolStatus==false)
{
OLogPanelTextEdit->append(tr("Selecciona un protocolo"));
}

/** Todavia falata meter los argumetos de los protoolos*/

//qDebug()<<UsersStatus<<MachinesStatus<<DictionaryStatus;
if(UsersStatus&&MachinesStatus&&DictionaryStatus&&ProtocolStatus)/** mira que los argumentos esten completos*/
{	
	/** ***********************/
	/**Inicio de la Ejecucion */
	/** ***********************/
	qDebug()<<UsersStatus<<MachinesStatus<<DictionaryStatus;
	OArguments<<"-u"<<OUser<<"-h"<<OMachine<<"-P"<<ODictionary<<"-M"<<"ftp"<<"-O"<<"OLog";
	OMedusa->start(MedusaPath,OArguments);
	OMedusa->closeWriteChannel();
	qDebug()<<OArguments;
	if(!OMedusa->waitForStarted())
	{
		OStandardOutput=OMedusa->readAll();
		OLogPanelTextEdit->append(OStandardOutput);
	}
	
	if(OMedusa->waitForFinished())
	{
		OStandardOutput=OMedusa->readAll();
		OLogPanelTextEdit->append(OStandardOutput);
	}else
	{
	OStandardOutput=OMedusa->readAll();
	OLogPanelTextEdit->append(OStandardOutput);
	}
}
OLogPanelTextEdit->append(tr("-----------------------------------------------------------------------"));
}

/** ***************************************/
/** ORemoteConnectionGroupBox Events SLOTS*/
/** ***************************************/

void OBruteForceGui::OSshCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		//OEmailGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		OEmailGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OTelnetCheckBox->setDisabled(true);
		ORshCheckBox->setDisabled(true);
		ORloginCheckBox->setDisabled(true);
		OVncCheckBox->setDisabled(true);
		OFtpCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		//ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		OTelnetCheckBox->setEnabled(true);
		ORshCheckBox->setEnabled(true);
		ORloginCheckBox->setEnabled(true);
		OVncCheckBox->setEnabled(true);
		OFtpCheckBox->setEnabled(true);
	}
}

void OBruteForceGui::OFtpCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		//OEmailGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		OEmailGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OTelnetCheckBox->setDisabled(true);
		ORshCheckBox->setDisabled(true);
		ORloginCheckBox->setDisabled(true);
		OVncCheckBox->setDisabled(true);
		OSshCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		//ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		OTelnetCheckBox->setEnabled(true);
		ORshCheckBox->setEnabled(true);
		ORloginCheckBox->setEnabled(true);
		OVncCheckBox->setEnabled(true);
		OSshCheckBox->setEnabled(true);
	}
}
		
void OBruteForceGui::OTelnetCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		//OEmailGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		OEmailGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OSshCheckBox->setDisabled(true);
		ORshCheckBox->setDisabled(true);
		ORloginCheckBox->setDisabled(true);
		OVncCheckBox->setDisabled(true);
		OFtpCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		//ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		OSshCheckBox->setEnabled(true);
		ORshCheckBox->setEnabled(true);
		ORloginCheckBox->setEnabled(true);
		OVncCheckBox->setEnabled(true);
		OFtpCheckBox->setEnabled(true);
	}
}
	
void OBruteForceGui::ORshCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		//OEmailGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		OEmailGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OSshCheckBox->setDisabled(true);
		OTelnetCheckBox->setDisabled(true);
		ORloginCheckBox->setDisabled(true);
		OVncCheckBox->setDisabled(true);
		OFtpCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		//ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		OSshCheckBox->setEnabled(true);
		OTelnetCheckBox->setEnabled(true);
		ORloginCheckBox->setEnabled(true);
		OVncCheckBox->setEnabled(true);
		OFtpCheckBox->setEnabled(true);
	}	
}
	
	
void OBruteForceGui::ORloginCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		//OEmailGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		OEmailGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OSshCheckBox->setDisabled(true);
		OTelnetCheckBox->setDisabled(true);
		ORshCheckBox->setDisabled(true);
		OVncCheckBox->setDisabled(true);
		OFtpCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		//ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		OSshCheckBox->setEnabled(true);
		OTelnetCheckBox->setEnabled(true);
		ORshCheckBox->setEnabled(true);
		OVncCheckBox->setEnabled(true);
		OFtpCheckBox->setEnabled(true);
	}
}
	
void OBruteForceGui::OVncCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		//OEmailGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		OEmailGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OSshCheckBox->setDisabled(true);
		OTelnetCheckBox->setDisabled(true);
		ORshCheckBox->setDisabled(true);
		ORloginCheckBox->setDisabled(true);
		OFtpCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		//ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		OSshCheckBox->setEnabled(true);
		OTelnetCheckBox->setEnabled(true);
		ORshCheckBox->setEnabled(true);
		ORloginCheckBox->setEnabled(true);
		OFtpCheckBox->setEnabled(true);
	}
}
	
	

/** ****************************/
/** Email GrupoBox Events SLOTS*/
/** ****************************/

void OBruteForceGui::OImapCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		//OEmailGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		ORemoteConnectionGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OPop3CheckBox->setDisabled(true);
		OSnmpCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		//OEmailGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		OPop3CheckBox->setEnabled(true);
		OSnmpCheckBox->setEnabled(true);
	}
}

void OBruteForceGui::OSnmpCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		//OEmailGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		ORemoteConnectionGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OPop3CheckBox->setDisabled(true);
		OImapCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		//OEmailGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		OPop3CheckBox->setEnabled(true);
		OImapCheckBox->setEnabled(true);
	}
}

void OBruteForceGui::OPop3CheckBoxState(int State)
{

	if(State==Qt::Checked)
	{
		//OEmailGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		ORemoteConnectionGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OSnmpCheckBox->setDisabled(true);
		OImapCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		//OEmailGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		OSnmpCheckBox->setEnabled(true);
		OImapCheckBox->setEnabled(true);
	}
}

/** ***********************************/
/** Repositoies GruopBox events "SLOT"*/
/** ***********************************/

void OBruteForceGui::OSvnCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		OEmailGroupBox->setDisabled(true);
		ORemoteConnectionGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OCvsCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		
		OCvsCheckBox->setEnabled(true);
	}
}

void OBruteForceGui::OCvsCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		OEmailGroupBox->setDisabled(true);
		ORemoteConnectionGroupBox->setDisabled(true);
		ODataBaseGroupBox->setDisabled(true);
		
		OSvnCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORemoteConnectionGroupBox->setEnabled(true);
		ODataBaseGroupBox->setEnabled(true);
		
		
		OSvnCheckBox->setEnabled(true);
	}
}

/** *********************************/
/** Database GruopBox events "SLOTS"*/
/** *********************************/

void OBruteForceGui::OMsSqlCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		OEmailGroupBox->setDisabled(true);
		ORemoteConnectionGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		
		//OMsSqlCheckBox->setDisabled(true);
		OPostGresSqlCheckBox->setDisabled(true);
		OMySqlCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORemoteConnectionGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		
		//OMsSqlCheckBox->setEnabled(true);
		OPostGresSqlCheckBox->setEnabled(true);
		OMySqlCheckBox->setEnabled(true);
	}
}

void OBruteForceGui::OPostGresSqlCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		OEmailGroupBox->setDisabled(true);
		ORemoteConnectionGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		
		OMsSqlCheckBox->setDisabled(true);
		OMySqlCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORemoteConnectionGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		
		OMsSqlCheckBox->setEnabled(true);
		OMySqlCheckBox->setEnabled(true);
	}
}

void OBruteForceGui::OMySqlCheckBoxState(int State)
{
	if(State==Qt::Checked)
	{
		OEmailGroupBox->setDisabled(true);
		ORemoteConnectionGroupBox->setDisabled(true);
		ORepositoriesGroupBox->setDisabled(true);
		
		OMsSqlCheckBox->setDisabled(true);
		OPostGresSqlCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OEmailGroupBox->setEnabled(true);
		ORemoteConnectionGroupBox->setEnabled(true);
		ORepositoriesGroupBox->setEnabled(true);
		
		OMsSqlCheckBox->setEnabled(true);
		OPostGresSqlCheckBox->setEnabled(true);
	}
}

void OBruteForceGui::OCallAboutDialog()
{
OAbout->exec();
}

void OBruteForceGui::OMachinesState(int State)
{
	if(State==Qt::Checked)
	{
		OMachinesLineEdit->setEnabled(true);
		OMachinesPushButton->setEnabled(true);
		OIpSpinBox1->setDisabled(true);
		OIpSpinBox2->setDisabled(true);
		OIpSpinBox3->setDisabled(true);
		OIpSpinBox4->setDisabled(true);
		OMachinesCheckBox->setDisabled(true);
		OMachinesPushButton->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OMachinesPushButton->setDisabled(true);
		OIpSpinBox1->setEnabled(true);
		OIpSpinBox2->setEnabled(true);
		OIpSpinBox3->setEnabled(true);
		OIpSpinBox4->setEnabled(true);
		OMachinesCheckBox->setEnabled(true);
		OMachinesLineEdit->setDisabled(true);
	}
}

void OBruteForceGui::OCallUsersDialog()
{
UsersDialog->exec();
}
	
	
void OBruteForceGui::ODictionaryComboBoxState(int State)
{
	if(State==Qt::Checked)
	{
		ODictionaryComboBox->setEnabled(true);
		ODictionaryLineEdit->setDisabled(true);
		ODictionaryToolButton->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		ODictionaryComboBox->setDisabled(true);
		ODictionaryLineEdit->setEnabled(true);
		ODictionaryToolButton->setEnabled(true);
	}
}

void OBruteForceGui::OLoadDictionary()
{
	ODictionaryLineEdit->setText(QFileDialog::getOpenFileName());
}

void OBruteForceGui::OMachinesButtonState(int State)
{
	if(State==Qt::Checked)
	{
	OMachinesPushButton->setEnabled(true);
	OIpSpinBox1->setDisabled(true);
	OIpSpinBox2->setDisabled(true);
	OIpSpinBox3->setDisabled(true);
	OIpSpinBox4->setDisabled(true);
	OMachinesLineEdit->setDisabled(true);
	OMachinesNameCheckBox->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
	OMachinesPushButton->setDisabled(true);
	OIpSpinBox1->setEnabled(true);
	OIpSpinBox2->setEnabled(true);
	OIpSpinBox3->setEnabled(true);
	OIpSpinBox4->setEnabled(true);
	//OMachinesLineEdit->setEnabled(true);
	OMachinesNameCheckBox->setEnabled(true);
	
	}
}

void OBruteForceGui::OUsersButtonState(int State)
{
	if(State==Qt::Checked)
	{
		OUsersPushButton->setEnabled(true);
		OUsersLineEdit->setDisabled(true);
	}
	if(State==Qt::Unchecked)
	{
		OUsersPushButton->setDisabled(true);
		OUsersLineEdit->setEnabled(true);
	}
}

void OBruteForceGui::OCallMachinesDialog()
{
	MachinesDialog->exec();
}

