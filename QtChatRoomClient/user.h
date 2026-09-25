#ifndef USER_H
#define USER_H
#include <QString>
struct User
{
    int id=-1;
    QString name;
    QString state;
    bool isOnline() const{return state==QStringLiteral("online");}
};

struct ChatMessage {
    int msgid = 0;
    int id = -1;
    QString name;
    int toid = -1;
    QString msg;
    QString time;
};

#endif // USER_H
