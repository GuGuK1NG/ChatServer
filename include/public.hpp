#pragma once

enum EnMsgType
{
    LOGIN_MSG = 1,
    REG_MSG,
    REG_MSG_ACK,   // 注册响应
    LOGIN_MSG_ACK, // 登录响应
    PRIVATE_CHAT_MSG,
    ADD_FRIEND_MSG,

    CREATE_GROUP_MSG,
    ADD_GROUP_MSG,
    GROUP_CHAT_MSG,

    LOGOUT_MSG
};

//{"msgid":1,"name":"GGK","password":"123456"}
//{"msgid":1,"name":"GK","password":"123456"}

//{"msgid":5,"id":1,"from":"GGK","to":2,"msg":"NI HAO222!"}
//{"msgid":5,"id":2,"from":"GK","to":1,"msg":"NI HAO333!"}

//{"msgid":6,"id":1,"friendid":2}