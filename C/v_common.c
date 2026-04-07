////////////////////////////////////////////////////////////////////////////////
//
// システム名：横断集計
//
// プログラム名：v_common
//
// 機能概要：
//  共通で使う処理を記述
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
#include "v_common.h"

////////////////////////////////////////////////////////////////////////////////
// strncpyの後にNULL文字を付加
////////////////////////////////////////////////////////////////////////////////
char *strncpy_and_add_null_char(char * __restrict__ _Dest, const char * __restrict__ _Source, size_t _Count)
{
    char *result = strncpy(_Dest, _Source, _Count);
    if (strlen(_Source) >= _Count)  {
        _Dest[_Count] = '\0';
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// ファイルを開いているかどうか
////////////////////////////////////////////////////////////////////////////////
bool is_file_open(FILE *fp)
{
    bool file_open_flag = false;
    if (fp != NULL)  {
        file_open_flag = true;
    }
    return file_open_flag;
}

////////////////////////////////////////////////////////////////////////////////
// ファイルの行数の取得
////////////////////////////////////////////////////////////////////////////////
int get_file_line_count(char *file_path, int max_line_length)
{
    int file_line_count = 0;
    FILE *fp;
    char tmp_line_string[max_line_length + 1];
    if ((fp = fopen(file_path, "r")) != NULL)  {
        while (fgets(tmp_line_string, sizeof(tmp_line_string), fp) != NULL) {
            file_line_count++;
        }
        fclose(fp);
    }
    return file_line_count;
}

////////////////////////////////////////////////////////////////////////////////
// 文字列をdelimiterで分割する
////////////////////////////////////////////////////////////////////////////////
int split(char *input_string, const char *delimiter, char *output_array[], size_t max_output_count)
{
    char *tok;
    int output_count = 0;
    tok = strtok(input_string, delimiter);
    while (tok != NULL && output_count < max_output_count) {
        output_array[output_count++] = tok;
        tok = strtok(NULL, delimiter);
    }
    return output_count;
}

////////////////////////////////////////////////////////////////////////////////
// システム日付をoutput_stringに代入
////////////////////////////////////////////////////////////////////////////////
size_t set_system_date(char *output_string, size_t maxsize, const char *format)
{
    time_t timer;
    struct tm *time_pointer;
    time(&timer);
    time_pointer = localtime(&timer);
    return strftime(output_string, maxsize, format, time_pointer);
}

////////////////////////////////////////////////////////////////////////////////
// 年月に月を加算
////////////////////////////////////////////////////////////////////////////////
void add_month_to_year_month(char *output_year_month, char *input_year_month, int add_number)
{
    char input_year_string[5], input_month_string[3];
    strncpy_and_add_null_char(input_year_string, input_year_month, 4);
    strncpy_and_add_null_char(input_month_string, input_year_month + 4, 2);

    int year = atoi(input_year_string);
    int month = atoi(input_month_string);

    month += add_number;

    while (month > 12) {
        month -= 12;
        year++;
    }
    while (month < 1) {
        month += 12;
        year--;
    }
    sprintf(output_year_month, "%04d%02d", year, month);
}

////////////////////////////////////////////////////////////////////////////////
// 年月を減算して月数を取得
////////////////////////////////////////////////////////////////////////////////
int subtract_year_month(char *year_month_0, char *year_month_1)
{
    char year_string_0[5], month_string_0[3], year_string_1[5], month_string_1[3];
    strncpy_and_add_null_char(year_string_0, year_month_0, 4);
    strncpy_and_add_null_char(month_string_0, year_month_0 + 4, 2);
    strncpy_and_add_null_char(year_string_1, year_month_1, 4);
    strncpy_and_add_null_char(month_string_1, year_month_1 + 4, 2);

    int year_0 = atoi(year_string_0);
    int month_0 = atoi(month_string_0);
    int year_1 = atoi(year_string_1);
    int month_1 = atoi(month_string_1);

    int result_month_count = (year_0 - year_1) * 12 + (month_0 - month_1);
    return result_month_count;
}

////////////////////////////////////////////////////////////////////////////////
// 数値の桁数を取得
////////////////////////////////////////////////////////////////////////////////
int get_number_of_digits(long number)
{
    return ((int) log10(number) + 1);
}

////////////////////////////////////////////////////////////////////////////////
// 文字をバッファしつつ出力
// output_bufに必ず'\0'が存在すること
////////////////////////////////////////////////////////////////////////////////
void buffered_fputc(char output_char, bool flush_flag, FILE *fp, char *output_buf, int output_buf_len)
{
    int tmp_last_index = strlen(output_buf);
    output_buf[tmp_last_index] = output_char;
    output_buf[tmp_last_index + 1] = '\0';
    buffered_fputs_flush(false, fp, output_buf, output_buf_len);
}

////////////////////////////////////////////////////////////////////////////////
// 文字列をバッファしつつ出力
// output_bufに必ず'\0'が存在すること
////////////////////////////////////////////////////////////////////////////////
void buffered_fputs(char *string, bool flush_flag, FILE *fp, char *output_buf, int output_buf_len)
{
    int last_buff_len = strlen(output_buf);
    int write_len = (output_buf_len - 1) - last_buff_len;
    int string_len = strlen(string);
    if (string_len < write_len) {
        write_len = string_len;
    }
    strncpy_and_add_null_char(output_buf + last_buff_len, string, write_len);
    buffered_fputs_flush(false, fp, output_buf, output_buf_len);
    if (write_len < string_len) {
        buffered_fputs(string + write_len, flush_flag, fp, output_buf, output_buf_len);
    }
}

////////////////////////////////////////////////////////////////////////////////
// 文字列をバッファしつつ出力する処理のフラッシュ
// output_bufに必ず'\0'が存在すること
////////////////////////////////////////////////////////////////////////////////
void buffered_fputs_flush(bool flush_flag, FILE *fp, char *output_buf, int output_buf_len)
{
    int output_len = strlen(output_buf);
    if (output_len > 0 && (flush_flag || output_len == (output_buf_len - 1))) {
        fputs(output_buf, fp);
        output_buf[0] = '\0';
    }
}

