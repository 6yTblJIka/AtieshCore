/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "LoginDatabase.h"
#include "MySQLPreparedStatement.h"

void LoginDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_LOGINDATABASE_STATEMENTS);

    PrepareStatement(LOGIN_SEL_REALMLIST, "SELECT id, name, address, localAddress, localSubnetMask, port, icon, flag, timezone, allowedSecurityLevel, population, gamebuild FROM realmlist WHERE flag <> 3 ORDER BY name", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_DEL_EXPIRED_IP_BANS, "DELETE FROM ip_banned WHERE unbandate<>bandate AND unbandate<=UNIX_TIMESTAMP()", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_EXPIRED_ACCOUNT_BANS, "UPDATE account_banned SET active = 0 WHERE active = 1 AND unbandate<>bandate AND unbandate<=UNIX_TIMESTAMP()", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_IP_INFO, "SELECT unbandate > UNIX_TIMESTAMP() OR unbandate = bandate AS banned, NULL as country FROM ip_banned WHERE ip = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_IP_AUTO_BANNED, "INSERT INTO ip_banned (ip, bandate, unbandate, bannedby, banreason) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, 'Trinity Auth', 'Failed login autoban')", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_IP_BANNED_ALL, "SELECT ip, bandate, unbandate, bannedby, banreason FROM ip_banned WHERE (bandate = unbandate OR unbandate > UNIX_TIMESTAMP()) ORDER BY unbandate", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_IP_BANNED_BY_IP, "SELECT ip, bandate, unbandate, bannedby, banreason FROM ip_banned WHERE (bandate = unbandate OR unbandate > UNIX_TIMESTAMP()) AND ip LIKE CONCAT('%%', ?, '%%') ORDER BY unbandate", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BANNED_ALL, "SELECT account.id, username FROM account, account_banned WHERE account.id = account_banned.id AND active = 1 GROUP BY account.id", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BANNED_BY_FILTER, "SELECT account.id, username FROM account, account_banned WHERE account.id = account_banned.id AND active = 1 AND username LIKE CONCAT('%%', ?, '%%') GROUP BY account.id", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BANNED_BY_USERNAME, "SELECT account.id, username FROM account, account_banned WHERE account.id = account_banned.id AND active = 1 AND username = ? GROUP BY account.id", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_ACCOUNT_AUTO_BANNED, "INSERT INTO account_banned (id, bandate, unbandate, bannedby, banreason, active) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, 'Trinity Auth', 'Failed login autoban', 1)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_ACCOUNT_BANNED, "DELETE FROM account_banned WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_LOGON, "UPDATE account SET salt = ?, verifier = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_LOGONPROOF, "UPDATE account SET session_key_auth = ?, last_ip = ?, last_login = NOW(), locale = ?, failed_logins = 0, os = ? WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_LOGONCHALLENGE, "SELECT a.id, a.username, a.locked, a.lock_country, a.last_ip, a.failed_logins, ab.unbandate > UNIX_TIMESTAMP() OR ab.unbandate = ab.bandate, "
        "ab.unbandate = ab.bandate, aa.SecurityLevel, a.totp_secret, a.salt, a.verifier "
        "FROM account a LEFT JOIN account_access aa ON a.id = aa.AccountID LEFT JOIN account_banned ab ON ab.id = a.id AND ab.active = 1 WHERE a.username = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_RECONNECTCHALLENGE, "SELECT a.id, UPPER(a.username), a.locked, a.lock_country, a.last_ip, a.failed_logins, ab.unbandate > UNIX_TIMESTAMP() OR ab.unbandate = ab.bandate, "
        "ab.unbandate = ab.bandate, aa.SecurityLevel, a.session_key_auth "
        "FROM account a LEFT JOIN account_access aa ON a.id = aa.AccountID LEFT JOIN account_banned ab ON ab.id = a.id AND ab.active = 1 WHERE a.username = ? AND a.session_key_auth IS NOT NULL", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_FAILEDLOGINS, "UPDATE account SET failed_logins = failed_logins + 1 WHERE username = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_ID_BY_NAME, "SELECT id FROM account WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_LIST_BY_NAME, "SELECT id, username FROM account WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_INFO_BY_NAME, "SELECT a.id, a.session_key_auth, a.last_ip, a.locked, a.lock_country, a.expansion, a.mutetime, a.locale, a.recruiter, a.os, aa.SecurityLevel, "
        "ab.unbandate > UNIX_TIMESTAMP() OR ab.unbandate = ab.bandate, r.id FROM account a LEFT JOIN account_access aa ON a.id = aa.AccountID AND aa.RealmID IN (-1, ?) "
        "LEFT JOIN account_banned ab ON a.id = ab.id AND ab.active = 1 LEFT JOIN account r ON a.id = r.recruiter WHERE a.username = ? AND a.session_key_auth IS NOT NULL ORDER BY aa.RealmID DESC LIMIT 1", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_LIST_BY_EMAIL, "SELECT id, username FROM account WHERE email = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_REALM_CHARACTER_COUNTS, "SELECT realmid, numchars FROM realmcharacters WHERE  acctid = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BY_IP, "SELECT id, username FROM account WHERE last_ip = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BY_ID, "SELECT 1 FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_IP_BANNED, "INSERT INTO ip_banned (ip, bandate, unbandate, bannedby, banreason) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_IP_NOT_BANNED, "DELETE FROM ip_banned WHERE ip = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_ACCOUNT_BANNED, "INSERT INTO account_banned (id, bandate, unbandate, bannedby, banreason, active) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?, 1)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_NOT_BANNED, "UPDATE account_banned SET active = 0 WHERE id = ? AND active != 0", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_REALM_CHARACTERS, "DELETE FROM realmcharacters WHERE acctid = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_REP_REALM_CHARACTERS, "REPLACE INTO realmcharacters (numchars, acctid, realmid) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_SUM_REALM_CHARACTERS, "SELECT SUM(numchars) FROM realmcharacters WHERE acctid = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_ACCOUNT, "INSERT INTO account(username, salt, verifier, reg_mail, email, joindate) VALUES(?, ?, ?, ?, ?, NOW())", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_REALM_CHARACTERS_INIT, "INSERT INTO realmcharacters (realmid, acctid, numchars) SELECT realmlist.id, account.id, 0 FROM realmlist, account LEFT JOIN realmcharacters ON acctid = account.id WHERE acctid IS NULL", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_EXPANSION, "UPDATE account SET expansion = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_LOCK, "UPDATE account SET locked = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_LOCK_COUNTRY, "UPDATE account SET lock_country = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_LOG, "INSERT INTO logs (time, realm, type, level, string) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_USERNAME, "UPDATE account SET username = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_EMAIL, "UPDATE account SET email = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_REG_EMAIL, "UPDATE account SET reg_mail = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_MUTE_TIME, "UPDATE account SET mutetime = ? , mutereason = ? , muteby = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_MUTE_TIME_LOGIN, "UPDATE account SET mutetime = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_LAST_IP, "UPDATE account SET last_ip = ? WHERE username = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_LAST_ATTEMPT_IP, "UPDATE account SET last_attempt_ip = ? WHERE username = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_ONLINE, "UPDATE account SET online = 1 WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_UPTIME_PLAYERS, "UPDATE uptime SET uptime = ?, maxplayers = ? WHERE realmid = ? AND starttime = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_OLD_LOGS, "DELETE FROM logs WHERE (time + ?) < ? AND realm = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_ACCOUNT_ACCESS, "DELETE FROM account_access WHERE AccountID = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_ACCOUNT_ACCESS_BY_REALM, "DELETE FROM account_access WHERE AccountID = ? AND (RealmID = ? OR RealmID = -1)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_ACCOUNT_ACCESS, "INSERT INTO account_access (AccountID, SecurityLevel, RealmID) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME, "SELECT id FROM account WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_GET_GMLEVEL_BY_REALMID, "SELECT SecurityLevel FROM account_access WHERE AccountID = ? AND (RealmID = ? OR RealmID = -1) ORDER BY RealmID DESC", CONNECTION_BOTH);
    PrepareStatement(LOGIN_GET_USERNAME_BY_ID, "SELECT username FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_CHECK_PASSWORD, "SELECT salt, verifier FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_CHECK_PASSWORD_BY_NAME, "SELECT salt, verifier FROM account WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_PINFO, "SELECT a.username, aa.SecurityLevel, a.email, a.reg_mail, a.last_ip, DATE_FORMAT(a.last_login, '%Y-%m-%d %T'), a.mutetime, a.mutereason, a.muteby, a.failed_logins, a.locked, a.OS FROM account a LEFT JOIN account_access aa ON (a.id = aa.AccountID AND (aa.RealmID = ? OR aa.RealmID = -1)) WHERE a.id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_PINFO_BANS, "SELECT unbandate, bandate = unbandate, bannedby, banreason FROM account_banned WHERE id = ? AND active ORDER BY bandate ASC LIMIT 1", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_GM_ACCOUNTS, "SELECT a.username, aa.SecurityLevel FROM account a, account_access aa WHERE a.id = aa.AccountID AND aa.SecurityLevel >= ? AND (aa.RealmID = -1 OR aa.RealmID = ?)", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_INFO, "SELECT a.username, a.last_ip, aa.SecurityLevel, a.expansion FROM account a LEFT JOIN account_access aa ON a.id = aa.AccountID WHERE a.id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_ACCESS_SECLEVEL_TEST, "SELECT 1 FROM account_access WHERE AccountID = ? AND SecurityLevel > ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_ACCESS, "SELECT a.id, aa.SecurityLevel, aa.RealmID FROM account a LEFT JOIN account_access aa ON a.id = aa.AccountID WHERE a.username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_WHOIS, "SELECT username, email, last_ip FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_LAST_ATTEMPT_IP, "SELECT last_attempt_ip FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_LAST_IP, "SELECT last_ip FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_REALMLIST_SECURITY_LEVEL, "SELECT allowedSecurityLevel from realmlist WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_DEL_ACCOUNT, "DELETE FROM account WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_AUTOBROADCAST, "SELECT id, weight, text FROM autobroadcast WHERE realmid = ? OR realmid = -1", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_GET_EMAIL_BY_ID, "SELECT email FROM account WHERE id = ?", CONNECTION_SYNCH);
    // 0: uint32, 1: uint32, 2: uint32, 3: uint8, 4: uint32, 5: string // Complete name: "Login_Insert_AccountLoginDeLete_IP_Logging"
    PrepareStatement(LOGIN_INS_ALDL_IP_LOGGING, "INSERT INTO logs_ip_actions (account_id, character_guid, realm_id, type, ip, systemnote, unixtime, time) VALUES (?, ?, ?, ?, (SELECT last_ip FROM account WHERE id = ?), ?, unix_timestamp(NOW()), NOW())", CONNECTION_ASYNC);
    // 0: uint32, 1: uint32, 2: uint32, 3: uint8, 4: uint32, 5: string // Complete name: "Login_Insert_FailedAccountLogin_IP_Logging"
    PrepareStatement(LOGIN_INS_FACL_IP_LOGGING, "INSERT INTO logs_ip_actions (account_id, character_guid, realm_id, type, ip, systemnote, unixtime, time) VALUES (?, ?, ?, ?, (SELECT last_attempt_ip FROM account WHERE id = ?), ?, unix_timestamp(NOW()), NOW())", CONNECTION_ASYNC);
    // 0: uint32, 1: uint32, 2: uint32, 3: uint8, 4: string, 5: string // Complete name: "Login_Insert_CharacterDelete_IP_Logging"
    PrepareStatement(LOGIN_INS_CHAR_IP_LOGGING, "INSERT INTO logs_ip_actions (account_id, character_guid, realm_id, type, ip, systemnote, unixtime, time) VALUES (?, ?, ?, ?, ?, ?, unix_timestamp(NOW()), NOW())", CONNECTION_ASYNC);
    // 0: uint32, 1: string, 2: string                                 // Complete name: "Login_Insert_Failed_Account_Login_due_password_IP_Logging"
    PrepareStatement(LOGIN_INS_FALP_IP_LOGGING, "INSERT INTO logs_ip_actions (account_id, character_guid, realm_id, type, ip, systemnote, unixtime, time) VALUES (?, 0, 0, 1, ?, ?, unix_timestamp(NOW()), NOW())", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_ACCESS_BY_ID, "SELECT SecurityLevel, RealmID FROM account_access WHERE AccountID = ? and (RealmID = ? OR RealmID = -1) ORDER BY SecurityLevel desc", CONNECTION_SYNCH);

    PrepareStatement(LOGIN_SEL_RBAC_ACCOUNT_PERMISSIONS, "SELECT permissionId, granted FROM rbac_account_permissions WHERE accountId = ? AND (realmId = ? OR realmId = -1) ORDER BY permissionId, realmId", CONNECTION_BOTH);
    PrepareStatement(LOGIN_INS_RBAC_ACCOUNT_PERMISSION, "INSERT INTO rbac_account_permissions (accountId, permissionId, granted, realmId) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE granted = VALUES(granted)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_RBAC_ACCOUNT_PERMISSION, "DELETE FROM rbac_account_permissions WHERE accountId = ? AND permissionId = ? AND (realmId = ? OR realmId = -1)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_ACCOUNT_MUTE, "INSERT INTO account_muted VALUES (?, UNIX_TIMESTAMP(), ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_MUTE_INFO, "SELECT mutedate, mutetime, mutereason, mutedby FROM account_muted WHERE guid = ? ORDER BY mutedate ASC", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_DEL_ACCOUNT_MUTED, "DELETE FROM account_muted WHERE guid = ?", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_SEL_SECRET_DIGEST, "SELECT digest FROM secret_digest WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_SECRET_DIGEST, "INSERT INTO secret_digest (id, digest) VALUES (?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_SECRET_DIGEST, "DELETE FROM secret_digest WHERE id = ?", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_SEL_ACCOUNT_TOTP_SECRET, "SELECT totp_secret FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_UPD_ACCOUNT_TOTP_SECRET, "UPDATE account SET totp_secret = ? WHERE id = ?", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_INS_GM_CHAR_ITEM_ADD, "INSERT INTO account_gm_log_item (guid, player, account, item, item_guid, count, position, target, realmId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_GM_CHAR_LOG, "INSERT INTO account_gm_log (player, account, command, position, selected, realmId) VALUES (?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);

    // VIP system
    PrepareStatement(LOGIN_SET_ACCOUNT_PREMIUM, "INSERT INTO account_premium (id, setdate, unsetdate, active) VALUES (?, unix_timestamp(NOW()), ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_ACCOUNT_PREMIUM, "DELETE FROM account_premium WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_PREMIUM, "UPDATE account_premium SET setdate = unix_timestamp(NOW()), unsetdate = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_GET_ACCOUNT_PREMIUM_STATUS_BY_ID, "SELECT 1 FROM account_premium WHERE id = ? AND active = 1", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_GET_ACCOUNT_PREMIUM_UNSETDATE_BY_ID, "SELECT unsetdate FROM account_premium WHERE id = ?", CONNECTION_SYNCH);
    // VIP coins
    PrepareStatement(LOGIN_UPD_ACCOUNT_COINS, "UPDATE account SET coins = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_GET_ACCOUNT_COINS_BY_ID, "SELECT coins FROM account WHERE id = ?", CONNECTION_SYNCH);
    // Anticheat system
    PrepareStatement(LOGIN_INS_ACCOUNT_ANTICHEAT, "INSERT INTO anticheat_logs (account, player, description, position, realmId) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    // Warden log system
    PrepareStatement(LOGIN_INS_ACCOUNT_WARDEN, "INSERT INTO warden_log (`check`, `action`, `account`, `guid`, `map`, `position_x`, `position_y`, `position_z`) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
 }

LoginDatabaseConnection::LoginDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo)
{
}

LoginDatabaseConnection::LoginDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo)
{
}

LoginDatabaseConnection::~LoginDatabaseConnection()
{
}

