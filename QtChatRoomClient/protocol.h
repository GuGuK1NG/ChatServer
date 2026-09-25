#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
enum EnMsgType { LOGIN_MSG = 1, REG_MSG, REG_MSG_ACK, LOGIN_MSG_ACK, PRIVATE_CHAT_MSG,
                 ADD_FRIEND_MSG, CREATE_GROUP_MSG, ADD_GROUP_MSG, GROUP_CHAT_MSG, LOGOUT_MSG };
//登录协议
//js信息打包器，发送前打包好
inline QByteArray packMsg(const QJsonObject &js);

inline QByteArray packMsg(const QJsonObject &js){
    QByteArray data=QJsonDocument(js).toJson(QJsonDocument::Compact);
    data.append('\0');
    return data;
}
#endif // PROTOCOL_H
