#include "widget.h"
#include "qchatclient.h"
#include "logindialog.h"
#include <QApplication>
#include <QTimer>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "=== main start ===";

    //自己输入测试地址和端口
    LoginDialog dlg;
    if (dlg.exec() != QDialog::Accepted) {

        return 0;
    }
    //MainWindow w;
    //w.show();
    qDebug()<<"login success";



    return a.exec();
}
