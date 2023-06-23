-- 
DELETE FROM `creature` WHERE `guid` IN (60891, 60892, 60894, 60895, 60898, 60899, 60900, 60901, 60902, 60903, 60904, 60905, 60906, 60907, 60908, 60909, 60910, 60911, 60912, 60913, 60914, 60915, 60916, 60917, 60918, 60920, 61672, 61673, 61674, 61675, 61676, 61677, 61678, 61679, 61680, 61681, 61682, 61683, 61684, 61685, 61686, 61687, 61688, 61689, 61690, 61691, 61692, 61693);
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`) VALUES
(60891, 12998, 0, 1, 1, 12932, 1, -1853.49, -4091.65, 9.81663, 4.70644, 300, 0, 0, 656, 0, 0),
(60892, 12998, 0, 1, 1, 12933, 1, -1853.19, -4122.06, 11.1314, 4.76928, 300, 0, 0, 656, 0, 2),
(60894, 12998, 0, 1, 1, 0, 1, -4110.72, -1068.8, 168.909, 4.87338, 300, 5, 0, 656, 0, 1),
(60895, 12998, 0, 1, 1, 0, 1, -4147.28, -1106.97, 168.917, 0.341632, 300, 0, 0, 656, 0, 0),
(60898, 12998, 0, 1, 1, 0, 1, -4029.86, -1453.53, 167.187, 4.37309, 300, 5, 0, 656, 0, 1),
(60899, 12998, 0, 1, 1, 0, 1, -4101.46, -1260.41, 148.958, 4.91972, 300, 0, 0, 656, 0, 0),
(60900, 12998, 0, 1, 1, 0, 1, -4074.56, -1256.74, 146.399, 0.106011, 300, 0, 0, 656, 0, 2),
(60901, 2098, 0, 1, 1, 0, 0, -1813.51, -4155.14, 9.89494, 1.82635, 300, 0, 0, 102, 0, 0),
(60902, 2098, 0, 1, 1, 0, 0, -1818.03, -4142.99, 9.89494, 4.68913, 300, 0, 0, 102, 0, 0),
(60903, 6368, 0, 1, 1, 0, 0, -1855.72, -4117.62, 13.0457, 5.2373, 300, 0, 0, 1, 0, 0),
(60904, 6368, 0, 1, 1, 0, 0, -1839.99, -4245.72, 2.13572, 1.15007, 300, 8, 0, 1, 0, 1),
(60905, 4075, 0, 1, 1, 0, 0, -1818.5, -4149.38, 9.89524, 0.0426621, 300, 15, 0, 8, 0, 1),
(60906, 4075, 0, 1, 1, 0, 0, -1853.79, -4149.49, 9.74445, 5.99991, 300, 15, 0, 8, 0, 1),
(60907, 4075, 0, 1, 1, 0, 0, -1832.93, -4231.47, 2.13493, 4.15422, 300, 15, 0, 8, 0, 1),
(60908, 4075, 0, 1, 1, 0, 0, -1765.09, -4228.05, 2.06121, 3.92646, 300, 15, 0, 8, 0, 1),
(60909, 4075, 0, 1, 1, 0, 0, -1685.3, -4200.7, 1.99848, 0.324622, 300, 15, 0, 8, 0, 1),
(60910, 4075, 0, 1, 1, 0, 0, -1720.4, -4190.35, 1.99845, 2.78763, 300, 15, 0, 8, 0, 1),
(60911, 4075, 0, 1, 1, 0, 0, -1953.66, -4080.47, 2.03243, 1.65973, 300, 15, 0, 8, 0, 1),
(60912, 4075, 0, 1, 1, 0, 0, -1981, -4082.81, 2.06496, 2.00923, 300, 15, 0, 8, 0, 1),
(60913, 4075, 0, 1, 1, 0, 0, -1952.94, -4118.76, 2.0329, 5.26471, 300, 15, 0, 8, 0, 1),
(60914, 4075, 0, 1, 1, 0, 0, -1933.78, -4107.02, 2.0329, 5.249, 300, 15, 0, 8, 0, 1),
(60915, 4075, 0, 1, 1, 0, 0, -1898.76, -4155.48, 2.0329, 6.10665, 300, 15, 0, 8, 0, 1),
(60916, 4075, 0, 1, 1, 0, 0, -1773.96, -4198.48, 1.99868, 5.41943, 300, 15, 0, 8, 0, 1),
(60917, 727, 0, 1, 1, 0, 0, -3972.82, -1513.31, 169.195, 5.53077, 300, 0, 0, 1910, 0, 0),
(60918, 727, 0, 1, 1, 0, 0, -3985.24, -1508.53, 168.705, 4.14848, 300, 0, 0, 1910, 0, 0),
(60920, 721, 0, 1, 1, 0, 0, -3971.98, -1494.18, 169.931, 2.12686, 300, 15, 0, 1, 0, 1),
(61672, 721, 0, 1, 1, 0, 0, -3994.42, -1376.7, 150.124, 1.10741, 300, 15, 0, 1, 0, 1),
(61673, 721, 0, 1, 1, 0, 0, -4026.06, -1270.89, 146.802, 1.82841, 300, 15, 0, 1, 0, 1),
(61674, 721, 0, 1, 1, 0, 0, -4096.43, -1088.23, 170.199, 1.84097, 300, 15, 0, 1, 0, 1),
(61675, 2110, 0, 1, 1, 0, 0, -4110.85, -1068.93, 168.91, 1.74516, 300, 15, 0, 1, 0, 1),
(61676, 2110, 0, 1, 1, 0, 0, -4048.29, -1227.35, 146.314, 5.142, 300, 15, 0, 1, 0, 1),
(61677, 2442, 0, 1, 1, 0, 0, -4053.71, -1187.58, 146.314, 4.69747, 300, 7, 0, 1, 0, 1),
(61678, 2442, 0, 1, 1, 0, 0, -4041.5, -1230.92, 146.314, 3.80212, 300, 7, 0, 1, 0, 1),
(61679, 2442, 0, 1, 1, 0, 0, -4055.91, -1247.15, 146.314, 4.19089, 300, 7, 0, 1, 0, 1),
(61680, 2098, 0, 1, 1, 0, 0, -4142.79, -1053.89, 167.925, 5.52528, 300, 0, 0, 102, 0, 0),
(61681, 2098, 0, 1, 1, 0, 0, -4131.62, -1063.01, 167.925, 4.41786, 300, 0, 0, 102, 0, 0),
(61682, 620, 0, 1, 1, 0, 0, -4126.41, -1091.58, 168.916, 4.29377, 300, 7, 0, 1, 0, 1),
(61683, 620, 0, 1, 1, 0, 0, -4117.23, -1101.1, 168.953, 3.54764, 300, 7, 0, 1, 0, 1),
(61684, 620, 0, 1, 1, 0, 0, -4137.29, -1102.33, 168.916, 3.74399, 300, 7, 0, 1, 0, 1),
(61685, 620, 0, 1, 1, 0, 0, -4080.96, -1256.69, 146.684, 3.85709, 300, 7, 0, 1, 0, 1),
(61686, 620, 0, 1, 1, 0, 0, -4074.13, -1268.01, 146.158, 4.99592, 300, 7, 0, 1, 0, 1),
(61687, 620, 0, 1, 1, 0, 0, -4091.94, -1279.57, 146.18, 3.76912, 300, 7, 0, 1, 0,  1),
(61688, 620, 0, 1, 1, 0, 0, -4039.9, -1446.43, 166.2, 4.68177, 300, 7, 0, 1, 0, 1),
(61689, 620, 0, 1, 1, 0, 0, -4022.11, -1449.52, 165.607, 5.71378, 300, 7, 0, 1, 0, 1),
(61690, 620, 0, 1, 1, 0, 0, -4017.85, -1460.55, 168.343, 4.69512, 300, 7, 0, 1, 0, 1),
(61691, 620, 0, 1, 1, 0, 0, -4001.15, -1502.53, 169.703, 5.06818, 300, 7, 0, 1, 0, 1),
(61692, 620, 0, 1, 1, 0, 0, -3964.18, -1526.38, 169.163, 5.06818, 300, 7, 0, 1, 0, 1),
(61693, 12996, 0, 1, 1, 0, 0, -4025.61, -1404.07, 155.248, 3.81547, 300, 0, 0, 1910, 0, 2);

DELETE FROM `creature_addon` WHERE `guid` IN (60891,60892,60900,61693,60903);
INSERT INTO `creature_addon` (`guid`, `bytes2`, `path_id`, `bytes1`) VALUES
(60891, 1, 0, 3),
(60903, 0, 0, 3),
(60892, 1, 608920, 0),
(60900, 1, 609000, 0),
(61693, 1, 616930, 0);

DELETE FROM `waypoint_data` WHERE `id` IN (608920, 609000, 616930);
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `action_chance`) VALUES
(608920, 1, -1851.5, -4161.39, 8.0422, 255, 5000,100),
(608920, 2, -1849.24, -4170.83, 6.40288, 255, 0,100),
(608920, 3, -1843.77, -4176.64, 5.19335, 255, 0,100),
(608920, 4, -1827.66, -4188.65, 3.6245, 255, 0,100),
(608920, 5, -1821.43, -4200.03, 3.37323, 255, 5000,100),
(609000, 1, -4056.62, -1257.13, 146.314, 255, 0,100),
(609000, 2, -4022.13, -1239.04, 146.314, 255, 0,100),
(609000, 3, -4027.52, -1197.61, 146.314, 255, 0,100),
(609000, 4, -4073.56, -1205.96, 146.314, 255, 0,100),
(609000, 5, -4074.82, -1234.27, 146.481, 255, 0,100),
(609000, 6, -4071.62, -1244.78, 146.406, 255, 0,100),
(609000, 7, -4073.64, -1253.11, 146.433, 255, 0,100),
(616930, 1, -4032.54, -1410.32, 156.721, 255, 0, 100),
(616930, 2, -4074.89, -1407.43, 166.031, 255, 0, 100),
(616930, 3, -4121.2, -1392.13, 186.617, 255, 0, 100),
(616930, 4, -4139.65, -1392.91, 194.53, 255, 0, 100),
(616930, 5, -4160.59, -1400.4, 197.085, 255, 0, 100),
(616930, 6, -4139.07, -1393.15, 194.301, 255, 0, 100),
(616930, 7, -4120.76, -1392.22, 186.412, 255, 0, 100),
(616930, 8, -4074.23, -1407.58, 165.77, 255, 0, 100),
(616930, 9, -4032.55, -1410.22, 156.712, 255, 0, 100),
(616930, 10, -4020.63, -1397.18, 153.371, 255, 0, 100),
(616930, 11, -4017.04, -1356.28, 148.046, 255, 0, 100),
(616930, 12, -4009.14, -1314.06, 146.624, 255, 0, 100),
(616930, 13, -4018.54, -1285.38, 146.625, 255, 0, 100),
(616930, 14, -3993.26, -1242.82, 146.911, 255, 0, 100),
(616930, 15, -3994.51, -1185.12, 154.931, 255, 0, 100),
(616930, 16, -4020.88, -1159.39, 157.864, 255, 0, 100),
(616930, 17, -4044.1, -1151.67, 160.279, 255, 0, 100),
(616930, 18, -4067.39, -1109.54, 166.369, 255, 0, 100),
(616930, 19, -4087.61, -1099.67, 168.504, 255, 0, 100),
(616930, 20, -4128.91, -1098.02, 168.917, 255, 0, 100),
(616930, 21, -4136.27, -1079.66, 168.917, 255, 0, 100),
(616930, 22, -4128.58, -1098.3, 168.916, 255, 0, 100),
(616930, 23, -4086.78, -1099.88, 168.434, 255, 0, 100),
(616930, 24, -4066.85, -1110.06, 166.183, 255, 0, 100),
(616930, 25, -4043.64, -1151.81, 160.212, 255, 0, 100),
(616930, 26, -4020.62, -1159.42, 157.841, 255, 0, 100),
(616930, 27, -3994.12, -1185.26, 154.956, 255, 0, 100),
(616930, 28, -3992.99, -1243.2, 146.901, 255, 0, 100),
(616930, 29, -4018.42, -1285.85, 146.623, 255, 0, 100),
(616930, 30, -4008.84, -1314.12, 146.627, 255, 0, 100),
(616930, 31, -4016.71, -1356.63, 148.065, 255, 0, 100),
(616930, 32, -4020.15, -1397.47, 153.434, 255, 0, 100);
