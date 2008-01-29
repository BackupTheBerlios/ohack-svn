#include<OSessionNameDialog.h>
#include<stdlib.h>
#include<OMessage.h>

OSessionNameDialog::OSessionNameDialog(QWidget *parent):QDialog(parent)
{
setupUi(this);
connect(OAcceptPushButton,SIGNAL(clicked()),this,SLOT(OGetSessionName()));
}


void OSessionNameDialog::OGetSessionName()
{
OSessionName=OSessionLineEdit->text();
if(OSessionName.count()==0)
{
	/*Falta remplazar el dialogo de kdilog con el propio hecho en Olibs*/
	OMessage Message;
		Message.OShow("Porfavor Ingrese Un Nombre");
}
else
{
close();
}

}
