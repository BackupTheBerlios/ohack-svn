#ifndef OSESSIONNAMEDIALOG_H
#define OSESSIONNAMEDIALOG_H
#include<ui_OSessionNameDialog.h>
#include<QtCore>

class OSessionNameDialog : public QDialog , public Ui::OSessionNameDialog
{

Q_OBJECT
	public:
	OSessionNameDialog(QWidget *parent=0);
	QString OSessionName;
	public slots:
	void OGetSessionName();



};

#endif


