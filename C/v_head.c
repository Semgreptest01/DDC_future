////////////////////////////////////////////////////////////////////////////////
//
// システム名：横断集計
//
// プログラム名：v_head
//
// 機能概要：
//   指定した入力ファイルの先頭行から指定した行数まで、入力ファイルをそのまま標準出力に書き出す。
//
// 作成日：2014/11/25
//
// 作成者：Hung-QQ
//
// Ｃ言語規格：C99
//
// ソース記述形式：indent -kr -nut -l90
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXSIZE_LINE     4096*16
#define MAXSIZE_OPTION  1024

#define msginf(fmt, ...) fprintf(stderr,"%s:%d:%s[INFO]"  fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#define msgerr(fmt, ...) fprintf(stderr,"%s:%d:%s[ERROR]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#ifdef DEBUG
#define msgdbg(fmt, ...) fprintf(stderr,"%s:%d:%s[DEBUG]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#else
#define msgdbg(fmt, ...) (void)0
#endif

char input_file[MAXSIZE_OPTION];        // 入力ファイルパス名

////////////////////////////////////////////////////////////////////////////////
// 引数解析
////////////////////////////////////////////////////////////////////////////////
int analyze_option(int argc, char *argv[])
{
    if (argc == 1) {
        // 引数が指定されていません
        msgerr("引数が指定されていません%s\n", "");
        return (1);
    }

    if (argc > 3) {
        // 引数は2個以下を指定してください
        msgerr("引数は2個以下を指定してください%s\n", "");
        return (1);
    }
    input_file[0] = '\0';

    char *s = strtok(argv[1], " \0");
    if (s[0] != '-') {
        // 許されない引数が指定されました
        msgerr("許されない引数が指定されました(%s)\n", argv[1]);
        return (1);
    }

    for (int i = 1; s[i] != '\0'; i++) {
        if (isdigit(s[i]) == 0) {
            // フィールド指定形式に誤りがあります
            msgerr("フィールド指定形式に誤りがあります(%s)\n", argv[1]);
            return (1);
        }
    }

    if (argc == 3) {
        strncpy(input_file, argv[2], sizeof(input_file) - 1);
        input_file[sizeof(input_file) - 1] = '\0';
    }

    return (0);
}

////////////////////////////////////////////////////////////////////////////////
// メイン
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // 処理が開始します
    msginf("%s\n", "開始");

    // 引数解析
    if (analyze_option(argc, argv) != 0) {
        // 引数の解析でエラーが発生
        msgerr("異常終了 引数の解析でエラーが発生%s\n", "");
        return (1);
    }
    // ファイル準備
    FILE *in_fp, *ot_fp;

    char count_line[MAXSIZE_OPTION] = "";
    strncpy(count_line, argv[1] + 1, sizeof(count_line) - 1);
    count_line[sizeof(count_line) - 1] = '\0';

    char *buff = "";
    long count_row_ouput = strtol(count_line, &buff, 10);

    if (count_row_ouput == 0) {
        // 行数は0を超えた値で入力してください
        msgerr("行数は0を超えた値で入力してください%s\n", "");
        return (1);
    }

    int check_input = 0;
    int in_cnt = 0;
    if (strcmp(input_file, "") == 0 || strcmp(input_file, "-") == 0) {
        in_fp = stdin;
    } else {
        if ((in_fp = fopen(input_file, "r")) == NULL) {
            // 入力ファイルが読み取り不可
            msgerr("異常終了 入力ファイルが読み取り不可(%s)\n",
                   input_file);
            return (1);
        }
        check_input = 1;
    }
    ot_fp = stdout;

    // ファイル読み取りループ
    char in_line[MAXSIZE_LINE];

    while (fgets(in_line, sizeof(in_line), in_fp) != NULL) {

        in_cnt++;

        if (in_cnt <= count_row_ouput) {

            if ((fputs(in_line, ot_fp)) == EOF) {
                msgerr("異常終了 結果出力でエラーが発生(LINE=%d)\n",
                       in_cnt);
                return 1;
            }

        } else {
            if (check_input)
                break;
        }
    }

    // 処理が正常に終了しました
    msginf("%s\n", "正常終了");
    return (0);
}
