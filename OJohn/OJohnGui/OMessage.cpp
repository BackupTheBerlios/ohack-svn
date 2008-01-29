#include<OMessage.h>

OMessage::OMessage(QWidget *parent):QDialog(parent)
{
setupUi(this);
}

 
void OMessage::OShow(QString Message)
{
OMessageLabel->setText(Message);
this->exec();
}
