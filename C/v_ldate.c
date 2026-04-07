////////////////////////////////////////////////////////////////////////////////
//
// システム名：横断集計
//
// プログラム名：v_ldate
//
// 機能概要：
//  引数により指定した日付または、週NOや月から外部ファイルの内容を標準出力に書き出す。
//  オプションの内容で出力内容の切り替えを行う。
//  指定した加算数により日付/週/月の加算・減算を行う。
//  -<ファイル入力オプション>を指定する事により、入力ファイルの指定フィールドからオプション指示の処理を行い、
//  ファイル編集をして標準出力に書き出す。
//
// 作成日：2014/12/02
//
// 作成者：Hirata
//
// Ｃ言語規格：C99
//
// ソース記述形式：indent -kr -nut -l90
//
// 修正履歴：
//   yyyy/mm/dd K.Shimadaxxxxxx
//     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
//     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
//
////////////////////////////////////////////////////////////////////////////////
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "v_common.h"

#define ENV_VAL_NAME_MD_WEEK_FILE_PATH "MDWEEK_FILE"

#define MAX_MD_WEEK_FILE_LINE_LENGTH 4096
#define BUFFER_LENGTH 512

#define BASE_NUMBER 10

#define OPTION_PREFIX_CHAR '-'
#define OPTION_WEEK "-w"
#define OPTION_DAY "-y"
#define OPTION_RANGE "-e"
#define OPTION_FILE "-f"

#define DATE_STRING_TODAY "today"
#define DATE_STRING_YESTERDAY "yday"

#define MAX_DAYS_IN_MONTH 31
#define DATE_STRING_FORMAT "%Y%m%d"
#define DATE_STRING_LENGTH 8
#define DAY_NUMBER_STRING_LENGTH 1
#define WEEK_NUMBER_STRING_LENGTH 6
#define MONTH_STRING_LENGTH 6

#define MARK_OF_WEEK_NUMBER 'w'
#define MARK_OF_MONTH 'm'

#define DAY_NUMBER_INVALID -1
#define DATE_INDEX_INVALID -1

#define ADD_NUMBER_DELIMITER "/"

#define get_day_number_char(day_number) ('0' + day_number)
enum md_week_file_field {
        MD_WEEK_FILE_FIELD_WEEK_NUMBER_THIS_YEAR
    ,   MD_WEEK_FILE_FIELD_MONDAY
    ,   MD_WEEK_FILE_FIELD_TUESDAY
    ,   MD_WEEK_FILE_FIELD_WEDNESDAY
    ,   MD_WEEK_FILE_FIELD_THURSDAY
    ,   MD_WEEK_FILE_FIELD_FRIDAY
    ,   MD_WEEK_FILE_FIELD_SATURDAY
    ,   MD_WEEK_FILE_FIELD_SUNDAY
    ,   MD_WEEK_FILE_FIELD_WEEK_NUMBER_LAST_YEAR
};

#define FIELD_NUMBER_INVALID -1

#define MD_WEEK_FILE_DELIMITER " "
#define INPUT_FILE_DELIMITER_CHAR ' '
#define OUTPUT_DELIMITER " "
#define MD_WEEK_FILE_FIELD_COUNT 9
#define MD_WEEK_FILE_MAX_FIELD_LENGTH 9

typedef struct
{
    char option[3];
    int day_number;
    char start_date[DATE_STRING_LENGTH + 1];
    char start_week_number[WEEK_NUMBER_STRING_LENGTH + 1];
    char start_month[MONTH_STRING_LENGTH + 1];
    char start_month2[MONTH_STRING_LENGTH + 1];
    long field_number;
    bool week_number_field_flag;
    long add_number;
    bool set_add_number_flag;
    FILE *input_file_pointer;
} arg_struct;

void create_md_week_table();
void add_to_md_date_index(int base_row, int base_column, int add_number, int *index_array);
void get_md_date_index(char *base_date, int *index_array);
char *get_md_date(int row, int column);
char *add_md_date(char *base_date, int add_number);
int get_md_week_number_row_index(char *week_number);
char *get_md_week_number(int row);
int get_md_month_last_row_index(char *month);
void get_md_month_first_and_last_index(char *month, int *first_index_array, int *last_index_array);
void analyze_arg(int argc, char **argv, arg_struct *args);

void output(arg_struct *args);
void output_from_arg(arg_struct *args);
void output_from_input_file(arg_struct *args);
void output_from_input_file_field(arg_struct *args, char *field, long line_count, char *output_buf);
char *get_md_date_and_day_number_and_md_week_number_string(char *date);
char *get_day_number_string(char *date);
char *get_md_week_number_string_by_md_date(char *date);
char *get_md_date_string(char *base_date, int add_number);
char *get_md_date_range_string(char *start_date, int add_number);
char *get_one_md_week_dates_string(char *week_number);
char *get_md_date_string_by_md_week_number_and_day_number(char *week_number, int day_number);
char *get_md_week_number_string(char *base_week_number, int add_number);
char *get_md_week_number_range_string(char *start_week_number, int add_number);
char *get_one_md_month_dates_string(char *month);
char *get_md_month_string(char *base_month, int add_number);
char *get_md_month_diff_string(char *start_month, char *end_month);

int md_week_file_line_count;
char ***md_week_table;
char **md_week_table_base_pointer_1;
char *md_week_table_base_pointer_2;

int main(int argc, char **argv)
{
    if (argc < 2 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "--version")) {
        msgerr2("引数は指定されていません。\n");
        exit(EXIT_FAILURE);
    }

    create_md_week_table();

    arg_struct args;
    analyze_arg(argc, argv, &args);

    output(&args);

    return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// 外部ファイルを読み込んでMD週テーブルを作成する
////////////////////////////////////////////////////////////////////////////////
void create_md_week_table()
{
    char *md_week_file_path = getenv(ENV_VAL_NAME_MD_WEEK_FILE_PATH);

    md_week_file_line_count = get_file_line_count(md_week_file_path, MAX_MD_WEEK_FILE_LINE_LENGTH);

    FILE *fp;
    if (md_week_file_line_count == 0 || (fp = fopen(md_week_file_path, "r")) == NULL)  {
        msgerr("日付テーブルファイルが開けません。%s\n", md_week_file_path);
        exit(EXIT_FAILURE);
    }

    md_week_table = malloc(sizeof(char **) * md_week_file_line_count);
    md_week_table_base_pointer_1 = malloc(sizeof(char *) * md_week_file_line_count * MD_WEEK_FILE_FIELD_COUNT);
    md_week_table_base_pointer_2 = malloc(sizeof(char) * md_week_file_line_count * MD_WEEK_FILE_FIELD_COUNT * MD_WEEK_FILE_MAX_FIELD_LENGTH);
    for (int i = 0; i < md_week_file_line_count; i++) {
        md_week_table[i] = md_week_table_base_pointer_1 + (i * MD_WEEK_FILE_FIELD_COUNT);
        for (int j = 0; j < MD_WEEK_FILE_FIELD_COUNT; j++) {
            md_week_table[i][j] = md_week_table_base_pointer_2 + (i * MD_WEEK_FILE_FIELD_COUNT * MD_WEEK_FILE_MAX_FIELD_LENGTH) + j * MD_WEEK_FILE_MAX_FIELD_LENGTH;
        }
    }
    char tmp_line_string[MAX_MD_WEEK_FILE_LINE_LENGTH];
    char *tmp_field_array[MD_WEEK_FILE_FIELD_COUNT];
    int tmp_line_number = 0;
    while (fgets(tmp_line_string, sizeof(tmp_line_string), fp) != NULL) {
        strtok(tmp_line_string, "\n");
        int tmp_field_count = split(tmp_line_string, MD_WEEK_FILE_DELIMITER, tmp_field_array, MD_WEEK_FILE_FIELD_COUNT);
        for (int i = 0; i < tmp_field_count; i++) {
            strcpy(md_week_table[tmp_line_number][i], tmp_field_array[i]);
        }
        tmp_line_number++;
    }
    fclose(fp);
}

////////////////////////////////////////////////////////////////////////////////
// MD週テーブルのインデックスを加算
////////////////////////////////////////////////////////////////////////////////
void add_to_md_date_index(int base_row, int base_column, int add_number, int *index_array)
{
    index_array[0] = DATE_INDEX_INVALID;
    index_array[1] = DATE_INDEX_INVALID;
    if (base_row != DATE_INDEX_INVALID && base_column != DATE_INDEX_INVALID) {
        int result_row = base_row;
        int result_col = base_column + add_number;
        while (result_row >= 0 && result_col <= MD_WEEK_FILE_FIELD_WEEK_NUMBER_THIS_YEAR) {
            result_row--;
            result_col += (MD_WEEK_FILE_FIELD_WEEK_NUMBER_LAST_YEAR - 1);
        }
        while (result_row < md_week_file_line_count && result_col >= MD_WEEK_FILE_FIELD_WEEK_NUMBER_LAST_YEAR) {
            result_row++;
            result_col -= (MD_WEEK_FILE_FIELD_WEEK_NUMBER_LAST_YEAR - 1);
        }
        if (result_row >= 0 && result_row < md_week_file_line_count) {
            index_array[0] = result_row;
            index_array[1] = result_col;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// MD週テーブルから日付のインデックスを取得
////////////////////////////////////////////////////////////////////////////////
void get_md_date_index(char *base_date, int *index_array)
{
    index_array[0] = DATE_INDEX_INVALID;
    index_array[1] = DATE_INDEX_INVALID;

    static char prev_base_date[DATE_STRING_LENGTH + 1] = "";
    static int prev_index_array[2];
    if (strcmp(base_date, prev_base_date) == 0) {
        index_array[0] = prev_index_array[0];
        index_array[1] = prev_index_array[1];
    } else if (strlen(base_date) == DATE_STRING_LENGTH) {
        for (int i = md_week_file_line_count - 1; i >= 0; i--) {
            for (int j = MD_WEEK_FILE_FIELD_SUNDAY; j >= MD_WEEK_FILE_FIELD_MONDAY; j--) {
                if (strcmp(base_date, get_md_date(i, j)) == 0) {
                    index_array[0] = i;
                    index_array[1] = j;
                    return;
                }
            }
        }
        strcpy(prev_base_date, base_date);
        prev_index_array[0] = index_array[0];
        prev_index_array[1] = index_array[1];
    }
}

////////////////////////////////////////////////////////////////////////////////
// MD週テーブルから引数のMD週NOの行のインデックスを取得
////////////////////////////////////////////////////////////////////////////////
int get_md_week_number_row_index(char *week_number)
{
    int result_index = DATE_INDEX_INVALID;

    static char prev_week_number[WEEK_NUMBER_STRING_LENGTH + 1] = "";
    static int prev_result_index = DATE_INDEX_INVALID;
    if (strcmp(week_number, prev_week_number) == 0) {
        result_index = prev_result_index;
    } else if (strlen(week_number) == WEEK_NUMBER_STRING_LENGTH) {
        for (int i = md_week_file_line_count - 1; i >= 0; i--) {
            if (strcmp(week_number, get_md_date(i, MD_WEEK_FILE_FIELD_WEEK_NUMBER_THIS_YEAR)) == 0) {
                result_index = i;
                break;
            }
        }
        strcpy(prev_week_number, week_number);
        prev_result_index = result_index;
    }

    return result_index;
}

////////////////////////////////////////////////////////////////////////////////
// MD週テーブルから引数の行のMD週NOを取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_week_number(int row)
{
    char *result_week_number = NULL;

    if (row >= 0 && row < md_week_file_line_count) {
        result_week_number = md_week_table[row][MD_WEEK_FILE_FIELD_WEEK_NUMBER_THIS_YEAR];
    }

    return result_week_number;
}

////////////////////////////////////////////////////////////////////////////////
// MD週NOを加算
////////////////////////////////////////////////////////////////////////////////
char *add_md_week_number(char *base_week_number, int add_number)
{
    char *result_week_number = NULL;

    int base_row = get_md_week_number_row_index(base_week_number);
    if (base_row != DATE_INDEX_INVALID) {
        result_week_number = get_md_week_number(base_row + add_number);
    }

    return result_week_number;
}

////////////////////////////////////////////////////////////////////////////////
// MD週テーブルから引数の月の最後の行のインデックスを取得
////////////////////////////////////////////////////////////////////////////////
int get_md_month_last_row_index(char *month)
{
    int result_row = DATE_INDEX_INVALID;

    static char prev_month[MONTH_STRING_LENGTH + 1] = "";
    static int prev_result_row = DATE_INDEX_INVALID;
    if (strcmp(month, prev_month) == 0) {
        result_row = prev_result_row;
    } else if (strlen(month) == MONTH_STRING_LENGTH) {
        if (strlen(month) == MONTH_STRING_LENGTH) {
            for (int i = md_week_file_line_count - 1; i >= 0; i--) {
                if (strncmp(month, get_md_date(i, MD_WEEK_FILE_FIELD_MONDAY), MONTH_STRING_LENGTH) == 0) {
                    result_row = i;
                    break;
                }
            }
        }
        strcpy(prev_month, month);
        prev_result_row = result_row;
    }

    return result_row;
}

////////////////////////////////////////////////////////////////////////////////
// MD週テーブルから引数の月の最初と最後のインデックスを取得
////////////////////////////////////////////////////////////////////////////////
void get_md_month_first_and_last_index(char *month, int *first_index_array, int *last_index_array)
{
    first_index_array[0] = DATE_INDEX_INVALID;
    first_index_array[1] = DATE_INDEX_INVALID;
    last_index_array[0] = DATE_INDEX_INVALID;
    last_index_array[1] = DATE_INDEX_INVALID;

    static char prev_month[MONTH_STRING_LENGTH + 1] = "";
    static int prev_first_index_array[2], prev_last_index_array[2];
    if (strcmp(month, prev_month) == 0) {
        first_index_array[0] = prev_first_index_array[0];
        first_index_array[1] = prev_first_index_array[1];
        last_index_array[0] = prev_last_index_array[0];
        last_index_array[1] = prev_last_index_array[1];
    } else if (strlen(month) == MONTH_STRING_LENGTH) {
        for (int i = md_week_file_line_count - 1; i >= 0; i--) {
            for (int j = MD_WEEK_FILE_FIELD_SUNDAY; j >= MD_WEEK_FILE_FIELD_MONDAY; j--) {
                if (strncmp(month, get_md_date(i, j), MONTH_STRING_LENGTH) == 0) {
                    first_index_array[0] = i;
                    first_index_array[1] = j;
                    if (last_index_array[0] == DATE_INDEX_INVALID) {
                        last_index_array[0] = first_index_array[0];
                        last_index_array[1] = first_index_array[1];
                    }
                } else if (first_index_array[0] != DATE_INDEX_INVALID) {
                    break;
                }
            }
        }
        strcpy(prev_month, month);
        prev_first_index_array[0] = first_index_array[0];
        prev_first_index_array[1] = first_index_array[1];
        prev_last_index_array[0] = last_index_array[0];
        prev_last_index_array[1] = last_index_array[1];
    }
}

////////////////////////////////////////////////////////////////////////////////
// MD週テーブルから日付を取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_date(int row, int column)
{
    char *result_date = NULL;

    if (row != DATE_INDEX_INVALID && column != DATE_INDEX_INVALID) {
        result_date = md_week_table[row][column];
    }

    return result_date;
}

////////////////////////////////////////////////////////////////////////////////
// MD日付を加算
////////////////////////////////////////////////////////////////////////////////
char *add_md_date(char *base_date, int add_number)
{
    char *result_date = NULL;

    static char prev_base_date[DATE_STRING_LENGTH + 1] = "";
    static int prev_add_number = 0;
    static char *prev_result_date = NULL;
    if (add_number == prev_add_number && strcmp(base_date, prev_base_date) == 0) {
        result_date = prev_result_date;
    } else if (strlen(base_date) == DATE_STRING_LENGTH) {
        int base_date_index_array[2], date_index_array[2];
        get_md_date_index(base_date, base_date_index_array);
        add_to_md_date_index(base_date_index_array[0], base_date_index_array[1], add_number, date_index_array);

        if (date_index_array[0] != DATE_INDEX_INVALID && date_index_array[1] != DATE_INDEX_INVALID) {
            result_date = get_md_date(date_index_array[0], date_index_array[1]);
        }
        strcpy(prev_base_date, base_date);
        prev_add_number = add_number;
        prev_result_date = result_date;
    }

    return result_date;
}

////////////////////////////////////////////////////////////////////////////////
// 引数の解析
////////////////////////////////////////////////////////////////////////////////
void analyze_arg(int argc, char **argv, arg_struct *args)
{
    strcpy(args->option, "");
    args->day_number = DAY_NUMBER_INVALID;
    strcpy(args->start_date, "");
    strcpy(args->start_week_number, "");
    strcpy(args->start_month, "");
    strcpy(args->start_month2, "");
    args->field_number = FIELD_NUMBER_INVALID;
    args->week_number_field_flag = false;
    args->add_number = 0;
    args->set_add_number_flag = false;
    args->input_file_pointer = NULL;

    bool err_flag = false;
    bool set_err_msg_flag = false;
    int arg_count = argc;

    char system_date[DATE_STRING_LENGTH + 1];
    set_system_date(system_date, DATE_STRING_LENGTH + 1, DATE_STRING_FORMAT);

    if (arg_count >= 3) {
        if (strcmp(argv[arg_count - 2], OPTION_FILE) == 0) {
            if (argv[arg_count - 1][0] == OPTION_PREFIX_CHAR) {
                args->input_file_pointer = stdin;
            } else {
                if ((args->input_file_pointer = fopen(argv[arg_count - 1], "r")) == NULL) {
                    args->input_file_pointer = stdin;
                }
            }
            arg_count -= 2;
        } else if (strcmp(argv[arg_count - 1], OPTION_FILE) == 0) {
            args->input_file_pointer = stdin;
            arg_count--;
        }
    }
    bool input_file_open_flag = is_file_open(args->input_file_pointer);

    for (int i = 1; i < arg_count; i++) {
        int tmp_arg_len = strlen(argv[i]);
        char tmp_arg[tmp_arg_len + 1];
        strcpy(tmp_arg, argv[i]);

        if (i == 1 && tmp_arg[0] == OPTION_PREFIX_CHAR) {
            if (
                tmp_arg_len == 2
            &&  (
                    strncmp(tmp_arg, OPTION_WEEK, 2) == 0
                ||  strncmp(tmp_arg, OPTION_DAY, 2) == 0
                ||  strncmp(tmp_arg, OPTION_RANGE, 2) == 0
                )
            ) {
                strcpy(args->option, tmp_arg);
            } else if (
                tmp_arg_len == 3
            &&  strncmp(tmp_arg, OPTION_DAY, 2) == 0
            &&  tmp_arg[2] >= get_day_number_char(MD_WEEK_FILE_FIELD_MONDAY)
            &&  tmp_arg[2] <= get_day_number_char(MD_WEEK_FILE_FIELD_SUNDAY)
            ) {
                strncpy_and_add_null_char(args->option, tmp_arg, 2);
                args->day_number = tmp_arg[2] - (get_day_number_char(MD_WEEK_FILE_FIELD_MONDAY) - 1);
            } else {
                err_flag = true;
                break;
            }
        } else if (
                strlen(args->start_date) == 0
            &&  strlen(args->start_week_number) == 0
            &&  strlen(args->start_month) == 0
            &&  args->field_number == FIELD_NUMBER_INVALID
        ) {
            char *tmp_array[2];
            int tmp_array_count = split(tmp_arg, ADD_NUMBER_DELIMITER, tmp_array, 2);
            char *not_numeric_string;
            char *tmp_date_string = tmp_array[0];
            int tmp_date_string_len = strlen(tmp_date_string);

            bool named_date_flag = (strcmp(tmp_date_string, DATE_STRING_TODAY) == 0 || strcmp(tmp_date_string, DATE_STRING_YESTERDAY) == 0);

            if (named_date_flag || tmp_date_string_len >= WEEK_NUMBER_STRING_LENGTH) {
                if (named_date_flag) {
                    // 当日・前日
                    int tmp_add_number = 0;
                    if (strcmp(tmp_date_string, DATE_STRING_YESTERDAY) == 0) {
                        tmp_add_number = -1;
                    }
                    char *tmp_start_date = add_md_date(system_date, tmp_add_number);
                    if (tmp_start_date != NULL) {
                        strcpy(args->start_date, tmp_start_date);
                    } else {
                        msgerr("日付テーブルに該当日付なし。%s\n", tmp_date_string);
                        set_err_msg_flag = true;
                        err_flag = true;
                        break;
                    }
                } else if (tmp_date_string_len == WEEK_NUMBER_STRING_LENGTH || tmp_date_string[tmp_date_string_len - 1] == MARK_OF_WEEK_NUMBER) {
                    // 週指定
                    strncpy_and_add_null_char(args->start_week_number, tmp_date_string, WEEK_NUMBER_STRING_LENGTH);
                    if (get_md_week_number_row_index(args->start_week_number) == DATE_INDEX_INVALID) {
                        msgerr("日付テーブルに該当週NOなし。%s\n", tmp_date_string);
                        set_err_msg_flag = true;
                        err_flag = true;
                        break;
                    }
                } else if (tmp_date_string[tmp_date_string_len - 1] == MARK_OF_MONTH) {
                    // 月指定
                    strncpy_and_add_null_char(args->start_month, tmp_date_string, MONTH_STRING_LENGTH);
                    if (get_md_month_last_row_index(args->start_month) == DATE_INDEX_INVALID) {
                        msgerr("日付テーブルに該当月なし。%s\n", tmp_date_string);
                        set_err_msg_flag = true;
                        err_flag = true;
                        break;
                    }
                } else if (tmp_date_string_len == DATE_STRING_LENGTH) {
                    // その他（日付）
                    if (add_md_date(tmp_date_string, 0) == NULL) {
                        err_flag = true;
                        break;
                    } else {
                        strcpy(args->start_date, tmp_date_string);
                    }
                } else {
                    err_flag = true;
                    break;
                }
            } else {
                if (!input_file_open_flag) {
                    // ファイル指定オプション無しでフィールド番号を指定された場合
                    args->input_file_pointer = stdin;
                    input_file_open_flag = true;
                }
                if (tmp_date_string[tmp_date_string_len - 1] == MARK_OF_WEEK_NUMBER) {
                    args->week_number_field_flag = true;
                    tmp_date_string[tmp_date_string_len - 1] = '\0';
                }
                args->field_number = strtol(tmp_date_string, &not_numeric_string, BASE_NUMBER);
                if (args->field_number <= 0 || *not_numeric_string) {
                    err_flag = true;
                    break;
                }
            }
            if (!err_flag && tmp_array_count > 1) {
                args->add_number = strtol(tmp_array[1], &not_numeric_string, BASE_NUMBER);
                if (*not_numeric_string) {
                    msgerr("加算数が数値でありません。 %s\n", tmp_array[1]);
                    set_err_msg_flag = true;
                    err_flag = true;
                    break;
                } else if (args->add_number == 0 && input_file_open_flag) {
                    msgerr2("加算数に0の指定はできません。\n");
                    set_err_msg_flag = true;
                    err_flag = true;
                } else {
                    args->set_add_number_flag = true;
                }
            }
        } else if (
            strlen(args->option) == 0
        &&  strlen(args->start_month) > 0
        &&  strlen(args->start_month2) == 0
        &&  tmp_arg_len == MONTH_STRING_LENGTH + 1
        &&  tmp_arg[MONTH_STRING_LENGTH] == MARK_OF_MONTH
        ) {
            strncpy_and_add_null_char(args->start_month2, tmp_arg, MONTH_STRING_LENGTH);
            if (get_md_month_last_row_index(args->start_month2) == DATE_INDEX_INVALID) {
                msgerr("日付テーブルに該当月なし。%s\n", tmp_arg);
                set_err_msg_flag = true;
                err_flag = true;
                break;
            }
        } if (strcmp(tmp_arg, OPTION_FILE) == 0) {
            err_flag = true;
            break;
        }
    }

    if (!err_flag) {
        msgdbg("option                 : %s\n", args->option);
        msgdbg("day_number             : %d\n", args->day_number);
        msgdbg("start_date             : %s\n", args->start_date);
        msgdbg("start_week_number      : %s\n", args->start_week_number);
        msgdbg("start_month            : %s\n", args->start_month);
        msgdbg("start_month2           : %s\n", args->start_month2);
        msgdbg("field_number           : %ld\n", args->field_number);
        msgdbg("week_number_field_flag : %d\n", args->week_number_field_flag);
        msgdbg("add_number             : %ld\n", args->add_number);
        msgdbg("set_add_number_flag    : %d\n", args->set_add_number_flag);
        msgdbg("input_file_pointer     : %d\n", is_file_open(args->input_file_pointer));
    }

    if (err_flag) {
        if (!set_err_msg_flag) {
            msgerr2("許されない引数が指定されました。\n");
        }
        exit(EXIT_FAILURE);
    }
}

////////////////////////////////////////////////////////////////////////////////
// 出力処理
////////////////////////////////////////////////////////////////////////////////
void output(arg_struct *args)
{
    if (!is_file_open(args->input_file_pointer)) {
        output_from_arg(args);
    } else {
        output_from_input_file(args);
    }
}

////////////////////////////////////////////////////////////////////////////////
// 引数からの出力処理
////////////////////////////////////////////////////////////////////////////////
void output_from_arg(arg_struct *args)
{
    bool err_flag = true;
    bool set_err_msg_flag = false;

    char *output_string = NULL;
    if (strlen(args->start_date) > 0) {
        if (!args->set_add_number_flag) {
            if (strlen(args->option) == 0) {
                output_string = get_md_date_and_day_number_and_md_week_number_string(args->start_date);
                if (output_string != NULL) {
                    err_flag = false;
                }
            } else if (strcmp(args->option, OPTION_DAY) == 0) {
                output_string = get_day_number_string(args->start_date);
                if (output_string != NULL) {
                    err_flag = false;
                }
            } else if (strcmp(args->option, OPTION_WEEK) == 0) {
                output_string = get_md_week_number_string_by_md_date(args->start_date);
                if (output_string != NULL) {
                    err_flag = false;
                }
            }
        } else {
            if (strlen(args->option) == 0) {
                output_string = get_md_date_string(args->start_date, args->add_number);
                if (output_string != NULL) {
                    err_flag = false;
                }
            } else if (strcmp(args->option, OPTION_RANGE) == 0) {
                output_string = get_md_date_range_string(args->start_date, args->add_number);
                if (output_string != NULL) {
                    err_flag = false;
                }
            }
        }
    } else if (strlen(args->start_week_number) > 0) {
        if (!args->set_add_number_flag) {
            if (strlen(args->option) == 0) {
                output_string = get_one_md_week_dates_string(args->start_week_number);
                if (output_string != NULL) {
                    err_flag = false;
                }
            } else if (strcmp(args->option, OPTION_DAY) == 0 && args->day_number != DAY_NUMBER_INVALID) {
                output_string = get_md_date_string_by_md_week_number_and_day_number(args->start_week_number, args->day_number);
                if (output_string != NULL) {
                    err_flag = false;
                }
            }
        } else {
            if (strlen(args->option) == 0) {
                output_string = get_md_week_number_string(args->start_week_number, args->add_number);
                if (output_string != NULL) {
                    err_flag = false;
                }
            } else if (strcmp(args->option, OPTION_RANGE) == 0) {
                output_string = get_md_week_number_range_string(args->start_week_number, args->add_number);
                if (output_string != NULL) {
                    err_flag = false;
                }
            }
        }
    } else if (strlen(args->option) == 0 && strlen(args->start_month) > 0) {
        if (strlen(args->start_month2) == 0) {
            if (!args->set_add_number_flag) {
                output_string = get_one_md_month_dates_string(args->start_month);
                if (output_string != NULL) {
                    err_flag = false;
                }
            } else {
                output_string = get_md_month_string(args->start_month, args->add_number);
                if (output_string != NULL) {
                    err_flag = false;
                }
            }
        } else if (!args->set_add_number_flag) {
            output_string = get_md_month_diff_string(args->start_month2, args->start_month);
            if (output_string != NULL) {
                err_flag = false;
            }
        }
    }

    if (!err_flag && output_string != NULL) {
        printf("%s\n", output_string);
        free(output_string);
        err_flag = false;
    }

    if (err_flag) {
        if (!set_err_msg_flag) {
            msgerr2("許されない引数が指定されました。\n");
        }
        exit(EXIT_FAILURE);
    }
}

////////////////////////////////////////////////////////////////////////////////
// 入力ファイルのフィールドからの出力処理
////////////////////////////////////////////////////////////////////////////////
void output_from_input_file_field(arg_struct *args, char *field, long line_count, char *output_buf)
{
    bool err_flag = true;
    bool set_err_msg_flag = false;

    if (strcmp(field, DATE_STRING_TODAY) == 0 || strcmp(field, DATE_STRING_YESTERDAY) == 0) {
        msgerr("行=%ld today/ydayは指定出来ません。\n", line_count);
        set_err_msg_flag = true;
    } else {
        static char prev_field[DATE_STRING_LENGTH + 1] = "";
        static bool prev_output_field_flag;
        static char *prev_output_string = NULL;
        bool output_field_flag = false;
        char *output_string = NULL;
        if (strcmp(field, prev_field) == 0) {
            output_field_flag = prev_output_field_flag;
            output_string = prev_output_string;
            err_flag = false;
        } else {
            int field_len = strlen(field);
            if (!args->week_number_field_flag && field_len == DATE_STRING_LENGTH && add_md_date(field, 0) != NULL) {
                if (!args->set_add_number_flag) {
                    if (strlen(args->option) == 0) {
                        output_string = get_md_date_and_day_number_and_md_week_number_string(field);
                        if (output_string != NULL) {
                            err_flag = false;
                        }
                    } else if (strcmp(args->option, OPTION_DAY) == 0) {
                        output_field_flag = true;
                        output_string = get_day_number_string(field);
                        if (output_string != NULL) {
                            err_flag = false;
                        }
                    } else if (strcmp(args->option, OPTION_WEEK) == 0) {
                        output_field_flag = true;
                        output_string = get_md_week_number_string_by_md_date(field);
                        if (output_string != NULL) {
                            err_flag = false;
                        }
                    }
                } else {
                    if (strlen(args->option) == 0) {
                        output_field_flag = true;
                        output_string = get_md_date_and_day_number_and_md_week_number_string(add_md_date(field, args->add_number));
                        if (output_string != NULL) {
                            err_flag = false;
                        }
                    } else if (strcmp(args->option, OPTION_RANGE) == 0) {
                        output_string = get_md_date_range_string(field, args->add_number);
                        if (output_string != NULL) {
                            err_flag = false;
                        }
                    }
                }
            } else if (args->week_number_field_flag && field_len == WEEK_NUMBER_STRING_LENGTH && get_md_week_number_row_index(field) != DATE_INDEX_INVALID) {
                if (!args->set_add_number_flag) {
                    if (strlen(args->option) == 0) {
                        output_field_flag = true;
                        output_string = get_one_md_week_dates_string(field);
                        if (output_string != NULL) {
                            err_flag = false;
                        }
                    } else if (strcmp(args->option, OPTION_DAY) == 0 && args->day_number != DAY_NUMBER_INVALID) {
                        output_field_flag = true;
                        output_string = get_md_date_string_by_md_week_number_and_day_number(field, args->day_number);
                        if (output_string != NULL) {
                            err_flag = false;
                        }
                    }
                } else {
                    if (strlen(args->option) == 0) {
                        output_field_flag = true;
                        output_string = get_md_week_number_string(field, args->add_number);
                        if (output_string != NULL) {
                            err_flag = false;
                        }
                    } else if (strcmp(args->option, OPTION_RANGE) == 0) {
                        output_string = get_md_week_number_range_string(field, args->add_number);
                        if (output_string != NULL) {
                            err_flag = false;
                        }
                    }
                }
            }
            if (!err_flag) {
                free(prev_output_string);
                strcpy(prev_field, field);
                prev_output_field_flag = output_field_flag;
                prev_output_string = output_string;
            }
        }

        if (!err_flag && output_string != NULL) {
            if (output_field_flag) {
                buffered_fputs(field, false, stdout, output_buf, BUFFER_LENGTH);
                buffered_fputs(OUTPUT_DELIMITER, false, stdout, output_buf, BUFFER_LENGTH);
            }
            buffered_fputs(output_string, false, stdout, output_buf, BUFFER_LENGTH);
            err_flag = false;
        }
    }

    if (err_flag) {
        if (!set_err_msg_flag) {
            msgerr("行=%ld 日付指定/週NO指定に対して入力データが正しくありません。\n", line_count);
        }
        exit(EXIT_FAILURE);
    }
}

////////////////////////////////////////////////////////////////////////////////
// 入力ファイルからの出力処理
////////////////////////////////////////////////////////////////////////////////
void output_from_input_file(arg_struct *args)
{
    char input_buf[BUFFER_LENGTH], output_buf[BUFFER_LENGTH];
    output_buf[0] = '\0';
    long line_count = 1;
    int field_count = 1;
    char tmp_field[DATE_STRING_LENGTH + 1];
    int tmp_field_last_index = 0;
    while (fgets(input_buf, sizeof(input_buf), args->input_file_pointer) != NULL) {
        for(int i = 0; i < strlen(input_buf); i++) {
            char tmp_char = input_buf[i];
            bool line_end_flag = false;
            bool output_char_flag = true;
            bool output_delimiter_flag = false;
            bool target_field_read_flag = (field_count == args->field_number);
            bool target_field_end_flag = false;
            if (tmp_char == INPUT_FILE_DELIMITER_CHAR) {
                target_field_end_flag = target_field_read_flag;
                field_count++;
                output_delimiter_flag = true;
            } else if (tmp_char == '\n') {
                line_end_flag = true;
                target_field_end_flag = target_field_read_flag;
                field_count = 1;
            } else if (target_field_read_flag) {
                if (tmp_field_last_index < DATE_STRING_LENGTH) {
                    tmp_field[tmp_field_last_index++] = tmp_char;
                    output_char_flag = false;
                } else {
                    tmp_field_last_index = 0;
                    target_field_end_flag = true;
                }
            }
            if (target_field_end_flag) {
                tmp_field[tmp_field_last_index] = '\0';
                tmp_field_last_index = 0;
                output_from_input_file_field(args, tmp_field, line_count, output_buf);
            }
            if (output_delimiter_flag) {
                buffered_fputs(OUTPUT_DELIMITER, false, stdout, output_buf, BUFFER_LENGTH);
            } else if (output_char_flag) {
                buffered_fputc(tmp_char, false, stdout, output_buf, BUFFER_LENGTH);
            }
            if (line_end_flag) {
                line_count++;
            }
        }
    }
    buffered_fputs_flush(true, stdout, output_buf, BUFFER_LENGTH);
}

////////////////////////////////////////////////////////////////////////////////
// "MD日付 曜日番号 MD週NO"を取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_date_and_day_number_and_md_week_number_string(char *date)
{
    char *result_string = NULL;

    int date_index_array[2];
    get_md_date_index(date, date_index_array);
    int row = date_index_array[0];
    int column = date_index_array[1];
    if (row != DATE_INDEX_INVALID && column != DATE_INDEX_INVALID) {
        result_string = malloc(sizeof(char) * (DATE_STRING_LENGTH + strlen(OUTPUT_DELIMITER) + DAY_NUMBER_STRING_LENGTH + strlen(OUTPUT_DELIMITER) + WEEK_NUMBER_STRING_LENGTH + 1));
        sprintf(result_string, "%s%s%d%s%s", date, OUTPUT_DELIMITER, column, OUTPUT_DELIMITER, get_md_week_number(row));
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// "曜日番号"を取得
////////////////////////////////////////////////////////////////////////////////
char *get_day_number_string(char *date)
{
    char *result_string = NULL;

    int date_index_array[2];
    get_md_date_index(date, date_index_array);
    int row = date_index_array[0];
    int column = date_index_array[1];
    if (row != DATE_INDEX_INVALID && column != DATE_INDEX_INVALID) {
        result_string = malloc(sizeof(char) * (DAY_NUMBER_STRING_LENGTH + 1));
        sprintf(result_string, "%d", column);
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// "MD週NO"を取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_week_number_string_by_md_date(char *date)
{
    char *result_string = NULL;

    int date_index_array[2];
    get_md_date_index(date, date_index_array);
    int row = date_index_array[0];
    int column = date_index_array[1];
    if (row != DATE_INDEX_INVALID && column != DATE_INDEX_INVALID) {
        result_string = malloc(sizeof(char) * (WEEK_NUMBER_STRING_LENGTH + 1));
        strcpy(result_string, get_md_week_number(row));
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// "MD日付"をMD日付と加算数から取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_date_string(char *base_date, int add_number)
{
    char *result_string = NULL;

    char *date = add_md_date(base_date, add_number);
    if (date != NULL) {
        result_string = malloc(sizeof(char) * (DATE_STRING_LENGTH + 1));
        strcpy(result_string, date);
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// 加算数の幅の"MD日付"の範囲を取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_date_range_string(char *start_date, int add_number)
{
    char *result_string = NULL;

    int date_index_array[2];
    get_md_date_index(start_date, date_index_array);
    int start_row = date_index_array[0];
    int start_column = date_index_array[1];
    if (start_row != DATE_INDEX_INVALID && start_column != DATE_INDEX_INVALID) {
        add_to_md_date_index(start_row, start_column, add_number, date_index_array);
        int end_row = date_index_array[0];
        int end_column = date_index_array[1];
        if (end_row != DATE_INDEX_INVALID && end_column != DATE_INDEX_INVALID) {
            int result_date_count = abs(add_number) + 1;
            result_string = malloc(sizeof(char) * ((result_date_count * DATE_STRING_LENGTH) + ((result_date_count - 1) * strlen(OUTPUT_DELIMITER)) + 1));
            result_string[0] = '\0';

            if (add_number < 0) {
                int tmp_row = start_row;
                int tmp_column = start_column;
                start_row = end_row;
                start_column = end_column;
                end_row = tmp_row;
                end_column = tmp_column;
            }

            for (int i = start_row; i <= end_row; i++) {
                int tmp_start_column = (i == start_row) ? start_column : MD_WEEK_FILE_FIELD_MONDAY;
                for (int j = tmp_start_column; j <= MD_WEEK_FILE_FIELD_SUNDAY; j++) {
                    if (i == end_row && j > end_column) {
                        break;
                    } else {
                        if (strlen(result_string) > 0) {
                            strcat(result_string, OUTPUT_DELIMITER);
                        }
                        strcat(result_string, get_md_date(i, j));
                    }
                }
            }
        }
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// MD週NOの週に含まれる"MD日付"を取得
////////////////////////////////////////////////////////////////////////////////
char *get_one_md_week_dates_string(char *week_number)
{
    char *result_string = NULL;

    int row = get_md_week_number_row_index(week_number);
    if (row != DATE_INDEX_INVALID) {
        int result_date_count = (MD_WEEK_FILE_FIELD_SUNDAY - MD_WEEK_FILE_FIELD_MONDAY) + 1;
        result_string = malloc(sizeof(char) * ((result_date_count * DATE_STRING_LENGTH) + ((result_date_count - 1) * strlen(OUTPUT_DELIMITER)) + 1));
        result_string[0] = '\0';
        for (int i = MD_WEEK_FILE_FIELD_MONDAY; i <= MD_WEEK_FILE_FIELD_SUNDAY; i++) {
            if (strlen(result_string) > 0) {
                strcat(result_string, OUTPUT_DELIMITER);
            }
            strcat(result_string, get_md_date(row, i));
        }
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// "MD日付"をMD週NOと曜日番号から取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_date_string_by_md_week_number_and_day_number(char *week_number, int day_number)
{
    char *result_string = NULL;

    int row = get_md_week_number_row_index(week_number);
    if (row != DATE_INDEX_INVALID) {
        result_string = malloc(sizeof(char) * (DATE_STRING_LENGTH + 1));
        strcpy(result_string, get_md_date(row, day_number));
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// "MD週NO"をMD週NOと加算数から取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_week_number_string(char *base_week_number, int add_number)
{
    char *result_string = NULL;

    char *week_number = add_md_week_number(base_week_number, add_number);
    if (week_number != NULL) {
        result_string = malloc(sizeof(char) * (WEEK_NUMBER_STRING_LENGTH + 1));
        strcpy(result_string, week_number);
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// 加算数の幅の"MD週NO"の範囲を取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_week_number_range_string(char *start_week_number, int add_number)
{
    char *result_string = NULL;

    int start_row = get_md_week_number_row_index(start_week_number);
    if (start_row != DATE_INDEX_INVALID) {
        int end_row = start_row + add_number;
        if (get_md_week_number(end_row) != NULL) {
            int result_date_count = abs(add_number) + 1;
            result_string = malloc(sizeof(char) * ((result_date_count * WEEK_NUMBER_STRING_LENGTH) + ((result_date_count - 1) * strlen(OUTPUT_DELIMITER)) + 1));
            result_string[0] = '\0';

            if (add_number < 0) {
                int tmp_row = start_row;
                start_row = end_row;
                end_row = tmp_row;
            }

            for (int i = start_row; i <= end_row; i++) {
                if (strlen(result_string) > 0) {
                    strcat(result_string, OUTPUT_DELIMITER);
                }
                strcat(result_string, get_md_week_number(i));
            }
        }
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// MD月に含まれる"MD日付"を取得
////////////////////////////////////////////////////////////////////////////////
char *get_one_md_month_dates_string(char *month)
{
    char *result_string = NULL;

    int first_index_array[2], last_index_array[2];
    get_md_month_first_and_last_index(month, first_index_array, last_index_array);
    int first_row = first_index_array[0];
    int first_column = first_index_array[1];
    int last_row = last_index_array[0];
    int last_column = last_index_array[1];
    if (first_row != DATE_INDEX_INVALID) {
        int result_date_count = MAX_DAYS_IN_MONTH;
        result_string = malloc(sizeof(char) * ((result_date_count * DATE_STRING_LENGTH) + ((result_date_count - 1) * strlen(OUTPUT_DELIMITER)) + 1));
        result_string[0] = '\0';
        for (int i = first_row; i <= last_row; i++) {
            int tmp_start_column = (i == first_row) ? first_column : MD_WEEK_FILE_FIELD_MONDAY;
            for (int j = tmp_start_column; j <= MD_WEEK_FILE_FIELD_SUNDAY; j++) {
                if (i == last_row && j > last_column) {
                    break;
                } else {
                    if (strlen(result_string) > 0) {
                        strcat(result_string, OUTPUT_DELIMITER);
                    }
                    strcat(result_string, get_md_date(i, j));
                }
            }
        }
    }

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// "MD月"をMD月と加算数から取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_month_string(char *base_month, int add_number)
{
    char *result_string = NULL;

    result_string = malloc(sizeof(char) * (MONTH_STRING_LENGTH + 1));
    add_month_to_year_month(result_string, base_month, add_number);

    return result_string;
}

////////////////////////////////////////////////////////////////////////////////
// "月数の差"を開始月と終了月から取得
////////////////////////////////////////////////////////////////////////////////
char *get_md_month_diff_string(char *start_month, char *end_month)
{
    char *result_string = NULL;

    result_string = malloc(sizeof(char) * (get_number_of_digits(INT_MAX) + 1));
    sprintf(result_string, "%d", subtract_year_month(end_month, start_month));

    return result_string;
}

