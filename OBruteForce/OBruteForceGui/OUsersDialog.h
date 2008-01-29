#ifndef OUSERDIALOG_H
#define OUSERDIALOG_H
#include<ui_OUsersDialog.h>
#include<QtGui>
class OUsersDialog:public QDialog, Ui::OUsersDialog
{
	Q_OBJECT
	public:
	OUsersDialog(QWidget *parent=0);
	~OUsersDialog();
	QModelIndex OUsersIndex;
	QStandardItemModel *UsersModel;
	public slots:
	void OAddUser();
	void ORemoveUser();
	void OCancel();
	
	
};

#endif
