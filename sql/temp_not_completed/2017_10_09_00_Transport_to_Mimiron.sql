-- Transport Tram - remove
UPDATE `gameobject_template` SET `ScriptName`='go_transport_to_mimiron' WHERE `entry`='194675';
-- Transoirt offset coords in .gps
DELETE FROM `trinity_string` where `entry` IN (179);
INSERT INTO `trinity_string` (`entry`, `content_default`) VALUES ('179', 'TransOffsetX: %f TransOffsetY: %f TransOffsetZ: %f TransOffsetO: %f');

-- Transport Activate Tram
UPDATE `gameobject_template` SET `data2` = 3000, `ScriptName` = 'go_mimiron_activate_tram' WHERE `entry` IN (194914, 194912, 194437);
UPDATE `gameobject_template_addon` SET `flags` = 32 WHERE `entry` IN (194914, 194912, 194437);

DELETE FROM gameobject WHERE id = '194437';
INSERT INTO `gameobject` (`id`, `map`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`) VALUES
('194437','603','1','1','2306.87','274.237','424.288','1.52255','0','0','0.689847','0.723956','300','0','1');

DELETE FROM gameobject_template WHERE entry = '194438';
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `data0`, `data1`, `data2`, `data3`, `data4`, `data5`, `data6`, `data7`, `data8`, `data9`, `data10`, `data11`, `data12`, `data13`, `data14`, `data15`, `data16`, `data17`, `data18`, `data19`, `data20`, `data21`, `data22`, `data23`, `ScriptName`) VALUES
('194438','1','8504','Activate Tram','','','','1','0','0','3000','0','0','1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','go_mimiron_activate_tram');

DELETE FROM gameobject_template_addon WHERE entry = '194438';
INSERT INTO `gameobject_template_addon` (`entry`,`faction`, `flags`) VALUES
('194438','0','32');

DELETE FROM gameobject WHERE id = '194438';
INSERT INTO `gameobject` (`id`, `map`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`) VALUES
(194438, 603, 1,1,2306.99, 2589.35, 424.382, 4.71676, 0, 0, 0.705559, -0.708651, 300, 0, 1);