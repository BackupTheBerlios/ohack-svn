#include<OLoadFilesDialog.h>
#include<stdlib.h>
#include<QtCore>
OLoadFilesDialog::OLoadFilesDialog(QWidget *parent):QDialog(parent)
{
OLoadFilesDialogStatus=false;
setupUi(this);
//OAdvanceWidget->setHidden(true);//para que el OLogWidget aparesca oculto inicialmente
this->adjustSize();
OPasswordFileLineEdit->setDisabled(true);
OGetPasswordFileToolButton->setDisabled(true);
OPasswordFileLabel->setDisabled(true);
//OAdvancePushButton->setDefault(true)
/*Inicia el Advance widget hide y redimensiona el dialogo*/
//connect(OAdvancePushButton,SIGNAL(clicked()),this,SLOT(OShowHide()));
//connect(OAdvancePushButton,SIGNAL(clicked()),this,SLOT(OResize()));
connect(OLoadPushButton,SIGNAL(clicked()),this,SLOT(OLoadShadow()));
connect(OPasswordFileCheckBox,SIGNAL(clicked(bool)),this,SLOT(OEnablePasswordFile(bool)));
connect(OGetFileToolButton,SIGNAL(clicked()),this,SLOT(OGetShadow()));
connect(OGetPasswordFileToolButton,SIGNAL(clicked()),this,SLOT(OGetPassword()));
// connect(OLoadPushButton,SIGNAL(clicked()),this,SLOT(OPutShadow(OJG)));

/** Objectos no implementados por ahora*/
OPasswordFileCheckBox->hide();
OPasswordFileLabel->hide();
OPasswordFileLineEdit->hide();
OGetPasswordFileToolButton->hide();
}

// void OLoadFilesDialog::OShowHide()
// {
// if(OAdvanceWidget->isHidden())
// {
// OAdvanceWidget->setVisible(true);
// }
// else
// {
// OAdvanceWidget->setHidden(true);
// }
// }

void OLoadFilesDialog::OEnablePasswordFile(bool Status)
{

if(Status)
{
OPasswordFileLineEdit->setEnabled(true);
OGetPasswordFileToolButton->setEnabled(true);
OPasswordFileLabel->setEnabled(true);
}

if(!Status)
{
OPasswordFileLineEdit->setDisabled(true);
OGetPasswordFileToolButton->setDisabled(true);
OPasswordFileLabel->setDisabled(true);
}

}

/*Redimensiona la e dialogo*/
// void OLoadFilesDialog::OResize()
// {
// 	this->adjustSize();
// }

void OLoadFilesDialog::OGetShadow()
{
 OShadowFile = QFileDialog::getOpenFileName(this);
 OShadowLineEdit->setText(OShadowFile);
}

void OLoadFilesDialog::OGetPassword()
{
 OPasswordFile = QFileDialog::getOpenFileName(this);
 OPasswordFileLineEdit->setText(OPasswordFile);
}

void OLoadFilesDialog::OLoadShadow()
{
int Counter;
OShadowFile=OShadowLineEdit->text();
QFile Shadow(OShadowFile);
// OJGUI->OShadowFile=OShadowFile;
Counter=OShadowFile.count();
	if(Counter==0)
	{
	/*Falta remplazar el dialogo de kdilog con el propio hecho en Olibs*/
	// OShadowLineEdit->insert(tr("Inserte Archivo Aqui"));
	Message.OShow(tr("Por Favor Ingrese Un Archivo"));
	}
	else if(!Shadow.exists())
	{
	Message.OShow(tr("El Archivo no no Existe"));
	}
	else if(!Shadow.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	Message.OShow(tr("El Archivo no se puede leer"));
	}
	else
	{
	/*Aui va ver que el archivo si tenga passwords pero por ahora solo close*/
	OLoadFilesDialogStatus=true;
	this->close();
	}
}


