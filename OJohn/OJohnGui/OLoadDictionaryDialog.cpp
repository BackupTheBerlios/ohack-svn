#include<OLoadDictionaryDialog.h>
extern "C"
{
#include<stdlib.h>
}
OLoadDictionaryDialog::OLoadDictionaryDialog(QWidget *parent):QDialog(parent)
{
setupUi(this);


ODictionaryComboBox->setDisabled(true);

connect(ODictionayDefaultCheckBox,SIGNAL(toggled(bool)),this,SLOT(OEnableDefaultDictionary(bool)));

/*Conexion de los botones*/
connect(OLoadPushButton,SIGNAL(clicked()),this,SLOT(OLoadDictionary()));
connect(OQuitPushButton,SIGNAL(clicked()),this,SLOT(OClose()));
connect(OGetFileToolButton,SIGNAL(clicked()),this,SLOT(OGetDictionary()));



//qDebug()<<"Archivos = "<<Files<<endl;
}



OLoadDictionaryDialog::~OLoadDictionaryDialog()
{

}

/**/

void OLoadDictionaryDialog::OEnableDefaultDictionary(bool Status)
{
if(Status)
{
ODictionaryComboBox->setEnabled(true);
ODictionaryLineEdit->setDisabled(true);
OGetFileToolButton->setDisabled(true);
}
if(!Status)
{
ODictionaryComboBox->setDisabled(true);
ODictionaryLineEdit->setEnabled(true);
OGetFileToolButton->setEnabled(true);
}

}

/*******************************************************/
/*Construcciones de los SLOTS de los botones           */
/*******************************************************/

void OLoadDictionaryDialog::OClose()
{
close();
}

void OLoadDictionaryDialog::OLoadDictionary()
{
int Counter;

if(!ODictionayDefaultCheckBox->isChecked())
{
	ODictionaryFile=ODictionaryLineEdit->text();
	Counter=ODictionaryFile.count();
	if(Counter==0)
	{
	/*Falta remplazar el dialogo de kdilog con el propio hecho en Olibs*/
	// OShadowLineEdit->insert(tr("Inserte Archivo Aqui"));
	//system("kdialog --sorry \"Porfavor Ingrese Un Archivo \\n o seleccione uno abajo\"");
	Message.OShow(tr("Por Favor Ingrese Un Archivo \n o Seleccione Uno Abajo"));
	}
	else
	{
	/*Aui va ver que el archivo si tenga passwords pero por ahora solo close*/
	this->close();
	}
}
if(ODictionayDefaultCheckBox->isChecked())
{
ODictionaryFile=ODictionaryComboBox->currentText();
this->close();
}

}
void OLoadDictionaryDialog::OGetDictionary()
{
ODictionaryFile = QFileDialog::getOpenFileName(this);
ODictionaryLineEdit->setText(ODictionaryFile);
}







