#include<OMachinesDialog.h>

OMachinesDialog::OMachinesDialog(QWidget *parent):QDialog(parent)
{
setupUi(this);
MachinesModel=new QStandardItemModel(parent);
OMachinesTableView->setModel(MachinesModel);
OMachinesTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
connect(OMachineAddPushButton,SIGNAL(clicked()),this,SLOT(OAddMachine()));
connect(OMachineRemovePushButton,SIGNAL(clicked()),this,SLOT(ORemoveMachine()));
connect(OClosePushButton,SIGNAL(clicked()),this,SLOT(OClose()));
}

OMachinesDialog::~OMachinesDialog()
{
delete MachinesModel;
}

void OMachinesDialog::OClose()
{
MachinesModel->clear();
close();
}

void OMachinesDialog::OAddMachine()
{
QStandardItem *Item=new QStandardItem(OMachinesLineEdit->text());
	MachinesModel->appendRow(Item);
//delete Item;
}

void OMachinesDialog::ORemoveMachine()
{
	MachinesModel->removeRow(0);
}

