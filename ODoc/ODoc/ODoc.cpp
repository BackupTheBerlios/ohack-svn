#include<ODoc.h>
#include<QtCore>
extern "C"
{
#include<stdlib.h>
}



ODoc::ODoc(QString folder,QWidget *parent):QMainWindow(parent)
{

setupUi(this);
Folder=folder;
delete tab_2;

setTree();
setViewer(Viewer);
setSplitter();
connections();

}

void ODoc::displayDoc(const QModelIndex &index)
//slot to link the actions from the textbrowser and the treeview
{


QString text=model->fileName(index);
tabWidget->setTabText(tabWidget->indexOf(tab),text);
//Viewer = new QTextBrowser(tabWidget->widget(1));
//setViewer(Viewer,tabWidget->widget(1));
text=path.remove("index.html")+text;

Viewer->setSource(text);



}

/*void ODoc::newTab(const QModelIndex &index)
//this is a slot that creates a new tab
{

QWidget *newtab=new QWidget;
tabWidget->addTab(newtab,"new tab");
QTextBrowser *Viewer=new QTextBrowser(newtab);
setViewer(Viewer,newtab);
Viewer->setSource(model->fileName(index));
//QGridLayout *layout=new QGridLayout(newtab);
//layout->addWidget(Viewer);

QTextStream out(stdout);
out<<tabWidget->currentWidget()<<"\n";

}*/

void ODoc::setTree()
//settings for the treeview
{

model = new QDirModel();
QStringList files("*.html");
files<<"*.htm"<<Folder;

model->setNameFilters(files);
model->setFilter( QDir::Dirs | QDir::Files);
//model->setFilter( QDir::Files );

Tree->setModel(model);
QString cleanPath(getenv("OHACK"));
	cleanPath+="/ODoc";
Tree->setRootIndex(model->index(QDir::cleanPath(cleanPath)));
Tree->hideColumn(2);
Tree->hideColumn(1);
Tree->hideColumn(3);


}

void ODoc::setViewer(QTextBrowser *Viewer)
//settings for the textbrowser
{

path+=getenv("OHACK");
path+="/ODoc/"+Folder+"/index.html";
//qDebug()<<path;
QUrl url(path);

Viewer->setSearchPaths(getSourcePath());

Viewer->setSource(url);
Viewer->setOpenExternalLinks (true);

QGridLayout *layout=new QGridLayout(tab);
layout->addWidget(Viewer);

tabWidget->setTabText(0,"Index");

} 

void ODoc::setSplitter()
//sets the frames to resizing
{

splitter=new QSplitter;
splitter->addWidget(Tree);
splitter->addWidget(tabWidget);
setCentralWidget(splitter);
splitter->setStretchFactor(0, 0);
splitter->setStretchFactor(1, 1);

}

void ODoc::connections()
//all the connections in this block
{

connect(Tree,SIGNAL(clicked(QModelIndex)),this,SLOT(displayDoc(QModelIndex)));
//connect(Tree,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(newTab(QModelIndex)));

connect(action_Previous,SIGNAL(activated()),Viewer,SLOT(backward()));
connect(action_Next,SIGNAL(activated()),Viewer,SLOT(forward()));
connect(action_Home,SIGNAL(activated()),Viewer,SLOT(home()));

}

QStringList ODoc::getSourcePath()
//this is to get the source path to search for the looking files for the browser
{
QStringList temp;

QModelIndex parentIndex = model->index(QDir::cleanPath(getenv("OHACK")));
     int numRows = model->rowCount(parentIndex);
for (int row = 0; row < numRows; ++row) {
         QModelIndex index = model->index(row, 0, parentIndex);
if(model->isDir(index)) {
temp<<model->filePath(index);
//QTextStream out(stdout);
//out<<model->filePath(index)<<"\n";
}
}
return temp;

}
