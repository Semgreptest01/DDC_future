////////////////////////////////////////////////////////////////////////////////
//
// システム名：横断集計
//
// プログラム名：v_colcnt
//
// 機能概要：
//   指定した入力ファイルの行ごとに列数を集計し、標準出力に書きだす。
//
// 作成日：2014/12/04
//
// 作成者：Hung.LV
//
// Ｃ言語規格：C99
//
// ソース記述形式：indent -kr -nut -l90
//
// 修正履歴
// 2014/12/24 K.Shimada
//            前行と同一件数の場合は出力しない
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define MAXSIZE_LINE     4096*16
#define MAXNUM_FIELDS   512
#define MAXSIZE_FIELD   1024
#define MAXSIZE_OPTION  1024

#define msginf(fmt, ...) fprintf(stderr,"%s:%d:%s[INFO]"  fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#define msgerr(fmt, ...) fprintf(stderr,"%s:%d:%s[ERROR]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#ifdef DEBUG
#define msgdbg(fmt, ...) fprintf(stderr,"%s:%d:%s[DEBUG]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#else                           /*  */
#define msgdbg(fmt, ...) (void)0
#endif                          /*  */
char input_file[MAXSIZE_OPTION];        // 入力ファイルパス名

////////////////////////////////////////////////////////////////////////////////
// 引数解析
////////////////////////////////////////////////////////////////////////////////
int analyze_option(int argc, char *argv[])
{
    if (argc > 2) {
        msgerr("引数は1個以下を指定してください%s\n", "");
        return 1;
    }
    input_file[0] = '\0';

    // 入力ファイルを取得する。
    if (optind < argc) {
        strncpy(input_file, argv[optind], sizeof(input_file) - 1);
        input_file[sizeof(input_file) - 1] = '\0';
    } else {

        // 入力ファイルを設定しない
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// メイン
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    msginf("%s\n", "開始");
    if (analyze_option(argc, argv) != 0) {
        msgerr("異常終了 引数の解析でエラーが発生%s\n", "");
        return (1);
    }
    // ファイル準備
    FILE *in_fp, *ot_fp;
    if (strcmp(input_file, "") == 0 || strcmp(input_file, "-") == 0) {
        in_fp = stdin;
    } else {
        if ((in_fp = fopen(input_file, "r")) == NULL) {
            msgerr("異常終了 入力ファイルが読み取り不可(%s)\n",
                   input_file);
            return (1);
        }
    }
    ot_fp = stdout;

    // ファイル読み取りループ
    char in_line[MAXSIZE_LINE];
    int prev = 0;
    int count_Line = 0;

    while (fgets(in_line, sizeof(in_line), in_fp) != NULL) {
        int column = 0;
        for (char *f = strtok(in_line, " \n"); f; f = strtok(NULL, " \n")) {
            column++;
        }

        if (prev != column) {
            char c_clolumn[MAXNUM_FIELDS];
            snprintf(c_clolumn, sizeof(c_clolumn), "%d", column);
            if ((fputs(c_clolumn, ot_fp)) == EOF) {
                msgerr("異常終了 結果出力でエラーが発生(LINE=%d)\n",
                       count_Line);
                return 1;
            }
            fputs("\n", ot_fp);
        }
        prev = column;

        count_Line++;
    }
    fclose(in_fp);

    // 正常終了
    msginf("%s\n", "正常終了");
    return 0;
}
