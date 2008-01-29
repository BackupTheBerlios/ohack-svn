#include<ODoc.h>

int main(int argc,char **argv)
{

QApplication app(argc,argv);

ODoc Doc(argv[1]);

Doc.show();

return app.exec();
}
