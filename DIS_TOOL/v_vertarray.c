/****************************************************************************/
/*  システム名称：  汎用ＣＩＳツール                                        */
/*  コマンド名称：  v_vertarray                                           */
/*  機能        ：  指定したキー以外の項目を縦展開を行う                    */
/*　　・                                                                    */
/*　　・                                                                    */
/*  制約事項    ：                                                          */
/*　　・インプットはフィールドセパレータは半角スペースとする。              */
/*　　・インプットのレコード(行)レングスはLFを含めて最大6144バイトとする。  */
/*　　・"fs="指定は10文字までとする                                         */
/*　　・フィールド数は400までとする                                         */
/*  使用法      ：  v_vertarray [fs=$] pk=$ < input > output              */
/*  ステータス  ：  0 : 正常終了                                            */
/*                  1 : 警告終了（出力失敗データ有り）                      */
/*                  2 : アーギュメント不正                                  */
/*                  3 : ファイルオープンエラー                              */
/*                  4 : ファイルリードエラー                                */
/*                  5 : メモリ確保エラー                                    */
/*  作成者      ：  北条                                                    */
/*  作成日      ：  2001/11/09                                              */
/*  修正履歴    ：                                                          */
/*  No. 修正日付    修正者          修正内容                                */
/*  001 2001/11/23   北条         標準入力対応                              */
/*  002 2002/01/19   木下         デバッグ用の出力を排除・処理効率アップ    */
/*　003 2002/11/11　 川村　 最大レコードサイズ変更(1024Byte=>3072byte)      */
/*　                        １レコード最大項目数変更(200=>400)              */
/*　004 2003/02/05　寺澤　　　 最大レコードサイズ変更(3072Byte=>6144byte)   */
/*　005 2015/04/21　蓮沼　　　 最大レコードサイズ変更(6144Byte=>1024*16byte)*/
/*　006 2015/05/11　蓮沼　　　 レコードサイズに関わらず、全データの取込対応 */
/*　　　　　　　　　　　　　　　 ※ ハードウェア全体のメモリ使用状態等により*/
/*　　　　　　　　　　　　　　　　　あまりにも大きいレコードサイズの場合は、*/
/*　　　　　　　　　　　　　　　　　処理に失敗（異常終了）する可能性がある  */
/*　　　　　　　　　　　　　　　本対応に伴い、No.005の対応を差し戻し        */
/****************************************************************************/
/*--------------------------------------------------------------------------*/
/*  ヘッダーファイルインクルード定義                                        */
/*--------------------------------------------------------------------------*/
/*  System Headder File Include                                             */
#include <string.h>

/*  User Headder File Include                                               */
#include "libdistool.inc"

/*--------------------------------------------------------------------------*/
/*  ｄｅｆｉｅｎ定義                                                        */
/*--------------------------------------------------------------------------*/
#define     ARGC_MIN    2       /* 最小引数カウント            */
#define     OPT_SEP_KEY "fs="   /* セパレータオプション        */
#define     SPA         " "
#define     RTN         "\n"
/* DEL 2015.05.11 K.Hasunuma No.005対応差戻・セグメンテーション違反発生リスク軽減 START */
///* ADD 2015.04.21 K.Hasunuma START */
//#undef      MAX_RECLEN
//#define     MAX_RECLEN  1024*16              /* 共通最大レコードサイズを上書き*/
///* ADD 2015.04.21 K.Hasunuma END */
/* DEL 2015.05.11 K.Hasunuma No.005対応差戻・セグメンテーション違反発生リスク軽減 END */

/*--------------------------------------------------------------------------*/
/*  内部関数定義                                                            */
/*--------------------------------------------------------------------------*/
void usage();

main(int argc, char *argv[])
{
    int loop_cnt = 1;           /* カウンタ                 */
    int fld_key = 0, iFN = 0;   /* フィールドキー           */

/* MODD 2015.05.11 K.Hasunuma 1レコード全データ取得の為、配列を再定義できる形に変更 START */
//    char read_tbl[MAX_RECLEN];  /* リード用ＴＢＬ           */
//    char cp_tbl[MAX_RECLEN];    /* コピー用ＴＢＬ           */
    char* read_tbl = (char *)malloc( sizeof( char ) * MAX_RECLEN );  /* リード用ＴＢＬ（1行分の全体の）           */
    // メモリ割り当てに成功したか
    if(read_tbl == NULL) {
        // メモリ確保失敗
        fprintf(stderr, "MALLOC_ERR：MEMORY ALLOCATION FAILURE FOR ARRAY(read_tbl)[MAX_RECLEN]【1】.\n");
        exit(MALLOC_ERR);
    }
    char read_tbl_work[MAX_RECLEN];  /* リード用ＴＢＬ（fgets1回分の取得用）        */
    int tblLen = MAX_RECLEN;             /* 動的設定用配列サイズ */
    int avoidanceFlg = 0;      /* 欠落回避処理実施フラグ */
    int fileEofFlg = 0;        /* ファイルEOFフラグ */
/* MOD 2015.05.11 K.Hasunuma END */

    char *fld_p;                /* フィールドポインタ       */
    char sep_key_tbl[10];       /* セパレートキー           */
/* DEL 2015.05.11 K.Hasunuma キー項目の桁数に合わせて配列サイズを決めるため、定義場所を移動 START */
//    char fld_cut[400][MAX_RECLEN];      /* フィールドデータＴＢＬ   */
/* DEL 2015.05.11 K.Hasunuma END */
    FILE *fp;                   /* ファイルポインタ         */

    /*  引数チェック                                                        */
    if (argc < ARGC_MIN) {
        usage();
    }

    /* ADD 2014.12.16 Phuong.VM */
    // -フィールド数
    if (argv[loop_cnt][0] == '-') {
        char *field_no_opt = argv[loop_cnt];
        field_no_opt++;
        iFN = atoi(field_no_opt);
        if (iFN < 1) {
            puts("指定フィールド数が正しくありません");
            exit(ARG_ERR);
        }
        loop_cnt++;
    }
    /* ADD 2014.12.16 Phuong.VM */

    /*  区切り文字取得                                                      */
    if (NULL != strstr(argv[loop_cnt], OPT_SEP_KEY)) {
        strcpy(read_tbl, argv[loop_cnt]);
        strcpy(sep_key_tbl, strtok(read_tbl, OPT_SEP_KEY));
        loop_cnt++;
    } else {
        strcpy(sep_key_tbl, SPA);
    }

    /*  ＫＥＹ項目数取得                                                    */
    if (NULL != strstr(argv[loop_cnt], OPT_TRN_KEY)) {
        strcpy(read_tbl, argv[loop_cnt]);
        fld_key = atoi(strtok(read_tbl, OPT_TRN_KEY));
        loop_cnt++;
    }

    if (argc > loop_cnt) {
        /*ファイルオープン */
        fp = fopen(argv[loop_cnt], "r");
        if (NULL == fp) {
            fprintf(stderr, "File Open Error\n");
            return FOPEN_ERR;
        }
    } else {
        fp = stdin;
    }

    /*////////////////////////////////////////////////////////////////////// */
    /*  メイン処理                                                          */
    /*////////////////////////////////////////////////////////////////////// */

    while (1) {
        /*  リード処理                                                      */
        /* MOD 2015.05.11 K.Hasunuma START 1レコード全データ取得の為、配列を再定義できる形に変更 */
        /* if (NULL == fgets(read_tbl, MAX_RECLEN, fp)){ */
        /*         break; */
        /* } */
        if (NULL == fgets(read_tbl_work, MAX_RECLEN, fp)){
            if(avoidanceFlg == 0) {
                break;
            } else {
                // サイズ超過による欠落回避処理ループ中なのでそれまでに取得した分を反映させるため終わらせない */
                fileEofFlg = 1;
            }
        } else {
            /* 1行毎に使用する変数の初期化及び取得したワークデータの反映 */
            if (avoidanceFlg == 0) {
                // 配列の初期化
                if (read_tbl != NULL) {
                    free(read_tbl);
                    read_tbl = NULL;
                }
                read_tbl = (char *)malloc( sizeof( char ) * MAX_RECLEN );

                // メモリ割り当てに成功したか
                if(read_tbl == NULL) {
                    // メモリ確保失敗
                    fprintf(stderr, "MALLOC_ERR：MEMORY ALLOCATION FAILURE FOR ARRAY(read_tbl)[MAX_RECLEN]【2】.\n");
                    // ファイルクローズの上、強制終了
                    fclose(fp);
                    exit(MALLOC_ERR);
                }
                tblLen = MAX_RECLEN;
                // 今回取得したデータを拡張した配列にコピー
                strcpy(read_tbl, read_tbl_work);
            } else {
                // サイズ超過による欠落回避処理ループ中なので配列拡張
                // 配列拡張の前にそれまでの取得データを退避
                char* read_tbl_tmp = (char *)malloc( sizeof( char ) * tblLen );  /* 退避用ＴＢＬ           */
                if(read_tbl_tmp == NULL) {
                    // 退避用メモリ確保失敗
                    fprintf(stderr, "MALLOC_ERR：MEMORY ALLOCATION FAILURE FOR ARRAY(read_tbl_tmp)[");
                    char tblLenChar[20];
                    sprintf(tblLenChar, "%d", tblLen);
                    fprintf(stderr, tblLenChar);
                    fprintf(stderr, "].\n");
                    // キー指定されている場合は、メモリ超過発生行のキー情報を出力する
                    if (0 != fld_key) {
                        fprintf(stderr, "エラー対象レコードキー ：");
                        /*  項目切り出し（エラー行キー項目出力用）                  */
                        int i = 0;
                        for (i = 0; fld_key > i; i++) {
                            fprintf(stderr, " ");
                            if (0 == i) {
                                fprintf(stderr, strtok(strtok(read_tbl, RTN), sep_key_tbl));
                            } else {
                                fprintf(stderr, strtok(NULL, sep_key_tbl));
                            }
                        }
                        fprintf(stderr, "\n");
                    }

                    // メインの配列を念のため、開放
                    if (read_tbl != NULL) {
                        free(read_tbl);
                        read_tbl = NULL;
                    }
                    // ファイルクローズの上、強制終了
                    fclose(fp);
                    exit(MALLOC_ERR);
                }
                strcpy(read_tbl_tmp, read_tbl);

                // 拡張サイズでメモリ割り当て
                tblLen += MAX_RECLEN;
                if (read_tbl != NULL) {
                    free(read_tbl);
                    read_tbl = NULL;
                }
                read_tbl = (char *)malloc( sizeof( char ) * tblLen );

                // メモリ割り当てに成功したか
                if(read_tbl == NULL) {
                    // メモリ確保失敗
                    fprintf(stderr, "MALLOC_ERR：MEMORY ALLOCATION FAILURE FOR ARRAY(read_tbl)[");
                    char tblLenChar[20];
                    sprintf(tblLenChar, "%d", tblLen);
                    fprintf(stderr, tblLenChar);
                    fprintf(stderr, "].\n");
                    // キー指定されている場合は、メモリ超過発生行のキー情報を出力する
                    if (0 != fld_key) {
                        fprintf(stderr, "エラー対象レコードキー ：");
                        /*  項目切り出し（エラー行キー項目出力用）                  */
                        int i = 0;
                        for (i = 0; fld_key > i; i++) {
                            fprintf(stderr, " ");
                            if (0 == i) {
                                fprintf(stderr, strtok(strtok(read_tbl_tmp, RTN), sep_key_tbl));
                            } else {
                                fprintf(stderr, strtok(NULL, sep_key_tbl));
                            }
                        }
                        fprintf(stderr, "\n");
                    }

                    // 退避データを拡張した配列を念のため、開放
                    if (read_tbl_tmp != NULL) {
                        free(read_tbl_tmp);
                        read_tbl_tmp = NULL;
                    }
                    // ファイルクローズの上、強制終了
                    fclose(fp);
                    exit(MALLOC_ERR);
                }

                // 退避データを拡張した配列にコピー
                strcpy(read_tbl, read_tbl_tmp);
                if (read_tbl_tmp != NULL) {
                    free(read_tbl_tmp);
                    read_tbl_tmp = NULL;
                }
                // 今回取得したデータを退避データを拡張した配列に追記
                strcat(read_tbl, read_tbl_work);
            }

            /*  読み込まれたデータが欠落（MAX_RECLENを超過）していないか        */
            // 改行文字が含まれているかの確認
            if (strchr(read_tbl_work, '\n') == NULL) {
                // 改行文字が含まれていない＝１行の全データが取得できていないので
                // もう一度fgetsさせる
                avoidanceFlg = 1;
                continue;
            }
        }

        // １行の最後まで文字を取得完了、後続の区切り処理実施
        avoidanceFlg = 0;
        /* ADD 2015.05.11 K.Hasunuma END */

        /*  キー項目有り                                                    */
        if (0 != fld_key) {

            /* ADD 2015.05.11 K.Hasunuma START */
            char fld_cut[fld_key][MAX_RECLEN];      // 従来は[400][MAX_RECLEN]で定義していたが、400の部分はキー項目数分だけあれば十分
            /* ADD 2015.05.11 K.Hasunuma END */

            /*  項目切り出し                                                */
            for (loop_cnt = 0; fld_key > loop_cnt; loop_cnt++) {
                /* MOD 2015.05.11 K.Hasunuma セグメンテーション違反発生リスク軽減 START */
                //if (0 == loop_cnt) {
                //    strcpy(cp_tbl, strtok(strtok(read_tbl, RTN), sep_key_tbl));
                //} else {
                //    strcpy(cp_tbl, strtok(NULL, sep_key_tbl));
                //}
                //strcpy(fld_cut[loop_cnt], cp_tbl);
                if (0 == loop_cnt) {
                    strcpy(fld_cut[0], strtok(strtok(read_tbl, RTN), sep_key_tbl));
                } else {
                    strcpy(fld_cut[loop_cnt], strtok(NULL, sep_key_tbl));
                }
                /* MOD 2015.05.11 K.Hasunuma END */
            }

            /*  残項目表示                                                  */
            /* DEL 2014.12.16 Phuong.VM---
               while (NULL != (fld_p = strtok(NULL, sep_key_tbl))){
               for(loop_cnt = 0; fld_key > loop_cnt; loop_cnt++){
               if (0 == loop_cnt){
               fprintf(stdout, "%s", fld_cut[loop_cnt]);
               }else{
               fprintf(stdout, "%s%s", sep_key_tbl, fld_cut[loop_cnt]);
               }
               }
               fprintf(stdout, "%s%s\n", sep_key_tbl, fld_p);
               } DEL 2014.12.16 Phuong.VM--- */

            /* ADD 2014.12.16 Phuong.VM */
            int cnt = 0;

            while (NULL != (fld_p = strtok(NULL, sep_key_tbl))) {
                cnt = 1;
                for (loop_cnt = 0; fld_key > loop_cnt; loop_cnt++) {
                    if (0 == loop_cnt) {
                        fprintf(stdout, "%s", fld_cut[loop_cnt]);
                    } else {
                        fprintf(stdout, "%s%s", sep_key_tbl, fld_cut[loop_cnt]);
                    }
                }
                fprintf(stdout, "%s%s", sep_key_tbl, fld_p);
                while (iFN > cnt) {
                    if (NULL != (fld_p = strtok(NULL, sep_key_tbl))) {
                        fprintf(stdout, "%s%s", sep_key_tbl, fld_p);
                        cnt++;
                    } else {
                        break;
                    }
                }
                fprintf(stdout, "\n");
            }
            /* ADD 2014.12.16 Phuong.VM */
        }
        /*  キー項目無し                                                    */
        else {

            while (1) {
                if ((fld_p = strtok(fld_key ? NULL : read_tbl, FLD_SEP_STR)) == NULL) {
                    fld_key = 0;
                    break;
                }
                fprintf(stdout, "%s", fld_p);   /* DEL [\n] 2014.12.17 Phuong.VM */
                fld_key++;

                /* ADD 2014.12.17 Phuong.VM */
                int cnt = 1;
                while (iFN > cnt) {
                    if (NULL != (fld_p = strtok(NULL, FLD_SEP_STR))) {
                        fprintf(stdout, "%s%s", sep_key_tbl, fld_p);
                        cnt++;
                    } else {
                        break;
                    }
                }
                fprintf(stdout, "\n");
                /* ADD 2014.12.17 Phuong.VM */
            }
        }

        /* ADD 2015.05.11 K.Hasunuma fileEofFlgがONの場合は離脱 START */
        if (fileEofFlg == 1) {
            break;
        }
        /* ADD 2015.05.11 K.Hasunuma END */
    }

    /* ADD 2015.05.11 K.Hasunuma 取得したポインタをクリア START */
    if (read_tbl != NULL) {
        free(read_tbl);
        read_tbl = NULL;
    }
    /* ADD 2015.05.11 K.Hasunuma END */

    /*  ファイルクローズ                                                    */
    fclose(fp);
    return NRML_END;
}

/****************************************************************************/
/*      関数名称：     usage（使用方法表示）                                */
/*      処理概要：     エラー処理                                           */
/*      戻り値  ：     ARG_ERR                                              */
/****************************************************************************/
void usage()
{
    puts("使用方法:    v_vertarray [fs=$] pk=$ < inputfile > outputfile");
    exit(ARG_ERR);
}
