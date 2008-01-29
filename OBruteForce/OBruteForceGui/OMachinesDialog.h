#ifndef OMACHONESDIALOG_H
#define OMACHINESDIALOG_H
#include<ui_OMachinesDialog.h>
#include<QtGui>
class OMachinesDialog:public QDialog,public Ui::OMachinesDialog
{
	Q_OBJECT
	public:
		OMachinesDialog(QWidget *parent=0);
		~OMachinesDialog();
		QStandardItemModel *MachinesModel;
	public slots:
	void OAddMachine();
	void ORemoveMachine();
	void OClose();
};

#endif
