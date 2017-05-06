/* day count in ummulqura calendar (from 1318 AH till 1444 AH) */
static const unsigned int dat_diyanet[] = {
	15816U, 0U,
	15141, 15171, 15200, 15230, 15259, 15289, 15318, 15348, 15377,
	15407, 15436, 15466, 15495, 15525, 15554, 15584, 15613, 15643,
	15672, 15702, 15731, 15761, 15790, 15820, 15850, 15880, 15909,
	15939, 15968, 15998, 16027, 16057, 16086, 16116, 16145, 16175,
	16204, 16234, 16263, 16293, 16322, 16352, 16381, 16411, 16440,
	16470, 16499, 16529, 16558, 16588, 16617, 16647, 16676, 16706,
	16735, 16765, 16794, 16824, 16853, 16883, 16913, 16943, 16972,
	17002, 17031, 17061, 17090, 17120, 17149, 17179, 17208, 17238,
	17267, 17297, 17326, 17356, 17385, 17415, 17444, 17474, 17503,
	17533, 17562, 17592, 17621, 17651, 17680, 17710, 17739, 17769,
	17798, 17828, 17857, 17887, 17916, 17946, 17976, 18006, 18035,
	18065, 18094, 18124, 18153, 18183, 18212, 18242, 18271, 18301,
	18330, 18360, 18389, 18419, 18448, 18478, 18507, 18537, 18566,
	18596, 18625, 18655, 18685, 18715, 18744, 18774, 18803, 18833,
	18862, 18892, 18921, 18951, 18980, 19010, 19039, 19069, 19098,
	19128, 19157, 19187, 19216, 19246, 19275, 19305, 19334, 19364,
	19393, 19423, 19452, 19482, 19511, 19541, 19570, 19600, 19629,
	19659, 19688, 19718, 19748, 19778, 19807, 19837, 19866, 19896,
	19925, 19955, 19984, 20014, 20043, 20073, 20102, 20132, 20161,
	20191, 20220, 20250, 20279, 20309, 20338, 20368, 20397, 20427,
	20456, 20486, 20515, 20545, 20574, 20604, 20633, 20663, 20692,
	20722, 20751, 20781, 20811, 20841, 20870, 20900, 20929, 20959,
	20988, 21018, 21047, 21077, 21106, 21136, 21165, 21195, 21224,
	21254, 21283, 21313, 21342, 21372, 21401, 21431, 21460, 21490,
	21519, 21549, 21578, 21608, 21637, 21667, 21696, 21726, 21755,
	21785, 21814, 21844, 21874, 21904, 21933, 21963, 21992, 22022,
	22051, 22081, 22110, 22140, 22169, 22199, 22228, 22258, 22287,
	22317, 22346, 22376, 22405, 22435, 22464, 22494, 22523, 22553,
	22583, 22613, 22642, 22672, 22701, 22731, 22760, 22790, 22819,
	22849, 22878, 22908, 22937, 22967, 22996, 23026, 23055, 23085,
	23114, 23144, 23173, 23203, 23232, 23262, 23291, 23321, 23350,
	23380, 23409, 23439, 23468, 23498, 23527, 23557, 23586, 23616,
	23646, 23676, 23705, 23735, 23764, 23794, 23823, 23853, 23882,
	23912, 23941, 23971, 24000, 24030, 24059, 24089, 24118, 24148,
	24177, 24207, 24236, 24266, 24295, 24325, 24354, 24384, 24413,
	24443, 24472, 24502, 24531, 24561, 24590, 24619, 24648, 24678,
	24708, 24738, 24768, 24797, 24827, 24857, 24886, 24916, 24945,
	24974, 25004, 25033, 25063, 25092, 25122, 25151, 25181, 25211,
	25241, 25270, 25300, 25329, 25359, 25388, 25417, 25446, 25476,
	25505, 25535, 25565, 25595, 25624, 25654, 25684, 25713, 25743,
	25772, 25801, 25830, 25860, 25889, 25919, 25949, 25978, 26008,
	26038, 26067, 26097, 26126, 26156, 26185, 26214, 26244, 26274,
	26303, 26333, 26362, 26392, 26422, 26451, 26481, 26510, 26540,
	26569, 26599, 26628, 26658, 26687, 26716, 26746, 26776, 26805,
	26835, 26865, 26895, 26924, 26953, 26983, 27012, 27042, 27071,
	27100, 27130, 27159, 27189, 27219, 27248, 27278, 27308, 27338,
	27367, 27396, 27426, 27455, 27484, 27514, 27543, 27573, 27602,
	27632, 27662, 27692, 27722, 27751, 27780, 27809, 27839, 27868,
	27898, 27927, 27957, 27986, 28016, 28046, 28076, 28105, 28134,
	28164, 28194, 28223, 28252, 28282, 28311, 28340, 28370, 28400,
	28430, 28460, 28489, 28518, 28548, 28578, 28607, 28636, 28666,
	28695, 28725, 28754, 28784, 28814, 28843, 28872, 28902, 28932,
	28962, 28991, 29020, 29050, 29079, 29109, 29138, 29168, 29197,
	29226, 29256, 29286, 29316, 29345, 29375, 29404, 29434, 29463,
	29492, 29522, 29551, 29581, 29610, 29640, 29670, 29700, 29729,
	29759, 29789, 29818, 29847, 29877, 29906, 29935, 29965, 29994,
	30024, 30054, 30083, 30113, 30143, 30172, 30202, 30231, 30261,
	30290, 30319, 30349, 30378, 30408, 30438, 30467, 30497, 30526,
	30556, 30586, 30615, 30645, 30674, 30703, 30733, 30762, 30792,
	30821, 30851, 30880, 30910, 30940, 30969, 30999, 31028, 31058,
	31087, 31117, 31146, 31176, 31205, 31235, 31264, 31294, 31323,
	31353, 31383, 31412, 31442, 31472, 31501, 31530, 31560, 31589,
	31618, 31648, 31677, 31707, 31737, 31766, 31796, 31826, 31856,
	31885, 31914, 31944, 31973, 32002, 32032, 32062, 32091, 32120,
	32150, 32180, 32210, 32239, 32269, 32298, 32328, 32357, 32386,
	32416, 32445, 32475, 32504, 32534, 32564, 32594, 32623, 32653,
	32682, 32712, 32741, 32770, 32800, 32829, 32859, 32888, 32918,
	32948, 32977, 33007, 33037, 33066, 33096, 33125, 33154, 33184,
	33213, 33243, 33272, 33302, 33331, 33361, 33391, 33420, 33450,
	33479, 33509, 33539, 33568, 33598, 33627, 33656, 33686, 33715,
	33745, 33774, 33804, 33834, 33863, 33893, 33923, 33952, 33982,
	34011, 34040, 34070, 34099, 34129, 34158, 34188, 34217, 34247,
	34277, 34307, 34336, 34366, 34395, 34424, 34454, 34483, 34512,
	34542, 34571, 34601, 34631, 34661, 34691, 34720, 34749, 34779,
	34808, 34838, 34867, 34896, 34926, 34955, 34985, 35015, 35045,
	35074, 35104, 35133, 35163, 35192, 35222, 35251, 35280, 35310,
	35340, 35369, 35399, 35428, 35458, 35488, 35517, 35547, 35576,
	35606, 35635, 35665, 35694, 35724, 35753, 35782, 35812, 35841,
	35871, 35901, 35931, 35960, 35990, 36019, 36049, 36078, 36108,
	36137, 36166, 36196, 36225, 36255, 36285, 36314, 36344, 36374,
	36403, 36433, 36462, 36491, 36521, 36550, 36579, 36609, 36639,
	36668, 36698, 36728, 36758, 36787, 36817, 36846, 36875, 36905,
	36934, 36963, 36993, 37023, 37052, 37082, 37112, 37141, 37171,
	37200, 37230, 37259, 37289, 37318, 37348, 37377, 37406, 37436,
	37466, 37495, 37525, 37554, 37584, 37614, 37643, 37673, 37702,
	37732, 37761, 37790, 37820, 37849, 37879, 37908, 37938, 37968,
	37997, 38027, 38057, 38086, 38116, 38145, 38174, 38204, 38233,
	38263, 38292, 38322, 38352, 38381, 38411, 38441, 38470, 38500,
	38529, 38558, 38588, 38617, 38646, 38676, 38706, 38735, 38765,
	38795, 38825, 38854, 38884, 38913, 38942, 38972, 39001, 39030,
	39060, 39090, 39119, 39149, 39179, 39208, 39238, 39268, 39297,
	39326, 39356, 39385, 39414, 39444, 39474, 39503, 39533, 39562,
	39592, 39622, 39651, 39681, 39710, 39740, 39769, 39799, 39828,
	39857, 39887, 39917, 39946, 39976, 40005, 40035, 40065, 40094,
	40124, 40153, 40183, 40212, 40241, 40271, 40300, 40330, 40360,
	40389, 40419, 40449, 40478, 40508, 40538, 40567, 40596, 40626,
	40655, 40684, 40714, 40743, 40773, 40803, 40833, 40863, 40892,
	40922, 40951, 40980, 41010, 41039, 41068, 41098, 41127, 41157,
	41187, 41217, 41246, 41276, 41305, 41335, 41364, 41394, 41423,
	41452, 41482, 41511, 41541, 41571, 41600, 41630, 41659, 41689,
	41719, 41748, 41778, 41807, 41836, 41866, 41895, 41925, 41954,
	41984, 42013, 42043, 42073, 42103, 42132, 42162, 42191, 42220,
	42250, 42280, 42309, 42338, 42367, 42397, 42427, 42457, 42486,
	42516, 42546, 42575, 42604, 42634, 42663, 42692, 42722, 42751,
	42781, 42811, 42840, 42870, 42900, 42929, 42959, 42989, 43018,
	43047, 43076, 43106, 43135, 43165, 43194, 43224, 43254, 43283,
	43313, 43343, 43372, 43402, 43431, 43461, 43490, 43519, 43549,
	43578, 43608, 43638, 43667, 43697, 43727, 43756, 43786, 43815,
	43844, 43874, 43903, 43933, 43962, 43992, 44021, 44051, 44081,
	44110, 44140, 44169, 44199, 44228, 44258, 44287, 44317, 44346,
	44375, 44405, 44434, 44464, 44494, 44523, 44553, 44583, 44612,
	44642, 44671, 44701, 44730, 44760, 44789, 44818, 44848, 44877,
	44907, 44937, 44967, 44996, 45026, 45055, 45085, 45114, 45144,
	45173, 45202, 45231, 45261, 45291, 45321, 45350, 45380, 45410,
	45439, 45469, 45498, 45528, 45557, 45586, 45615, 45645, 45675,
	45704, 45734, 45764, 45794, 45823, 45853, 45882, 45911, 45941,
	45970, 46000, 46029, 46059, 46088, 46118, 46148, 46177, 46207,
	46237, 46266, 46295, 46325, 46354, 46384, 46413, 46442, 46472,
	46502, 46531, 46561, 46591, 46620, 46650, 46679, 46709, 46738,
	46768, 46797, 46827, 46856, 46886, 46915, 46945, 46974, 47004,
	47034, 47063, 47093, 47122, 47152, 47181, 47211, 47240, 47270,
	47299, 47328, 47358, 47388, 47417, 47447, 47477, 47506, 47536,
	47565, 47595, 47624, 47653, 47683, 47712, 47742, 47771, 47801,
	47831, 47860, 47890, 47920, 47949, 47979, 48008, 48037, 48067,
	48096, 48126, 48155, 48185, 48215, 48244, 48274, 48304, 48333,
	48363, 48392, 48422, 48451, 48480, 48509, 48539, 48569, 48598,
	48628, 48658, 48688, 48717, 48747, 48776, 48806, 48835, 48864,
	48893, 48923, 48953, 48982, 49012, 49042, 49071, 49101, 49131,
	49160, 49189, 49219, 49248, 49278, 49307, 49336, 49366, 49396,
	49425, 49455, 49485, 49515, 49544, 49573, 49603, 49632, 49662,
	49691, 49720, 49750, 49780, 49809, 49839, 49869, 49898, 49928,
	49957, 49987, 50016, 50046, 50075, 50104, 50134, 50164, 50193,
	50223, 50252, 50282, 50311, 50341, 50371, 50400, 50430, 50459,
	50489, 50518, 50548, 50578, 50607, 50636, 50666, 50695, 50725,
	50755, 50784, 50814, 50843, 50873, 50902, 50931, 50961, 50990,
	51020, 51049, 51079, 51109, 51138, 51168, 51198, 51227, 51257,
	51286, 51315, 51345, 51374, 51404, 51433, 51463, 51492, 51522,
	51552, 51582, 51611, 51641, 51670, 51700, 51729, 51758, 51787,
	51817, 51846, 51876, 51906, 51936, 51965, 51995, 52025, 52054,
	52084, 52113, 52142, 52171, 52201, 52230, 52260, 52290, 52319,
	52349, 52379, 52409, 52438, 52468, 52497, 52526, 52555, 52585,
	52614, 52644, 52673, 52703, 52733, 52763, 52792, 52822, 52851,
	52881, 52910, 52940, 52969, 52998, 53028, 53057, 53087, 53117,
	53146, 53176, 53206, 53235, 53265, 53294, 53324, 53353, 53382,
	53412, 53441, 53471, 53500, 53530, 53560, 53589, 53619, 53649,
	53678, 53708, 53737, 53767, 53796, 53825, 53855, 53884, 53914,
	53943, 53973, 54003, 54032, 54062, 54092, 54121, 54151, 54180,
	54209, 54239, 54268, 54298, 54327, 54357, 54386, 54416, 54446,
	54476, 54505, 54535, 54564, 54593, 54623, 54652, 54682, 54711,
	54740, 54770, 54800, 54830, 54859, 54889, 54918, 54948, 54977,
	55007, 55036, 55065, 55095, 55124, 55154, 55183, 55213, 55243,
	55273, 55302, 55332, 55361, 55391, 55420, 55449, 55479, 55508,
	55538, 55567, 55597, 55627, 55656, 55686, 55716, 55745, 55775,
	55804, 55833, 55863, 55892, 55922, 55951, 55981, 56010, 56040,
	56070, 56100, 56129, 56159, 56188, 56217, 56247, 56276, 56306,
	56335, 56365, 56394, 56424, 56454, 56483, 56513, 56543, 56572,
	56601, 56631, 56660, 56690, 56719, 56749, 56778, 56808, 56837,
	56867, 56897, 56926, 56956, 56986, 57015, 57045, 57074, 57103,
	57133, 57162, 57192, 57221, 57251, 57281, 57310, 57340, 57369,
	57399, 57429, 57458, 57487, 57517, 57546, 57575, 57605, 57635,
	57664, 57694, 57724, 57753, 57783, 57813, 57842, 57871, 57901,
	57930, 57959, 57989, 58018, 58048, 58078, 58107, 58137, 58167,
	58197, 58226, 58255, 58285, 58314, 58343, 58373, 58402, 58432,
	58461, 58491, 58521, 58551, 58580, 58610, 58640, 58669, 58698,
	58727, 58757, 58786, 58816, 58845, 58875, 58905, 58934, 58964,
	58994, 59023, 59053, 59082, 59111, 59141, 59170, 59200, 59229,
	59259, 59288, 59318, 59348, 59378, 59407, 59437, 59466, 59495,
	59525, 59554, 59584, 59613, 59643, 59672, 59702, 59732, 59761,
	59791, 59820, 59850, 59880, 59909, 59938,
};
