#ifndef OMESSAGE_H
#define OMESSAGE_H
#include<ui_OMessage.h>
class QDialog;
class OMessage:public QDialog,public Ui::OMessageDialog
{
	public:
	OMessage(QWidget *parent=0);
	/*Sobrecargada para varios casos*/
	void OShow(QString Message);
};

#endif

