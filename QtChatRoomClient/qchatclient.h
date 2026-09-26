#ifndef QCHATCLIENT_H
#define QCHATCLIENT_H
#include <QDebug>
#include <QList>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QAbstractSocket>
#include <QNetworkProxy>
#include "user.h"
#include "protocol.h"
class QChatClient:public QTcpSocket
{
    Q_OBJECT
public:
    static QChatClient *instance();//单列模式
    void connectToServer(const QString &host,quint16 port);//连接服务器
    bool isReady() const;
    void sendlogin(const QString& name,const QString& pwd);
    void sendreg(const QString& name,const QString& pwd);
    void sendlogout();

    int myId() const;
    QString myName() const;



signals:
    void loginResult(int errno_, const QString &errmsg);
    void regResult(int errno_, const QString &errmsg);
    void privateChatReceived(const ChatMessage &msg);
    void linkStateChanged(bool connected, const QString &text);
private slots:
    void onReadyRead();
    void onSocketStateChanged(QAbstractSocket::SocketState s);
    void onErrorOccurred(QAbstractSocket::SocketError e);
private:
    explicit QChatClient(QObject *parent = nullptr);
    Q_DISABLE_COPY(QChatClient)
    void dispatch(const QJsonObject &js);
    void handleLoginAck(const QJsonObject &js);
    void handleRegAck(const QJsonObject &js);
    void handlePrivateChat(const QJsonObject &js);

    QByteArray m_buf;
    int m_myId = -1;
    QString m_myName;
    QList<User> m_friends;
};

#endif // QCHATCLIENT_H
