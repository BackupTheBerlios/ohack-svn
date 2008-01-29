#ifndef OLOADDICTIONARYDIALOG_H
#define OLOADDICTIONARYDIALOG_H

#include<ui_OLoadDictionaryDialog.h>
#include<QtCore>
#include<QtGui>
#include<OMessage.h>
class OMessage;
class OLoadDictionaryDialog : public QDialog , public Ui::OLoadDictionaryDialog
{
Q_OBJECT
Q_CLASSINFO("Autor","Omar Andrés Zapata Mesa")
Q_CLASSINFO("URL", "ohack.berlios.de")
	public:
	OLoadDictionaryDialog(QWidget *parent=0);
	~OLoadDictionaryDialog();
	QString ODictionaryFile;
	public slots:
	
	/*ençvento del checkbox*/
	void OEnableDefaultDictionary(bool Status);
	/*Eventos de los botnes*/
	void OGetDictionary();
	void OLoadDictionary();
	void OClose();
	private:
	OMessage Message;


};



#endif
