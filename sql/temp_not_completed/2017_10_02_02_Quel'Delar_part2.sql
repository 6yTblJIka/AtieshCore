
-- Table `creature_text`
DELETE FROM `creature_text` WHERE CreatureID IN (37781,37763,38056,37764,37765);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(38056, 11, 0, 'My lords and ladies, I present $n, bearer of Quel\'Delar.', 14, 0, 100, 0, 0, 0, 37820, 0, 'tainted_queldelar'),
(37781, 0, 0, 'Can that really be Quel\'Delar?', 14, 0, 100, 1, 0, 0, 37441, 0, 'tainted_queldelar'),
(37764, 1, 0, 'We shall see.', 14, 0, 100, 1, 0, 0, 37442, 0, 'tainted_queldelar'),
(37763, 2, 0, 'Look at it, Lor\'themar. It is certainly Quel\'Delar.', 14, 0, 100, 1, 0, 0, 37444, 0, 'tainted_queldelar'),
(37764, 3, 0, 'So it is. You have my thanks, $n, for restoring Quel\'Delar to its rightful owners', 12, 0, 100, 1, 0, 0, 37388, 0, 'tainted_queldelar'),
(37763, 4, 0, 'What is the meaning of this treachery?', 14, 0, 100, 1, 0, 0, 37389, 0, 'tainted_queldelar'),
(37781, 5, 0, 'Drop your weapon and surrender, traitor.', 14, 0, 100, 1, 0, 0, 37390, 0, 'tainted_queldelar'),
(37765, 6, 0, 'This is not my doing, Rommath. There is no treachery here.', 14, 0, 100, 1, 0, 0, 37430, 0, 'tainted_queldelar'),
(37765, 7, 0, 'Call off your men. Lor\'themar\'s own foolishness caused his injuries. Quel\'Delar is not chosen; it chooses its master.', 14, 0, 100, 1, 0, 0, 37431, 0, 'tainted_queldelar'),
(37763, 8, 0, 'Guards, return to your posts.', 14, 0, 100, 1, 0, 0, 37432, 0, 'tainted_queldelar'),
(37763, 9, 0, 'You will have what you have come for, $n. Take the sword and begone. And you, Auric, have a care for what you say in this sacred place.', 14, 0, 100, 1, 0, 0, 37434, 0, 'tainted_queldelar'),
(37765, 10, 0, 'Carry the sword back to Dalaran through that portal, $n. You have done what many quel\'dorei have dreamt of for years. Quel\'Delar has been restored at last.', 14, 0, 100, 1, 0, 0, 37440, 0, 'tainted_queldelar');

-- Table `gameobject_template`
UPDATE `gameobject_template` SET `ScriptName` = 'go_dalaran_portal_sunwell' WHERE (entry = 195682);
UPDATE `item_template` SET `ScriptName` = 'item_tainted_queldelar' WHERE `entry` IN (49879,49889);

DELETE FROM `creature` WHERE `id` IN (36991, 37523, 37746, 37763, 37764, 37765, 37781, 38056);
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawndist`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`unit_flags`,`dynamicflags`,`VerifiedBuild`) VALUES
(14568560, 37781, 580, 0, 0, 1, 2, 0,  0, 1720.53,  615.576, 28.0503, 5.59377,  300, 0, 0, 126, 0, 0, 0, 0, 0, 0),
(14568561, 37781, 580, 0, 0, 1, 2, 0,  0, 1686.59,  644.382, 28.1838, 2.15293,  300, 0, 0, 126, 0, 0, 0, 0, 0, 0),
(14568562, 37781, 580, 0, 0, 1, 2, 0,  0, 1658.78,  629.876, 28.3649, 5.07855,  300, 0, 0, 126, 0, 0, 0, 0, 0, 0),
(14568563, 36991, 580, 0, 0, 1, 2, 0,  0, 1739.09,  717.696, 71.1903, 4.47770,  300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568564, 36991, 580, 0, 0, 1, 2, 0,  0, 1761.82,  707.575, 71.1904, 3.84232,  300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568565, 36991, 580, 0, 0, 1, 2, 0,  0, 1778.09,  692.973, 71.1904, 3.73393,  300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568566, 37746, 580, 0, 0, 1, 2, 0,  0, 1698.45,  628.030, 28.1989, 3.47949,  300, 0, 0,  42, 0, 0, 0,33554432,0,0),
(14568567, 36991, 580, 0, 0, 1, 2, 0,  0, 1653.57,  634.692, 28.0849, 0.171978, 300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568568, 36991, 580, 0, 0, 1, 2, 0,  0, 1693.99,  674.897, 28.0503, 4.67702,  300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568569, 36991, 580, 0, 0, 1, 2, 0,  0, 1745.80,  622.226, 28.0503, 3.07638,  300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568570, 36991, 580, 0, 0, 1, 2, 0,  0, 1704.76,  583.796, 28.1657, 1.71842,  300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568571, 36991, 580, 0, 0, 1, 2, 0,  0, 1612.32,  606.707, 33.3215, 0.314329, 300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568572, 36991, 580, 0, 0, 1, 2, 0,  0, 1621.18,  584.858, 33.3215, 0.42507,  300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568573, 36991, 580, 0, 0, 1, 2, 0,  0, 1634.67,  563.787, 33.3215, 0.774572, 300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568574, 36991, 580, 0, 0, 1, 2, 0,  0, 1651.28,  548.805, 33.3215, 0.998411, 300, 0, 0,  84, 0, 0, 0, 0, 0, 0),
(14568575, 37781, 580, 0, 0, 1, 2, 0,  0, 1680.19,  620.726, 28.1394, 3.53524,  300, 0, 0, 126, 0, 0, 0, 0, 0, 0),
(14568576, 37763, 580, 0, 0, 1, 2, 0,  0, 1662.56,  622.758, 28.0502, 4.86257,  300, 0, 0,8200,1200, 0, 0, 0, 0, 0),
(14568577, 38056, 580, 0, 0, 1, 2, 0,  0, 1671.49,  606.427, 28.0504, 3.83603,  300, 0, 0,  41,60, 0, 0, 0, 0, 0),
(14568578, 37764, 580, 0, 0, 1, 2, 0,  0, 1665.90,  624.107, 28.0504, 5.00237,  300, 0, 0,16400,1800, 0, 0, 0, 0, 0),
(14568579, 37765, 580, 0, 0, 1, 2, 0,  0, 1703.85,  602.353, 28.0503, 2.64381,  300, 0, 0,  42, 0, 0, 0, 0, 0, 0),
(14568580, 37781, 580, 0, 0, 1, 2, 0,  0, 1666.81,  631.886, 28.0502, 4.56725,  300, 0, 0, 126, 0, 0, 0, 0, 0, 0),
(14568581, 37781, 580, 0, 0, 1, 2, 0,  0, 1659.63,  625.187, 28.2451, 1.65029,  300, 0, 0, 126, 0, 0, 0, 0, 0, 0),
(14568582, 37781, 580, 0, 0, 1, 2, 0,  0, 1671.96,  631.276, 28.0503, 4.46279,  300, 0, 0, 126, 0, 0, 0, 0, 0, 0),
(14568583, 37781, 580, 0, 0, 1, 2, 0,  0, 1671.33,  624.754, 28.0503, 1.472,    300, 0, 0, 126, 0, 0, 0, 0, 0, 0),
(14568584, 37781, 580, 0, 0, 1, 2, 0,  0, 1696.11,  608.161, 28.1093, 4.55154,  300, 0, 0, 126, 0, 0, 0, 0, 0, 0),
(14568590, 37523, 530, 0, 0, 1, 1, 0,  1, 12556.5, -6779.04, 15.0470, 0.0244971, 300, 0,0, 12600, 3994, 0, 0, 0, 0, 0),
(14568591, 37523, 530, 0, 0, 1, 1, 0,  1, 12556.5, -6770.14, 15.1006, 0.0340602, 300, 0,0, 12600, 3994, 0, 0, 0, 0, 0),
(14568592, 37523, 530, 0, 0, 1, 1, 0,  1, 12567.0, -6770.48, 15.0904, 6.22714,   300, 0,0, 12600, 3994, 0, 0, 0, 0, 0),
(14568593, 37523, 530, 0, 0, 1, 1, 0,  1, 12583.7, -6770.96, 15.0904, 6.26484,   300, 0,0, 12600, 3994, 0, 0, 0, 0, 0),
(14568594, 37523, 530, 0, 0, 1, 1, 0,  1, 12583.2, -6779.54, 15.0904, 0.132447,  300, 0,0, 12600, 3994, 0, 0, 0, 0, 0),
(14568595, 37523, 530, 0, 0, 1, 1, 0,  1, 12566.5, -6779.41, 15.0915, 0.00285489,300, 0,0, 12600, 3994, 0, 0, 0, 0, 0);

-- Table `creature_template`
UPDATE `creature_template` SET `ScriptName` = 'npc_queldelar_sunwell_plateau' WHERE (entry = 38056);
UPDATE `creature_template` SET `ScriptName` = 'npc_sunwell_warder' WHERE (entry = 37523);

DELETE FROM `gameobject` WHERE (id = 195682);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(21625, 195682, 712, 0, 0, 3, 1, 4.39529, 13.6833, 20.8039, 4.19445, 0, 0, 0.864607, -0.502449, 7200, 255, 1, 0),
(21630, 195682, 713, 0, 0, 3, 1, 22.177, 22.9527, 35.6576, 1.91986, 0, 0, 0.819151, 0.573577, 7200, 255, 1, 0),
(166014, 195682, 580, 0, 0, 1, 2, 1661.29, 657.049, 28.0513, 5.33584, 0, 0, -0.456159, 0.889899, 300, 255, 1, 0);