#ifndef OLOADFILESDIALOG_H
#define OLOADFILESDIALOG_H
#include<ui_OLoadFilesDialog.h>
#include<QtCore>
#include<OJohnGui.h>
#include<OMessage.h>
class OJohnGui;
class OLoadFilesDialog:public QDialog,public Ui::OLoadFilesDialog
{
Q_OBJECT
	public:
	bool OLoadFilesDialogStatus;
	OLoadFilesDialog(QWidget *parent=0);
	public slots:
	/*botones de los dialogos*/
	void OGetShadow();
	void OGetPassword();
	//void OResize();
	void OLoadShadow();
	//void OShowHide();
	void OEnablePasswordFile(bool Status);
	private:
	QString OSession;
	QString OShadowFile;
	QString OPasswordFile;
	QString UnshadowFile;
	OMessage Message;
};
#endif
