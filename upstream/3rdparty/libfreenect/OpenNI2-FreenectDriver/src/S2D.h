const uint16_t S2D[] = {
0, 315, 315, 315, 316, 316, 316, 316, 317, 317, 
317, 318, 318, 318, 319, 319, 319, 319, 320, 320, 
320, 321, 321, 321, 322, 322, 322, 322, 323, 323, 
323, 324, 324, 324, 325, 325, 325, 326, 326, 326, 
326, 327, 327, 327, 328, 328, 328, 329, 329, 329, 
330, 330, 330, 331, 331, 331, 332, 332, 332, 332, 
333, 333, 333, 334, 334, 334, 335, 335, 335, 336, 
336, 336, 337, 337, 337, 338, 338, 338, 339, 339, 
339, 340, 340, 340, 341, 341, 341, 342, 342, 343, 
343, 343, 344, 344, 344, 345, 345, 345, 346, 346, 
346, 347, 347, 347, 348, 348, 348, 349, 349, 350, 
350, 350, 351, 351, 351, 352, 352, 352, 353, 353, 
354, 354, 354, 355, 355, 355, 356, 356, 356, 357, 
357, 358, 358, 358, 359, 359, 359, 360, 360, 361, 
361, 361, 362, 362, 363, 363, 363, 364, 364, 364, 
365, 365, 366, 366, 366, 367, 367, 368, 368, 368, 
369, 369, 370, 370, 370, 371, 371, 372, 372, 372, 
373, 373, 374, 374, 374, 375, 375, 376, 376, 377, 
377, 377, 378, 378, 379, 379, 379, 380, 380, 381, 
381, 382, 382, 382, 383, 383, 384, 384, 385, 385, 
385, 386, 386, 387, 387, 388, 388, 389, 389, 389, 
390, 390, 391, 391, 392, 392, 393, 393, 393, 394, 
394, 395, 395, 396, 396, 397, 397, 398, 398, 398, 
399, 399, 400, 400, 401, 401, 402, 402, 403, 403, 
404, 404, 405, 405, 406, 406, 407, 407, 408, 408, 
409, 409, 409, 410, 410, 411, 411, 412, 412, 413, 
413, 414, 414, 415, 415, 416, 416, 417, 418, 418, 
419, 419, 420, 420, 421, 421, 422, 422, 423, 423, 
424, 424, 425, 425, 426, 426, 427, 427, 428, 429, 
429, 430, 430, 431, 431, 432, 432, 433, 433, 434, 
435, 435, 436, 436, 437, 437, 438, 438, 439, 440, 
440, 441, 441, 442, 442, 443, 444, 444, 445, 445, 
446, 446, 447, 448, 448, 449, 449, 450, 451, 451, 
452, 452, 453, 454, 454, 455, 455, 456, 457, 457, 
458, 458, 459, 460, 460, 461, 462, 462, 463, 463, 
464, 465, 465, 466, 467, 467, 468, 468, 469, 470, 
470, 471, 472, 472, 473, 474, 474, 475, 476, 476, 
477, 478, 478, 479, 480, 480, 481, 482, 482, 483, 
484, 484, 485, 486, 487, 487, 488, 489, 489, 490, 
491, 491, 492, 493, 494, 494, 495, 496, 496, 497, 
498, 499, 499, 500, 501, 502, 502, 503, 504, 504, 
505, 506, 507, 507, 508, 509, 510, 511, 511, 512, 
513, 514, 514, 515, 516, 517, 517, 518, 519, 520, 
521, 521, 522, 523, 524, 525, 525, 526, 527, 528, 
529, 529, 530, 531, 532, 533, 534, 534, 535, 536, 
537, 538, 539, 540, 540, 541, 542, 543, 544, 545, 
546, 546, 547, 548, 549, 550, 551, 552, 553, 554, 
554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 
564, 565, 565, 566, 567, 568, 569, 570, 571, 572, 
573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 
583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 
593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 
603, 604, 606, 607, 608, 609, 610, 611, 612, 613, 
614, 615, 616, 618, 619, 620, 621, 622, 623, 624, 
625, 627, 628, 629, 630, 631, 632, 634, 635, 636, 
637, 638, 640, 641, 642, 643, 644, 646, 647, 648, 
649, 650, 652, 653, 654, 655, 657, 658, 659, 661, 
662, 663, 664, 666, 667, 668, 670, 671, 672, 674, 
675, 676, 678, 679, 680, 682, 683, 684, 686, 687, 
688, 690, 691, 693, 694, 696, 697, 698, 700, 701, 
703, 704, 706, 707, 708, 710, 711, 713, 714, 716, 
717, 719, 720, 722, 723, 725, 727, 728, 730, 731, 
733, 734, 736, 738, 739, 741, 742, 744, 746, 747, 
749, 750, 752, 754, 755, 757, 759, 761, 762, 764, 
766, 767, 769, 771, 773, 774, 776, 778, 780, 781, 
783, 785, 787, 789, 790, 792, 794, 796, 798, 800, 
802, 803, 805, 807, 809, 811, 813, 815, 817, 819, 
821, 823, 825, 827, 829, 831, 833, 835, 837, 839, 
841, 843, 845, 847, 849, 851, 854, 856, 858, 860, 
862, 864, 867, 869, 871, 873, 875, 878, 880, 882, 
885, 887, 889, 891, 894, 896, 898, 901, 903, 906, 
908, 910, 913, 915, 918, 920, 923, 925, 928, 930, 
933, 935, 938, 940, 943, 946, 948, 951, 954, 956, 
959, 962, 964, 967, 970, 973, 975, 978, 981, 984, 
987, 989, 992, 995, 998, 1001, 1004, 1007, 1010, 1013, 
1016, 1019, 1022, 1025, 1028, 1031, 1034, 1038, 1041, 1044, 
1047, 1050, 1054, 1057, 1060, 1063, 1067, 1070, 1073, 1077, 
1080, 1084, 1087, 1090, 1094, 1097, 1101, 1105, 1108, 1112, 
1115, 1119, 1123, 1126, 1130, 1134, 1138, 1141, 1145, 1149, 
1153, 1157, 1161, 1165, 1169, 1173, 1177, 1181, 1185, 1189, 
1193, 1197, 1202, 1206, 1210, 1214, 1219, 1223, 1227, 1232, 
1236, 1241, 1245, 1250, 1255, 1259, 1264, 1268, 1273, 1278, 
1283, 1288, 1292, 1297, 1302, 1307, 1312, 1317, 1322, 1328, 
1333, 1338, 1343, 1349, 1354, 1359, 1365, 1370, 1376, 1381, 
1387, 1392, 1398, 1404, 1410, 1415, 1421, 1427, 1433, 1439, 
1445, 1452, 1458, 1464, 1470, 1477, 1483, 1489, 1496, 1503, 
1509, 1516, 1523, 1529, 1536, 1543, 1550, 1557, 1564, 1572, 
1579, 1586, 1594, 1601, 1609, 1616, 1624, 1632, 1639, 1647, 
1655, 1663, 1671, 1680, 1688, 1696, 1705, 1713, 1722, 1731, 
1739, 1748, 1757, 1766, 1776, 1785, 1794, 1804, 1813, 1823, 
1833, 1843, 1853, 1863, 1873, 1883, 1894, 1904, 1915, 1926, 
1936, 1947, 1959, 1970, 1981, 1993, 2005, 2016, 2028, 2040, 
2053, 2065, 2078, 2090, 2103, 2116, 2129, 2143, 2156, 2170, 
2184, 2198, 2212, 2226, 2241, 2256, 2271, 2286, 2301, 2317, 
2333, 2349, 2365, 2381, 2398, 2415, 2432, 2450, 2467, 2485, 
2503, 2522, 2541, 2560, 2579, 2598, 2618, 2639, 2659, 2680, 
2701, 2723, 2744, 2767, 2789, 2812, 2835, 2859, 2883, 2908, 
2933, 2958, 2984, 3010, 3037, 3064, 3092, 3120, 3149, 3178, 
3208, 3238, 3269, 3300, 3333, 3365, 3399, 3433, 3468, 3503, 
3539, 3576, 3614, 3653, 3692, 3732, 3774, 3816, 3859, 3903, 
3948, 3994, 4041, 4089, 4139, 4190, 4241, 4295, 4349, 4405, 
4463, 4522, 4582, 4645, 4708, 4774, 4842, 4911, 4983, 5056, 
5132, 5210, 5291, 5374, 5460, 5548, 5640, 5734, 5832, 5933, 
6038, 6146, 6259, 6375, 6497, 6622, 6753, 6889, 7030, 7178, 
7332, 7492, 7660, 7835, 8019, 8212, 8413, 8626, 8849, 9084, 
9331, 9593, 9870, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0};

