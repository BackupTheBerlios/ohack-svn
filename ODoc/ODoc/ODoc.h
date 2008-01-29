#ifndef ODOC_H
#define ODOC_H

#include<ui_ODoc.h>
#include<QtGui>

class ODoc:public QMainWindow,public Ui::ODocMainWindow
{
Q_OBJECT

public:

QString path;

ODoc(QString folder,QWidget *parent=0);
QStringList getSourcePath();
void setTree();
void setViewer(QTextBrowser *Viewer);
void setSplitter();
void connections();

public slots:

void displayDoc(const QModelIndex &index);
//void newTab(const QModelIndex &index);

private:

QDirModel *model;
QSplitter *splitter;
QString Folder;

};

#endif
