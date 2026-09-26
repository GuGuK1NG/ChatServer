#include "qchatclient.h"



QChatClient *QChatClient::instance()
{
    static QChatClient* client=new QChatClient();
    return client;
}

QChatClient::QChatClient(QObject *parent)
    :QTcpSocket(parent)
{
    setProxy(QNetworkProxy::NoProxy); //Qt会自动使用代理，关闭先
    connect(this, &QAbstractSocket::readyRead,    this, &QChatClient::onReadyRead);
    connect(this, &QAbstractSocket::stateChanged, this, &QChatClient::onSocketStateChanged);
    connect(this, &QAbstractSocket::errorOccurred, this, &QChatClient::onErrorOccurred);
}



void QChatClient::connectToServer(const QString &host, quint16 port)
{
    m_buf.clear();
    if(state()==QAbstractSocket::ConnectedState
        && peerAddress().toString()==host&&peerPort()==port){
        return;
    }       //已连接，不重复连接
    abort();
    connectToHost(host,port);
    //这里没有错误处理，错误处理在信号槽函数onSocketStateChanged触发
}

bool QChatClient::isReady() const
{
    return state()==QAbstractSocket::ConnectedState;
}

void QChatClient::sendlogin(const QString &name, const QString &pwd)
{
    QJsonObject js;
    js["msgid"]=LOGIN_MSG;
    js["name"]=name;
    js["password"]=pwd;
    write(packMsg(js));
}

void QChatClient::sendreg(const QString &name, const QString &pwd)
{
    QJsonObject js;
    js["msgid"]=REG_MSG;
    js["name"]=name;
    js["password"]=pwd;
    write(packMsg(js));
}

void QChatClient::sendlogout()
{
    if(m_myId==-1){
        return;
    }//没有登录，直接返回
    QJsonObject js;
    js["msgid"]=LOGOUT_MSG;
    js["id"]=m_myId;
    write(packMsg(js));

    m_myId = -1;                   // 本地状态更新
    m_myName.clear();
}

int QChatClient::myId() const
{
    return m_myId;
}

QString QChatClient::myName() const
{
    return m_myName;
}

void QChatClient::onReadyRead()
{
    QByteArray data=readAll();
    QJsonDocument doc=QJsonDocument::fromJson(data);
    if(!doc.isObject()){
        qDebug()<<"解析失败!"<<data;
        return;
    }
    dispatch(doc.object());
}

void QChatClient::onSocketStateChanged(QAbstractSocket::SocketState s)
{
    switch (s) {
    case QAbstractSocket::ConnectedState:
        emit linkStateChanged(true, "连接成功");
        break;
    case QAbstractSocket::UnconnectedState:
        m_myId = -1;                          // 断开时清登录态
        emit linkStateChanged(false, "未连接");
        break;
    default:
        break;
    }
}

void QChatClient::onErrorOccurred(SocketError e)
{
    Q_UNUSED(e);
    qDebug() << "socket 错误:" << e << errorString();
}



void QChatClient::dispatch(const QJsonObject &js)
{
    switch (js["msgid"].toInt()) {
    case LOGIN_MSG_ACK:    handleLoginAck(js);    break;
    case REG_MSG_ACK:      handleRegAck(js);      break;
    case PRIVATE_CHAT_MSG: handlePrivateChat(js); break;
    default:
        qDebug() << "暂未处理的消息类型:" << js;    break;
    }
}

void QChatClient::handleLoginAck(const QJsonObject &js)
{
    const int errno_ = js["errno"].toInt();
    const QString errmsg = js["errmsg"].toString();

    if (errno_ == 0) {
        m_myId = js["id"].toInt();
        m_myName = js["name"].toString();

        m_friends.clear();
        const QJsonArray arr = js["friends"].toArray();
        for(const QJsonValue &v:arr){
            const QJsonDocument doc=QJsonDocument::fromJson(v.toString().toUtf8());
            if(!doc.isObject()){
                continue;
            }
            const QJsonObject obj =doc.object();
            User u;
            u.id    = obj["id"].toInt();
            u.name  = obj["name"].toString();
            u.state = obj["state"].toString();
            m_friends.append(u);
        }
    } else {
        m_myId = -1;
        m_myName.clear();
    }

    emit loginResult(errno_, errmsg);
}

void QChatClient::handleRegAck(const QJsonObject &js)
{
    const int errno_ = js["errno"].toInt();
    const QString errmsg = js["errmsg"].toString();
    emit regResult(errno_, errmsg);
    //注册之后暂时先不自动登录
}

void QChatClient::handlePrivateChat(const QJsonObject &js){

}
