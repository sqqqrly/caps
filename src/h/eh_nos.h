/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Error message numbers.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   7/7/93    |  tjt  Revised error numbers - remove unused
 *-------------------------------------------------------------------------*/
#ifndef EH_NOS_H
#define EH_NOS_H

static char eh_nos_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *    Taken from  src/preproc/eh_nos.h
 *-------------------------------------------------------------------------*/

#define WALL_MSG              1
#define LOG_MSG               2
#define CRASH_MSG             3
#define LOCAL_MSG             4
#define ERR_NUMERIC           5
#define ERR_EVENT             6
#define ERR_ZONE              7
#define ERR_ORDER             8
#define ERR_CODE              9
#define ERR_PL                10
#define ERR_YN                11
#define ERR_SAM               12
#define ERR_SUPER             13
#define ERR_SKU_DUP           14
#define ERR_SKU_INV           15
#define ERR_PM_DUP            16
#define ERR_PM_INV            17
#define ERR_REF               18
#define ERR_TIME              19
#define ERR_PM_DEL            20
#define ERR_SKU_DEL           21
#define ERR_OF_QUE            22
#define ERR_OF_PND            23
#define ERR_OF_ACT            24
#define ERR_OF_COMP           25
#define ERR_INIT              26
#define ERR_CONFIG            27
#define ERR_NO_CONFIG         28
#define ERR_IS_CONFIG         29
#define ERR_PM_SKU            30
#define ERR_CAPS_SKU          31
#define ERR_QTY_INV           32
#define ERR_CAPS_DUP          33
#define ERR_OF_NCOMP          34
#define ERR_OF_PRI            35
#define ERR_RANGE             36
#define ERR_PM_DEA            37
#define ERR_RESTOCK           38
#define ERR_CAPS_PM           39
#define ERR_QTY_PM            40
#define ERR_CAPS_INV          41
#define ERR_QTY_ENABLE        42
#define ERR_NO_PXF            43
#define ERR_MP                46
#define ERR_MP_DONE           47
#define ERR_SD                48
#define ERR_SD_DONE           49
#define ERR_RP                50
#define ERR_RP_DONE           51
#define ERR_PM_DSKU           52
#define ERR_PFF_SRT           53
#define ERR_PFF_ITEM          54
#define ERR_PFF_RNG           55
#define ERR_OF_STATUS         56
#define ERR_ENTER             57
#define ERR_OF_MATCH          58
#define ERR_VALUE             59
#define ERR_OP_NAME           60
#define ERR_PROFILE           61
#define ERR_LEVEL             62
#define ERR_PASSWORD          63
#define ERR_FILE              64
#define ERR_XR_PICK           65
#define ERR_OH                66
#define ERR_OC                67
#define ERR_GH                68
#define ERR_OA                70
#define ERR_OD                71
#define ERR_GA                72
#define ERR_GD                73
#define ERR_HW_LOCK           74
#define ERR_OF_CANCEL         75
#define ERR_OF_HOLD           77
#define ERR_GRP_HOLD          78
#define ERR_FILE_INVALID      79
#define ERR_OUTPUT            80
#define ERR_CHK_CONFIG        81
#define ERR_NO_ORDER          82
#define ERR_BAD_PICK          83
#define ERR_PREFACE           84
#define ERR_PM_MISSING        85
#define ERR_QUAN_MISSING      86
#define ERR_NO_PENDING        87
#define ERR_OI_DONE           90
#define ERR_TO_DONE           91
#define ERR_READY             92
#define ERR_CONFIG_DONE       93
#define ERR_SAME_PRI          94
#define ERR_SQL_WORD          95
#define ERR_PICK              96
#define ERR_DUP_PICK          97
#define ERR_TIMEOUT           98
#define ERR_CONFIRM           99
#define ERR_PICK_RATE         100
#define ERR_PROD              101
#define ERR_NO_VAL_CONFIG     102
#define ERR_OI                103
#define ERR_CONFIG_VALID      104
#define ERR_OI_ABORT          105
#define ERR_BATCH_NO          106
#define ERR_PM_BATCH          107
#define ERR_CAPS_BATCH        108
#define ERR_SKU_BATCH         109
#define ERR_START             110
#define ERR_ED_CONFIG         111
#define ERR_BATCH_NO_PROC     112
#define ERR_CHNG_DSKU         114
#define ERR_ZB                115
#define ERR_REQ_ORDER         116
#define ERR_CHNG_DPM          117
#define ERR_TXCODE            118
#define ERR_REUSE             119
#define ERR_PACK              120
#define ERR_BAD_BATON         121
#define HOST_FAIL_RESP        128
#define HOST_RESP             129
#define ERR_PRNT_LABEL        130
#define CHK_INPROC            131
#define ERR_GRP_CNA           132
#define ERR_GRP_AGNQ          133
#define ERR_GRP_AP            134
#define ERR_GRP_CAN           135
#define ERR_GRP_ABO           137
#define ERR_GRP_NQ            138
#define ERR_GRP_GPA           139
#define ERR_GRP_PRI           140
#define ERR_COMM              141
#define ERR_COMM_BYPASS       142
#define ERR_AFTER_GRP         143
#define ERR_O_IN_GRP          144
#define ERR_NOT_QH            145
#define ERR_GRP_REQ           146
#define GRP_NOT_FND           147
#define ERR_GRP_PC            148
#define ERR_ORD_REQ           149
#define PM_MARK_DEL           150
#define PM_MARK_INS           151
#define SHORT_PRINT           152
#define PURGE_TRANS           153
#define ERR_OPEN              155
#define ERR_READ              156
#define ERR_WRITE             157
#define ERR_DUP_SCAN          158

#endif

/* end of eh_nos.h */
