/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Pickline control state equates.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Added to mfc.
 *  07/16/94   |  tjt  Added Special case picking.
 *  09/08/94   |  tjt  Revised for case picking.
 *-------------------------------------------------------------------------*/
#ifndef PLC_H
#define PLC_H
static char plc_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  State Name Definitions
 *-------------------------------------------------------------------------*/

#define NOOP     0
#define PMIDLE   1
#define PMIDLE0  2
#define UXNX1    3
#define UXNX2    4
#define UXNX3    5
#define UXNX4    6
#define UXNX5    7
#define UXSX1    8
#define UXSX2    9
#define UXSX3    10
#define UXSX4    11
#define UXSX5    12
#define UXSX6    13
#define UXSX7    14
#define HXNX1    15
#define HXNX2    16
#define HXNX3    17
#define HXNX4    18
#define HXNX5    19
#define HXNX6    20
#define HXNX7    21
#define HXNX8    22
#define HXZX1    23
#define HXZX2    24
#define HXZX3    25
#define HXZX4    26
#define HXZX5    27
#define HXZX6    28
#define HXZX7    29
#define HXZX8    30
#define HXZX9    31
#define HXZX10   32
#define HXZX11   33
#define HXZX12   34
#define HXSX1    35
#define HXSX2    36
#define HXSX3    37
#define HXSX4    38
#define HXSX5    39
#define HXSX6    40
#define HXSX7    41
#define HXSX8    42
#define HXSX9    43
#define HXSX10   44
#define HXSX11   45
#define HXSX12   46
#define HXSX13   47
#define HXSX14   48
#define HXSX15   49
#define HXSX16   50
#define HXSX17   51
#define HXSX18   52
#define HXSX19   53
#define HXSX20   54
#define HXSX21   55
#define UXCC0    56
#define UXCC1    57
#define UXCC2    58
#define UXCC3    59
#define UXCC4    60
#define UXCC5    61
#define UXCC6    62
#define UXCC7    63
#define UXCC10   64
#define UXCC11   65
#define UXCC12   66
#define UXCC13   67
#define UXCS0    68
#define UXCS1    69
#define UXCS2    70
#define UXCS3    71
#define UXCS4    72
#define UXCS5    73
#define UXCS6    74
#define UXCS7    75
#define UXCS8    76
#define UXCS9    77
#define UXCS10   78
#define UXCS11   79
#define UXCS14   80
#define UXCS15   81
#define UXCS16   82
#define UXCS17   83
#define UXCS18   84
#define PMT0     85
#define PMT0A    86
#define PMT1     87
#define PMT1A    88
#define PMT2     89
#define PMT2A    90
#define PMT3     91
#define PMT3A    92
#define PMT4     93
#define PMT4A    94
#define BLOFF    95
#define BLOFF1   96
#define BLON     97
#define BLON1    98
#define ZCOMMO0  99
#define ZCOMMO1  100
#define ZCOMMO2  101
#define ZCIDLE0  102
#define ZCIDLE1  103
#define ZCIDLE2  104
#define ZCWAIT0  105
#define ZCWAIT1  106
#define ZCAHD0   107
#define ZCAHD1   108
#define ZCAHD2   109
#define ZCDONE0  110
#define ZCDONE1  111
#define ZCDONE2  112
#define ZCLOCK0  113
#define ZCLOCK1  114
#define ZCLOCK2  115
#define ZCDXM1   116
#define ZCDXM2   117
#define ZCDXM3   118
#define ZCEXM1   119
#define ZCEXM2   120
#define ZCEXM3   121
#define ZCNOP1   122
#define ZCNOP2   123
#define ZCNOP3   124
#define EEDXM    125
#define EEDXM2   126
#define EEEXM    127
#define EEEXM2   128
#define EEEXM3   129
#define EENOP1   130
#define EENOP2   131
#define EENOP3   132
#define LEE1     133
#define LEE2     134
#define LEE3     135
#define ZCT0     136
#define ZCT0A    137
#define ZCT1     138
#define ZCT1A    139
#define ZCT2     140
#define ZCT2A    141
#define ZCT3     142
#define ZCT3A    143
#define ZCT4     144
#define ZCT4A    145
#define ZCT6     146
#define ZCT7     147
#define PMBOX1   148
#define PMBOX2   149
#define ZCBOX1   150
#define ZCBOX2   151

#endif

/* end of plc.h */
