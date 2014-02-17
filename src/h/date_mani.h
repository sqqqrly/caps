
#define   DT_DAY      0x0010
#define   DT_MONTH    0x0020
#define   DT_YEAR     0x0040
#define   DT_NULL     0x0080
#define   DT_SCI_IN   0x0100

#define   NULL_S_VAL   -0x8000
#define   NULL_L_VAL   -0x80000000L
#define   NULL_D_VAL   -DBL_MAX

#define   DATE_SIZE      11
#define   DATETIME_SIZE  20

typedef struct date
{
    int  year;
    int  month;
    int  day;
}DATE_STRUCT;

int iSprintfDate(char *dt, DATE_STRUCT *dateval);
int iScanDate(char *dt, DATE_STRUCT *dateval);
int iInitializeDate(DATE_STRUCT *dateval);
