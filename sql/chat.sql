-- =============================================================
--  chat 聊天室数据库初始化脚本
--  表结构由 src/server 下各 Model 的 SQL 语句反推得到，
--  列名与列序必须与代码严格一致（lower_case_table_names=0，表名大小写敏感）。
--  用法: mysql -uroot -p < chat.sql
-- =============================================================

CREATE DATABASE IF NOT EXISTS chat
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_unicode_ci;

USE chat;

-- -------------------------------------------------------------
-- 用户表
--   usermodel.cpp: insert into User(name,password,state)
--                  select * from User ...  -> row[0]=id [1]=name [2]=password [3]=state
-- -------------------------------------------------------------
CREATE TABLE IF NOT EXISTS User (
    id       INT         NOT NULL AUTO_INCREMENT COMMENT '用户id',
    name     VARCHAR(50) NOT NULL                COMMENT '用户名',
    password VARCHAR(50) NOT NULL                COMMENT '密码',
    state    VARCHAR(20) NOT NULL DEFAULT 'offline' COMMENT '在线状态 online/offline',
    PRIMARY KEY (id),
    UNIQUE KEY uk_name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';

-- -------------------------------------------------------------
-- 好友关系表（双向存储，FriendModel::insert 会写两条）
--   friendmodel.cpp: insert into Friend values(userid,friendid)
--                    select ... inner join Friend b on b.friendid=a.id where b.userid=?
-- -------------------------------------------------------------
CREATE TABLE IF NOT EXISTS Friend (
    userid   INT NOT NULL COMMENT '用户id',
    friendid INT NOT NULL COMMENT '好友id',
    PRIMARY KEY (userid, friendid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='好友关系表';

-- -------------------------------------------------------------
-- 群组表
--   groupmodel.cpp: insert into AllGroup(groupname,groupdesc)
--                   select a.id,a.groupname,a.groupdesc from AllGroup a ... -> row[0..2]
-- -------------------------------------------------------------
CREATE TABLE IF NOT EXISTS AllGroup (
    id        INT          NOT NULL AUTO_INCREMENT COMMENT '群id',
    groupname VARCHAR(50)  NOT NULL                COMMENT '群名称',
    groupdesc VARCHAR(200) NOT NULL DEFAULT ''     COMMENT '群描述',
    PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群组表';

-- -------------------------------------------------------------
-- 群成员表
--   groupmodel.cpp: insert into GroupUser values(groupid,userid,role)
--                   select userid from GroupUser where groupid=? and userid!=?
--   role: creator(创建者) / normal(普通成员)
-- -------------------------------------------------------------
CREATE TABLE IF NOT EXISTS GroupUser (
    groupid INT         NOT NULL COMMENT '群id',
    userid  INT         NOT NULL COMMENT '用户id',
    `role`  VARCHAR(20) NOT NULL DEFAULT 'normal' COMMENT '角色 creator/normal',
    PRIMARY KEY (groupid, userid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='群成员表';

-- -------------------------------------------------------------
-- 离线消息表
--   offlinemessgae.cpp: insert into OfflineMessage values(userid,message)
--                       select message from OfflineMessage where userid=?
--   message 存的是整条 JSON 报文
-- -------------------------------------------------------------
CREATE TABLE IF NOT EXISTS OfflineMessage (
    userid  INT           NOT NULL COMMENT '接收方用户id',
    message VARCHAR(8192) NOT NULL COMMENT '离线消息内容(JSON报文)'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='离线消息表';