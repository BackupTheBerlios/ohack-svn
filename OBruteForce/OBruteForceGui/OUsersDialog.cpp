#include<OUsersDialog.h>

OUsersDialog::OUsersDialog(QWidget *parent):QDialog(parent)
{
setupUi(this);
UsersModel=new  QStandardItemModel(parent);
OUsersTableView->setModel(UsersModel);
OUsersTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);/** para no editar "setEdit Triggers=Fijador"*/
connect(OUsersAddPushButton,SIGNAL(clicked()),this,SLOT(OAddUser()));
connect(OUsersRemovePushButton,SIGNAL(clicked()),this,SLOT(ORemoveUser()));
connect(OCancelPushButton,SIGNAL(clicked()),this,SLOT(OCancel()));
}

OUsersDialog::~OUsersDialog()
{
delete UsersModel;
}

void OUsersDialog::OAddUser()
{
	QStandardItem *Item = new QStandardItem(OUsersLineEdit->text());
	UsersModel->appendRow(Item);
	//OUsersIndex=OUsersTableView->currentIndex();
	//delete Item;
}

void OUsersDialog::ORemoveUser()
{
	UsersModel->removeRow(0);
	//OUsersIndex=OUsersTableView->currentIndex();
}

void OUsersDialog::OCancel()
{
//UsersModel->clear();
close();
}

