-- 端到端验收：完全照抄各个 Model 里的 SQL 语句，确认 chat 库能支撑服务端逻辑
USE chat;

-- ---------- usermodel.cpp ----------
INSERT INTO User(name,password,state) VALUES('alice','123456','offline');
SET @uid = LAST_INSERT_ID();
SELECT '[1] UserModel::query(name)' AS check_point;
SELECT * FROM User WHERE name = 'alice';

SELECT '[2] UserModel::updateState' AS check_point;
UPDATE User SET state = 'online' WHERE id = @uid;

-- ---------- friendmodel.cpp ----------
SELECT '[3] FriendModel insert + query' AS check_point;
INSERT INTO Friend VALUES(@uid, 99);
INSERT INTO Friend VALUES(99, @uid);
SELECT a.id,a.name,a.state FROM User a INNER JOIN Friend b ON b.friendid = a.id WHERE b.userid=99;

-- ---------- groupmodel.cpp ----------
SELECT '[4] GroupModel::createGroup' AS check_point;
INSERT INTO AllGroup(groupname,groupdesc) VALUES('g1','d1');
SET @gid = LAST_INSERT_ID();
INSERT INTO GroupUser VALUES(@gid, @uid, 'creator');
SELECT a.id,a.groupname,a.groupdesc FROM AllGroup a INNER JOIN GroupUser b ON a.id=b.groupid WHERE b.userid=@uid;

SELECT '[5] GroupModel::queryGroupUsers' AS check_point;
SELECT userid FROM GroupUser WHERE groupid = @gid AND userid!=99;

-- ---------- offlinemessgae.cpp ----------
SELECT '[6] OfflineMessage insert + query + remove' AS check_point;
INSERT INTO OfflineMessage VALUES(@uid, '{"msgid":5,"msg":"hi"}');
SELECT message FROM OfflineMessage WHERE userid = @uid;
DELETE FROM OfflineMessage WHERE userid=@uid;

-- ---------- 清理测试数据 ----------
DELETE FROM GroupUser WHERE groupid=@gid;
DELETE FROM AllGroup WHERE id=@gid;
DELETE FROM Friend WHERE userid=@uid OR friendid=@uid;
DELETE FROM User WHERE id=@uid;

SELECT '[7] cleaned, row counts' AS check_point;
SELECT
  (SELECT COUNT(*) FROM User)           AS User_rows,
  (SELECT COUNT(*) FROM Friend)         AS Friend_rows,
  (SELECT COUNT(*) FROM AllGroup)       AS AllGroup_rows,
  (SELECT COUNT(*) FROM GroupUser)      AS GroupUser_rows,
  (SELECT COUNT(*) FROM OfflineMessage) AS OfflineMessage_rows;

SELECT '=== ALL CHECKS PASSED ===' AS result;