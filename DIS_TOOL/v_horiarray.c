/****************************************************************************/
/*  システム名称：  汎用ＣＩＳツール                                        */
/*  コマンド名称：  v_horiarray                                           */
/*  機能        ：  キー以外の項目を横につなげる                            */
/*  制約事項    ：                                                          */
/*　　・                                                                    */
/* DEL 2014.12.18 Toan.VD */
/*  使用法      ：  v_horiarray [fs=string] pk=$ < inputfile > outputfile */
/* DEL 2014.12.18 Toan.VD */
/* ADD 2014.12.18 Toan.VD */
/*  使用法      ：  v_horiarray [-$] [fs=string] pk=$ < inputfile > outputfile */
/* ADD 2014.12.18 Toan.VD */
/*  ステータス  ：  0 : 正常終了                                            */
/*                  1 : アーギュメント不正                                  */
/*  作成者      ：  北条                                                    */
/*  作成日      ：  2001/11/08                                              */
/*  修正履歴    ：                                                          */
/*  No. 修正日付    修正者          修正内容                                */
/*  001 2001/11/26   北条   標準入力対応                                    */
/*　002 2002/01/18　 前川　 引数チェック対応　                              */
/*　003 2002/11/08　 川村　 最大レコードサイズ変更(1024Byte=>3072byte)      */
/*　004 2002/11/12　 川村　 最大キー指定数変更(20=>40)                      */
/*                          gets => fgetsに変更                             */
/*　005 2003/02/05　寺澤　　　 最大レコードサイズ変更(3072Byte=>6144byte)   */
/****************************************************************************/
/*--------------------------------------------------------------------------*/
/*  ヘッダーファイルインクルード定義                                        */
/*--------------------------------------------------------------------------*/
/*  System Headder File Include                                             */
#include <stdio.h>
#include <string.h>

/*--------------------------------------------------------------------------*/
/*  ｄｅｆｉｅｎ定義                                                        */
/*--------------------------------------------------------------------------*/
#define        ARGS_MIN        2

/* DEL 2014.12.18 Toan.VD */
// #define        ARGS_MAX        4
/* DEL 2014.12.18 Toan.VD */
/* ADD 2014.12.18 Toan.VD */
#define        ARGS_MAX        5        // 4 + 1
/* ADD 2014.12.18 Toan.VD */

#define     MAX_RECLEN      6144        /*  最大レコード長＋１      */
#define     MAX_KEYNUM      40  /*  最大指定キー数          */

void usage();

main(int argc, char *argv[])
{
    int i0 = 1, iNUM, i1, iFS = 1, iKB, iINPUT = 0, iFN = 0;
    char wC1[MAX_RECLEN], wC2[81], *wC3, wSPT[10], wKEY[MAX_KEYNUM][81];
    FILE *wF1;

    /*  アーギュメントの数をチェック                                        */
    if (argc < ARGS_MIN || ARGS_MAX < argc) {
        usage();
    }

    /* ADD 2014.12.18 Toan.VD */
    // -フィールド数
    if (argv[i0][0] == '-') {
        char *field_no_opt = argv[i0];
        field_no_opt++;
        iFN = atoi(field_no_opt);
        if (iFN < 1) {
            puts("指定フィールド数が正しくありません");
            exit(1);
        }
        i0++;
    }
    /* ADD 2014.12.18 Toan.VD */

    /*区切り文字取得 */
    if (NULL != strstr(argv[i0], "fs")) {
        strcpy(wC1, argv[i0]);
        strcpy(wSPT, strtok(wC1, "fs="));
        i0++;
    } else {
        strcpy(wSPT, " ");
    }

    /* ＫＥＹ項目数取得 */
    if (NULL != strstr(argv[i0], "pk")) {
        strcpy(wC1, argv[i0]);
        iNUM = atoi(strtok(wC1, "pk="));
        i0++;
    } else {
        iNUM = 0;
    }
    if (iNUM > MAX_KEYNUM) {
        printf("キー指定の最大数は[%d]\n", MAX_KEYNUM);
        exit(1);
    }

    /* ファイルオープン */
    wF1 = fopen(argv[i0], "r");
    if (NULL == wF1) {
        wF1 = stdin;
        iINPUT = 1;
    }
    /* ADD 2014.12.18 Toan.VD */
    int output_row_length = 0;


    int line_cnt = 0;
    /* ADD 2014.12.18 Toan.VD */

    /* 文字列取得 */
    while (1) {
        /* パラメータファイルから */
/* DEL 2002.11.12 Kawamura --
        if (0 == iINPUT){
-- DEL 2002.11.12 Kawamura -- */
        if (NULL == fgets(wC1, MAX_RECLEN, wF1)) {
            break;
        }
        /* ADD 2014.12.18 Toan.VD */
        line_cnt++;
        /* ADD 2014.12.18 Toan.VD */
/* DEL 2002.11.12 Kawamura --
        }
        else {
            if (NULL == gets(wC1)){
                break;
            }
        }
-- DEL 2002.11.12 Kawamura -- */

        /* キー項目有り */
        if (0 != iNUM) {
            /*項目切り出し */
            for (i1 = 0; iNUM > i1; i1++) {
                if (0 == i1) {
                    strcpy(wC2, strtok(strtok(wC1, "\n"), wSPT));
                } else {
                    strcpy(wC2, strtok(NULL, wSPT));
                }
                if (0 != strcmp(wKEY[i1], wC2)) {
                    iKB = 1;
                }
                strcpy(wKEY[i1], wC2);
            }

            /* キーブレーク時キー表示 */
            if (1 == iKB) {
                iKB = 0;
                for (i1 = 0; iNUM > i1; i1++) {
                    if (1 == iFS && 0 == i1) {
                    } else if (0 == i1) {
                        printf("\n");
                    } else {
                        printf("%s", wSPT);
                    }
                    printf("%s", wKEY[i1]);
                }
                output_row_length = 0;
            }
        }

        /*キー無し */
        else {
            /* DEL 2014.12.18 Toan.VD */
            /*wC3 = strtok(wC1,"\n");
               wC3 = strtok(wC1,wSPT);
               if (0 == iFS){
               printf("%s",wSPT);
               }
               printf("%s",wC3); */
            /* DEL 2014.12.18 Toan.VD */

            /* ADD 2014.12.18 Toan.VD */
            if (iFN > 0) {
                wC3 = strtok(wC1, "\n");
                int is_first_field = 0;
                while (1) {
                    if (is_first_field == 0) {
                        wC3 = strtok(wC1, wSPT);
                        is_first_field = 1;
                    } else {
                        wC3 = strtok(NULL, wSPT);
                    }

                    if (wC3 == NULL) {
                        break;
                    }

                    if (output_row_length < iFN) {
                        if (0 != output_row_length) {
                            printf("%s", wSPT);
                        }
                        printf("%s", wC3);
                        output_row_length++;
                    } else {
                        printf("\n");
                        printf("%s", wC3);
                        output_row_length = 1;
                    }
                }
            } else {
                wC3 = strtok(wC1, "\n");
                wC3 = strtok(wC1, wSPT);
                if (0 == iFS) {
                    printf("%s", wSPT);
                }
                printf("%s", wC3);
            }
            /* ADD 2014.12.18 Toan.VD */
        }

        /*残項目表示 */
        /* DEL 2014.12.18 Toan.VD */
        /*while (NULL != (wC3 = strtok(NULL,wSPT))){
           printf("%s",wSPT);
           printf("%s",wC3);
           } */
        /* DEL 2014.12.18 Toan.VD */
        /* ADD 2014.12.18 Toan.VD */
        if (iFN > 0) {
            while (NULL != (wC3 = strtok(NULL, wSPT))) {
                if (output_row_length < iFN) {
                    printf("%s", wSPT);
                    printf("%s", wC3);
                    output_row_length++;
                } else {
                    // Print key
                    for (i1 = 0; iNUM > i1; i1++) {
                        if (0 == i1) {
                            printf("\n");
                        } else {
                            printf("%s", wSPT);
                        }
                        printf("%s", wKEY[i1]);
                    }
                    printf("%s", wSPT);
                    printf("%s", wC3);
                    output_row_length = 1;
                }
            }
        } else {
            while (NULL != (wC3 = strtok(NULL, wSPT))) {
                printf("%s", wSPT);
                printf("%s", wC3);
            }
        }
        /* ADD 2014.12.18 Toan.VD */

        iFS = 0;
    }
    /* ADD 2014.12.18 Toan.VD */
    if (line_cnt > 0) {
        printf("\n");
    }
    /* ADD 2014.12.18 Toan.VD */
    return 0;
}

/****************************************************************************/
/*  関数名称：  usage（使用方法表示）                                       */
/*  処理概要：  エラー処理                                                  */
/*  戻り値  ：  ARG_ERR                                                     */
/****************************************************************************/
void usage()
{
    /* DEL 2014.12.18 Toan.VD */
    // puts("使用方法: v_horiarray [fs=string] pk=$ < inputfile > outputfile");
    /* DEL 2014.12.18 Toan.VD */
    /* ADD 2014.12.18 Toan.VD */
    puts("使用方法: v_horiarray [-$] [fs=string] pk=$ < inputfile > outputfile");
    /* ADD 2014.12.18 Toan.VD */

    exit(1);
}
