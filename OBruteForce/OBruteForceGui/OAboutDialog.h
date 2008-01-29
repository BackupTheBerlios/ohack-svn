#ifndef OABOUTDIALOG_H
#define OABOUTDIALOG_H
#include<ui_OAboutDialog.h>
class OAboutDialog:public QDialog,public Ui::OAboutDialog
{
	public:
	OAboutDialog(QWidget *parent=0);

};

#endif
