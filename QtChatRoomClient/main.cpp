#include "widget.h"
#include "qchatclient.h"

#include <QApplication>
#include <QTimer>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "=== main start ===";
    QChatClient *c = QChatClient::instance();
    QObject::connect(c, &QChatClient::linkStateChanged, [](bool ok, const QString &t){
        qDebug() << "连接状态:" << ok << t;
    });
    QObject::connect(c, &QChatClient::loginResult, [](int errno_, const QString &msg){
        qDebug() << "登录结果:" << errno_ << msg;
    });
    //自己输入测试地址和端口
    c->connectToServer("192.168.31.219", 6000);

    c->sendlogin("GGK", "123456");

    QTimer::singleShot(3000, &a, &QCoreApplication::quit);
    return a.exec();
}
