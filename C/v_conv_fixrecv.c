////////////////////////////////////////////////////////////////////////////////
// 
// システム名：横断集計
// 
// プログラム名：v_conv_fixrecv
//
// 機能概要：
//   受信した固定長レコードを、レイアウトファイルに従い、項目切出・変換を行い、
//   空白区切りのテキストレコードを編集後、標準出力に書き出す。
//
// 作成日：2014/12/04
//
// 作成者：K.Shimada
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "COBconv.h"
char *strdup(const char *s);


#define MAXSIZE_LINE     4096*16
#define MAXNUM_FIELDS   512
#define MAXSIZE_FIELD   1024
#define MAXSIZE_OPTION  1024

#define msginf(fmt, ...) fprintf(stderr,"%s:%d:%s[INFO]"  fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#define msgerr(fmt, ...) fprintf(stderr,"%s:%d:%s[ERROR]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#ifdef DEBUG
#define msgdbg(fmt, ...) fprintf(stderr,"%s:%d:%s[DEBUG]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#else
#define msgdbg(fmt, ...) (void)0
#endif

char input_file[MAXSIZE_OPTION];
char param_file[MAXSIZE_OPTION];
unsigned char in_data[MAXSIZE_LINE];
conv_record_t rec;
conv_field_t para[1000];

////////////////////////////////////////////////////////////////////////////////
// 引数解析
////////////////////////////////////////////////////////////////////////////////
int analyze_option(int argc, char *argv[])
{
    if (argc == 1) {
        msgerr("引数が指定されていません%s\n", "");
        return 1;
    }

    input_file[0] = '\0';
    param_file[0] = '\0';

    if (argc == 2 || argc == 3) {
        // レイアウト定義ファイルのみが指定された
        strncpy(param_file, argv[1], sizeof(param_file) - 1);
        param_file[sizeof(input_file) - 1] = '\0';
    }

    if (argc == 3) {
        // レイアウト定義ファイルと入力ファイルが指定された
        strncpy(input_file, argv[2], sizeof(input_file) - 1);
        input_file[sizeof(input_file) - 1] = '\0';
    }

    if (argc > 3) {
        msgerr("無効な引数が指定されました(%s)\n", argv[3]);
        return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// メイン
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    msginf("開始%s", "\n");

    // 引数解析
    if (analyze_option(argc, argv) != 0) {
        msgerr("異常終了 引数の解析でエラーが発生%s", "\n");
        return 1;
    }

    int para_num = sizeof(para) / sizeof(para[0]);

    // レイアウト定義ファイルの取り込み
    if (read_layout(param_file, &para_num, para) < 0) {
        msgerr("異常終了 レイアウト定義の読み込みでエラー発生%s",
               "\n");
        return 1;
    }


    if (init_cob2str("para", para_num, para) < 0) {
        msgerr
            ("異常終了 レイアウト定義初期化でエラー発生 layout=%s\n",
             "para");
        return 1;
    }
    // 最終項目の位置とサイズからレコード長を求める
    size_t cob_size = para[para_num - 1]._pos + para[para_num - 1]._byte;

    // ファイル準備
    FILE *in_fp, *ot_fp;
    int in_cnt = 0, ot_cnt = 0;
    if (strcmp(input_file, "") == 0 || strcmp(input_file, "-") == 0) {
        in_fp = stdin;
    } else {
        if ((in_fp = fopen(input_file, "r")) == NULL) {
            msgerr
                ("異常終了 入力ファイルが読み取り不可(%s)%s",
                 input_file, "\n");
            return (1);
        }
    }
    ot_fp = stdout;

    // ファイル読み取りループ
    while (true) {

        // ファイルＲＥＡＤ
        int rtn = fread(in_data, cob_size, 1, in_fp);
        if (rtn != 1) {
            if (feof(in_fp) != 0 && ferror(in_fp) == 0) {
                break;
            }
            msgerr("入力ファイルの読み取りでエラー発生 LINE=%d rtn=%d\n",
                   in_cnt, rtn);
            return -1;
        }
        in_cnt++;

        if (conv_cob2str(in_data, para_num, para, &rec, false) < 0) {
            msgerr("変換エラー発生 LINE=%d\n", in_cnt);
            return -1;
        }

        fprintf(ot_fp, "%s\n", rec.str);
        ot_cnt++;

    }

    // 正常終了
    msginf("入力件数=%d 出力件数=%d\n", in_cnt, ot_cnt);
    msginf("正常終了%s", "\n");
    return 0;
}
