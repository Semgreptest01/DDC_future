////////////////////////////////////////////////////////////////////////////////
//
// システム名：横断集計
//
// プログラム名：v_mjoin
//
// 機能概要：
//   指定した複数の入力ファイルをKEY順でマージし、入力ファイルのソート順は維持したまま標準出力に書き出す。
//   キーでマッチしたレコードについては、キーを除いた項目を横に連結する。
//   マッチしないファイルについては、0 で項目を補完する。　マージした内容を標準出力に書き出す。
//
// 作成日：2014/12/02
//
// 作成者：TOAN-VD
//
// Ｃ言語規格：C99
//
// ソース記述形式：indent -kr -nut -l90
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <stdbool.h>

#define MAXSIZE_LINE    4096*16
#define MAXNUM_FIELDS   512
#define MAXSIZE_OPTION  1024
#define MAXIMUM_FILE_NUMBER 5
#define MINIMUM_FILE_NUMBER 2

#define msginf(fmt, ...) fprintf(stderr,"%s:%d:%s[INFO]"  fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#define msgerr(fmt, ...) fprintf(stderr,"%s:%d:%s[ERROR]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#ifdef DEBUG
#define msgdbg(fmt, ...) fprintf(stderr,"%s:%d:%s[DEBUG]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#else
#define msgdbg(fmt, ...) (void)0
#endif

int field_no = -1;              // Input field number
char opt_value[MAXSIZE_OPTION]; // Input replace value
int input_file_number = 0;      // Number of input files

typedef struct {
    char input_file[MAXSIZE_OPTION];
    int line_num;
    FILE *in_fp;
    bool read_finished;
    bool wait_for_read;
} file_t;
static file_t files[MAXIMUM_FILE_NUMBER];       // List of input files

typedef struct {
    char field_line[MAXSIZE_LINE];
    int field_num;
    char *fields[MAXNUM_FIELDS];
} field_t;

////////////////////////////////////////////////////////////////////////////////
// 入力のフィールドを分解する
////////////////////////////////////////////////////////////////////////////////
int fieldsplit(field_t * field)
{

    field->field_num = 0;
    for (char *f = strtok(field->field_line, " \n"); f; f = strtok(NULL, " \n")) {
        field->fields[field->field_num++] = f;
    }
    if (field->field_num > MAXNUM_FIELDS) {
        msgerr("入力フィールドオーバー max=%d real=%d\n", MAXNUM_FIELDS,
               (field->field_num) - 1);
        return 1;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Copy key from field_src to field_dest
////////////////////////////////////////////////////////////////////////////////
int cpykey(field_t * field_dest, field_t * field_src)
{

    char *dest = field_dest->field_line;        // フィールドのコピー先ポインタ
    field_dest->field_num = 0;

    for (int i = 0; i < field_no; i++) {

        // フィールド番号に指定された内容を、入力領域 から キー領域 へコピー
        size_t size = strlen(field_src->fields[i]) + 1; // 文字列終端もコピー対象
        memcpy(dest, field_src->fields[i], size);
        field_dest->fields[field_dest->field_num++] = dest;

        // コピーした分進める
        dest += size;
    }
    return 0;

}

////////////////////////////////////////////////////////////////////////////////
// Compare keys in two fields
////////////////////////////////////////////////////////////////////////////////
int cmpkey(field_t * field1, field_t * field2)
{
    int rtn = 0;

    if (strcmp(field1->field_line, "") == 0) {
        return -1;
    }

    if (strcmp(field2->field_line, "") == 0) {
        return 1;
    }

    for (int i = 0; i < field_no; i++) {
        rtn = strcmp(field1->fields[i], field2->fields[i]);
        if (rtn != 0) {
            // 不一致のフィールドを見つけたら比較終了
            return rtn;
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 引数解析
////////////////////////////////////////////////////////////////////////////////
int analyze_option(int argc, char *argv[])
{
    if (argc == 1) {
        msgerr("引数が指定されていません%s\n", "");
        return 1;
    }

    opt_value[0] = '\0';
    strcpy(opt_value, "0");
    for (int i = 0; i < MAXIMUM_FILE_NUMBER; i++) {
        files[i].input_file[0] = '\0';
    }

    // Check option d
    char c;
    while ((c = getopt(argc, argv, "d:")) != -1) {
        switch (c) {
        case 'd':
            strcpy(opt_value, optarg);
            break;
        default:
            msgerr("許されない引数が指定されました%s\n", "");
            return 1;
        }
    }

    // Parameter error
    if (optind >= argc) {
        return 1;
    }

    char temp[MAXSIZE_OPTION];
    strncpy(temp, argv[optind], sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    char *validator;
    char *str_no;
    char *endptr;

    if ((validator = strstr(temp, "==")) != NULL) {
        // 実行方法エラー。
        msgerr("実行方法エラー。実行方法は%sで指定してください\n",
               "pk=XX(数値)");
        return 1;
    }

    if ((validator = strtok(temp, "=")) == NULL) {
        // 実行方法エラー。
        msgerr("pk=の指定は必須です%s\n", "");
        return 1;
    }

    if (strcmp(validator, "pk")) {
        // 実行方法エラー。
        msgerr("pk=の指定は必須です%s\n", "");
        return 1;
    }

    if ((str_no = strtok(NULL, "=")) == NULL) {
        // 実行方法エラー。
        msgerr("実行方法エラー。実行方法は%sで指定してください\n",
               "pk=XX(数値)");
        return 1;
    }

    if (strtok(NULL, "=") != NULL) {
        // 実行方法エラー。
        msgerr("実行方法エラー。実行方法は%sで指定してください\n",
               "pk=XX(数値)");
        return 1;
    }
    // ｐｋ＝の値を取得する。
    field_no = strtol(str_no, &endptr, 10);

    // フィールド番号に数値以外が指定された時はエラー
    if (*endptr != '\0') {
        msgerr("フィールド指定形式に誤りがあります(%s)\n", str_no);
        return 1;
    }

    if (field_no < 1) {
        msgerr("%s\n", "フィールド番号は1以上を指定してください");
        return 1;
    }
    if (field_no > 512) {
        msgerr("フィールド番号は %d以下を指定してください\n",
               MAXNUM_FIELDS);
        return 1;
    }

    optind++;
    // minimum file number 2
    if ((argc - optind) < MINIMUM_FILE_NUMBER) {
        msgerr("入力ファイルが最少で%dとしてください\n",
               MINIMUM_FILE_NUMBER);
        return 1;
    }
    // maximum file numbers 5
    if ((argc - optind) > MAXIMUM_FILE_NUMBER) {
        msgerr("入力ファイルが最大で%dとしてください\n",
               MAXIMUM_FILE_NUMBER);
        return 1;
    }
    // get input files
    int file_index = 0;
    bool is_mul_stdin = false;
    for (; optind < argc; optind++) {
        strncpy(files[file_index].input_file, argv[optind],
                sizeof(files[file_index].input_file) - 1);
        files[file_index].input_file[sizeof(files[file_index].input_file) - 1] = '\0';
        if (strcmp(files[file_index].input_file, "-") == 0) {
            if (!is_mul_stdin) {
                is_mul_stdin = true;
            } else {
                // Multiple stdin
                msgerr("%s\n",
                       "異常終了 標準入力から複数指定することができません");
                return 1;
            }
        }
        file_index++;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Check a line is empty
//  Line is "\n" "\r"
//  All character is [Space]" "
////////////////////////////////////////////////////////////////////////////////
bool is_empty_line(char *p_line)
{
    if (strcmp(p_line, "\n") == 0 || strcmp(p_line, "\r") == 0) {
        return true;
    }
    for (int i = 0; i < strlen(p_line); i++) {
        if (*(p_line + i) != ' ' && *(p_line + i) != '\n' && *(p_line + i) != '\r') {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Output fields of line, exclude key
////////////////////////////////////////////////////////////////////////////////
int printf_field(field_t * field, FILE * ot_fp)
{
    for (int i = field_no; i < field->field_num; i++) {

        if (fputs(" ", ot_fp) == EOF) {
            msgerr("異常終了 結果出力でエラーが発生(FILE=%s,LINE=%d)\n",
                   files[i].input_file, files[i].line_num);
            return 1;

        }
        if (fputs(field->fields[i], ot_fp) == EOF) {
            msgerr("異常終了 結果出力でエラーが発生(FILE=%s,LINE=%d)\n",
                   files[i].input_file, files[i].line_num);
            return 1;
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Output key of line
////////////////////////////////////////////////////////////////////////////////
int printf_key(field_t * field, FILE * ot_fp)
{
    for (int i = 0; i < field_no; i++) {
        if (i != 0)
            if (fputs(" ", ot_fp) == EOF) {
                msgerr
                    ("異常終了 結果出力でエラーが発生(FILE=%s,LINE=%d)\n",
                     files[i].input_file, files[i].line_num);
                return 1;

            }
        if (fputs(field->fields[i], ot_fp) == EOF) {
            msgerr("異常終了 結果出力でエラーが発生(FILE=%s,LINE=%d)\n",
                   files[i].input_file, files[i].line_num);
            return 1;
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Release resource
////////////////////////////////////////////////////////////////////////////////
void clear()
{
    for (int i = 0; i < input_file_number; i++) {
        fclose(files[i].in_fp);
    }
}

////////////////////////////////////////////////////////////////////////////////
// メイン
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    msginf("%s\n", "開始");

    static field_t fields[MAXIMUM_FILE_NUMBER];
    static field_t min_field;   // A field which only contains minimum key


    // 引数解析
    if (analyze_option(argc, argv) != 0) {
        msgerr("異常終了 引数の解析でエラーが発生%s\n", "");
        return (1);
    }

    FILE *in_fp, *ot_fp;
    ot_fp = stdout;

    // Open input files
    for (int i = 0; i < MAXIMUM_FILE_NUMBER; i++) {

        if (strcmp(files[i].input_file, "") != 0 && strcmp(files[i].input_file, "-") != 0) {
            files[i].in_fp = fopen(files[i].input_file, "r");
            if (files[i].in_fp == NULL) {
                msgerr("異常終了 入力ファイルが読み取り不可(%s)\n",
                       files[i].input_file);
                return (1);
            }
            input_file_number++;
        } else if (strcmp(files[i].input_file, "-") == 0) {
            files[i].in_fp = stdin;
            input_file_number++;
        } else {
            continue;
        }
    }

    while (true) {
        for (int i = 0; i < input_file_number; i++) {
            if (files[i].read_finished) {
                continue;
            }
            if (!files[i].wait_for_read) {
                in_fp = files[i].in_fp;
                files[i].line_num++;

                // File i is finish
                if (fgets(fields[i].field_line, MAXSIZE_LINE, in_fp) == NULL) {

                    // If is first line
                    if (files[i].line_num == 1) {
                        fields[i].field_num = field_no + 1;
                    }
                    files[i].read_finished = true;
                    continue;
                }
                // First line
                if (files[i].line_num == 1) {
                    // if first line is empty, add one column in result file
                    if (is_empty_line(fields[i].field_line)) {
                        fields[i].field_num = field_no + 1;
                        files[i].read_finished = true;
                        continue;
                    }
                }
                // Save fields
                fieldsplit(&fields[i]);
                files[i].wait_for_read = true;
            }
        }

        // Check all file is read
        int read_finished_num = 0;
        for (; read_finished_num < input_file_number; read_finished_num++) {
            if (!files[read_finished_num].read_finished) {
                break;
            }
        }
        if (read_finished_num == input_file_number) {
            break;
        }
        // Get minimum key index
        int min_key_index = 0;
        for (int i = 0; i < input_file_number; i++) {
            if (strcmp(fields[min_key_index].field_line, "") == 0) {
                if (min_key_index < input_file_number - 1) {
                    min_key_index++;
                }
            } else if (strcmp(fields[i].field_line, "") == 0) {
                continue;
            } else {
                // get minimum key
                if (cmpkey(&fields[i], &fields[min_key_index]) < 0) {
                    min_key_index = i;
                }
            }
        }

        // Output
        cpykey(&min_field, &fields[min_key_index]);
        printf_key(&min_field, ot_fp);
        for (int i = 0; i < input_file_number; i++) {
            if (cmpkey(&min_field, &fields[i]) == 0) {
                printf_field(&fields[i], ot_fp);
                files[i].wait_for_read = false;
                // Reset field
                fields[i].field_line[0] = '\0';
            } else {
                for (int j = 0; j < fields[i].field_num - field_no; j++) {
                    if (fputs(" ", ot_fp) == EOF) {
                        msgerr
                            ("異常終了 結果出力でエラーが発生(FILE=%s,LINE=%d)\n",
                             files[i].input_file, files[i].line_num);
                        return false;
                    }
                    if (fputs(opt_value, ot_fp) == EOF) {
                        msgerr
                            ("異常終了 結果出力でエラーが発生(FILE=%s,LINE=%d)\n",
                             files[i].input_file, files[i].line_num);
                        return false;
                    }
                }
            }
        }

        if (fputs("\n", ot_fp) == EOF) {
            msgerr("異常終了 結果出力でエラーが発生%s\n", "");
            return false;
        }

    }

    clear();

    // 正常終了
    msginf("%s\n", "正常終了");
    return 0;
}
