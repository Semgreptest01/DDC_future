/****************************************************************************/
/*  システム名称：  汎用ＣＩＳツール                                        */
/*  コマンド名称：  v_s1match                                             */
/*  機能        ：  シーケンシャルマッチング                                */
/*　　・指定したフィールドをキーとし、マスターとトランザクションのマッチ    */
/*      ングを行い、マッチしたトランザクションのレコードのみ出力する        */
/*      マッチしないトランは”+stderr”オプションでエラー出力が可能         */
/*　　・複数のフィールドをキーに選択できる                                  */
/*  制約事項    ：                                                          */
/*　　・インプットはフィールドセパレータは半角スペースとする。              */
/*　　・インプットのレコード（行）レングスはＬＦを含めて最大６１４４        */
/*      バイトとする。                                                      */
/*  使用法      ：  v_s1match [+stderr] pk=$$ master < tran > output      */
/*  ステータス  ：  0 : 正常終了                                            */
/*                  1 : 警告終了（出力失敗データ有り）                      */
/*                  2 : アーギュメント不正                                  */
/*                  3 : ファイルオープンエラー                              */
/*                  4 : ファイルリードエラー                                */
/*                  5 : メモリ確保エラー                                    */
/*  作成者      ：  木下                                                    */
/*  作成日      ：  2001/10/18                                              */
/*  修正履歴    ：                                                          */
/*  No. 修正日付    修正者            修正内容                              */
/*  001 2002/01/25  木下    マッチング処理及び出力処理のバグ対応            */
/*　002 2002/11/21　川村 　 最大レコードサイズ変更(1024Byte=>3072byte)      */
/*                          １レコード最大フィールド数を４００に変更        */
/*　003 2002/11/28　川村 　 マッチングキーソート順指定対応。                */
/*                          マッチングキー比較文字数指定対応。              */
/*　004 2003/02/05　寺澤　　　 最大レコードサイズ変更(3072Byte=>6144byte)   */
/****************************************************************************/
/*--------------------------------------------------------------------------*/
/*  ヘッダーファイルインクルード定義                                        */
/*--------------------------------------------------------------------------*/
/*  System Headder File Include                                             */
#include <string.h>
#include <stdlib.h>

/*  User Headder File Include                                               */
#include "libdistool.inc"

/*--------------------------------------------------------------------------*/
/*  ｄｅｆｉｅｎ定義                                                        */
/*--------------------------------------------------------------------------*/
#define     MIN_ARGS        3   /* 最小引数の数                 */
#define     MAX_ARGS        5   /* 最大引数の数                 */
#define     FS_HIFUN        "-" /* 連続パラメータ記号           */
#define     FS_COMMA        "," /* 個別パラメータ記号           */
#define     FS_DOT          "." /* 文字数指定パラメータ記号     */
#define     FS_SPACE        " " /* フィールドセパレータ         */
#define     MAX_FSNUM       400 /* 最大フィールド数             */
#define     MAX_FSLEN       1024        /* 最大フィールドサイズ         */

/*--------------------------------------------------------------------------*/
/*  構造体定義                                                              */
/*--------------------------------------------------------------------------*/
typedef struct {
    int break_flg;
    int fild_cnt;
    char cat_key[MAX_RECLEN];
    char all_rec[MAX_RECLEN];
} rec_split;

typedef struct {
    int order;                  /* 順序指定用キー情報          */
    int cutnum;                 /* キー比較文字数（先頭から）  */
} prim_key;

typedef struct {
    int code;
    char message[80];
} return_data;

/*--------------------------------------------------------------------------*/
/*  内部変数定義                                                            */
/*--------------------------------------------------------------------------*/
prim_key key_data[MAX_FSNUM] = { 0, 0 };

return_data rtn_status[80] = {
    {3, "マスターファイルオープンエラー"},
    {3, "トランファイルオープンエラー"},
    {4, "マスターリードエラー"},
    {4, "トランザクションーリードエラー"},
    {6, "パラメータ値は文字は使用しないで下さい"},
    {7, "マッチング処理に失敗しました"},
    {8, "アウトプット処理に失敗しました"}
};

int key_tbl[MAX_FSNUM] = { 0 }; /* パラメータＴＢＬ             */
int opt_flg = 0;                /* オプションフラグ[有：1]      */
int pk_cnt = 0;                 /* パラメータＴＢＬ要素数       */

/*--------------------------------------------------------------------------*/
/*  内部関数定義                                                            */
/*--------------------------------------------------------------------------*/
int pk_parm_get(char *argv);
int match(FILE * mas_fp, FILE * trn_fp);
int m_read(FILE * mas_fp, rec_split * mst_split);
int t_read(FILE * trn_fp, rec_split * trn_split);
int f_split(char *rec_buf, char *cat_char, int *mt_flg);
int icmp(const void *key1, const void *key2);
void usage();

#ifdef  NEW_TOKEN
char *new_strtok(char *src, char *dc, int *len);
#endif

/****************************************************************************/
/*      関数名称：                                                          */
/*      処理概要：                                                          */
/*      入  力  ：                                                          */
/*      入  力  ：                                                          */
/*      戻り値  ：                                                          */
/****************************************************************************/
main(int argc, char *argv[])
{
    int sts;                    /* ステータス値                 */
    FILE *mas_fp;               /* マスターファイルポインタ     */
    FILE *trn_fp;               /* トランファイルポインタ       */
    /* ADD 2014.12.15 Chin.TV */
    int stdin_check = 0;
    /* ADD 2014.12.15 Chin.TV */
    /*  引数チェック                                                        */
    if (argc < MIN_ARGS || MAX_ARGS < argc) {
        usage();
    }

    /*  オプションチェック                                                  */
    if (strstr(argv[1], OPT_UNMATCH_OUT) != NULL) {
        opt_flg = 1;
    }

    /*  パラメータキー取得                                                  */
    if (pk_parm_get(strtok(argv[1 + opt_flg], OPT_TRN_KEY))) {
        usage();
    }

    /*  マスターファイルパス取得－ファイルオープン                          */
    /* ADD 2014.12.15 Chin.TV */
    if ((argc > (2 + opt_flg)) && strcmp(argv[2 + opt_flg], "-") == 0) {
        stdin_check = 1;
        mas_fp = stdin;
    } else {
        /* ADD 2014.12.15 Chin.TV */
        if ((mas_fp = fopen(argv[2 + opt_flg], "r")) == NULL) {
            fprintf(stderr, "%s\n", rtn_status[0].message);
            exit(rtn_status[0].code);
        }
    }

    /*  トランザクション－ファイルオープン                                  */
    if (argc == 4 + opt_flg) {
        /* ADD 2014.12.15 Chin.TV */
        if (strcmp(argv[3 + opt_flg], "-") == 0) {
            if (stdin_check == 0) {
                trn_fp = stdin;
            } else {
                puts("異常終了 標準入力から複数指定することができません");
                exit(ARG_ERR);
            }
        } else {
            /* ADD 2014.12.15 Chin.TV */
            if ((trn_fp = fopen(argv[3 + opt_flg], "r")) == NULL) {
                fprintf(stderr, "%s\n", rtn_status[1].message);
                exit(rtn_status[1].code);
            }
        }
    } else {
        /* DEL 2014.12.15 Chin.TV */
        //trn_fp = stdin;
        /* DEL 2014.12.15 Chin.TV */
        /* ADD 2014.12.15 Chin.TV */
        if (stdin_check == 0) {
            trn_fp = stdin;
        } else {
            puts("異常終了 標準入力から複数指定することができません");
            exit(ARG_ERR);
        }
        /* ADD 2014.12.15 Chin.TV */
    }

    /*  マッチング処理                                                      */
    if ((sts = match(mas_fp, trn_fp)) != 0) {
        fprintf(stderr, "%s\n", rtn_status[sts].message);
        exit(rtn_status[sts].code);
    }

    /*  ファイルクローズ                                                    */
    fclose(mas_fp);
    fclose(trn_fp);

    return NRML_END;
}

/****************************************************************************/
/*      関数名称：          pk_parm_get                                     */
/*      処理概要：          "pk="に設定しているパラメータを取得する         */
/*      入  力  ：          入力文字列                                      */
/*      出  力  ：          パラメータＴＢＬ                                */
/*      戻り値  ：          NRML_END／ARG_ERR                               */
/****************************************************************************/
int pk_parm_get(char *argv)
{
    int loop_cnt = 0;           /* ループカウンタ                   */
    int fld_s;                  /* 複数キーのスタート値             */
    int fld_e;                  /* 複数キーのエンド値               */
    char *pk_point;             /* パラメータポインタ               */
    char pk_buf[80];            /* パラメータ値                     */
    char item_tbl[100][20];     /* フィールドアイテム格納ＴＢＬ     */
    int item_cnt = 0;           /* フィールドアイテム要素カウンタ   */
    int icnt = 0;               /* アイテムカウンタ                 */

    strcpy(pk_buf, argv);
/*  printf("pk_buf=%s\n", pk_buf);                        ** DEBUG */

    /*  [，]付引数が場合、アイテム別に分割する                              */
    if (strstr(pk_buf, FS_COMMA) != NULL) {
        pk_point = strtok(pk_buf, FS_COMMA);
        strcpy(item_tbl[item_cnt++], pk_point);
        while ((pk_point = strtok(NULL, FS_COMMA)) != NULL) {
            strcpy(item_tbl[item_cnt++], pk_point);
        }
    } else {
        strcpy(item_tbl[item_cnt++], pk_buf);
    }

    /*  アイテムからフィールド情報を抽出する                                */
    for (icnt = 0; item_cnt - 1 >= icnt; icnt++) {
        /*  [－]付きの引数                                                  */
        if (strstr(item_tbl[icnt], FS_HIFUN) != NULL) {
            fld_s = atoi(strtok(item_tbl[icnt], FS_HIFUN));
            fld_e = atoi(strtok(NULL, FS_HIFUN));
/*          printf("fld=[%d - %d]\n", fld_s,fld_e);       ** DEBUG */

            if (fld_s < 1 || fld_e < 1) {
                return ARG_ERR;
            }
            for (loop_cnt = fld_s; loop_cnt <= fld_e; loop_cnt++) {
                key_tbl[pk_cnt] = loop_cnt;
                key_data[pk_cnt].order = loop_cnt;
                key_data[pk_cnt].cutnum = 0;
                pk_cnt++;
            }
        }
        /*  [.]付きの引数                                                   */
        else if (strstr(item_tbl[icnt], FS_DOT) != NULL) {
            key_data[pk_cnt].order = atoi(strtok(item_tbl[icnt], FS_DOT));
            key_data[pk_cnt].cutnum = atoi(strtok(NULL, FS_DOT));
            key_tbl[pk_cnt] = key_data[pk_cnt].order;
            if (key_data[pk_cnt].order < 1) {
                return ARG_ERR;
            }
            pk_cnt++;
        } else {
            key_tbl[pk_cnt] = atoi(item_tbl[icnt]);
            key_data[pk_cnt].order = atoi(item_tbl[icnt]);
            key_data[pk_cnt].cutnum = 0;
            if (key_data[pk_cnt].order < 1) {
                return ARG_ERR;
            }
            pk_cnt++;
        }
/*      printf("KEY-ORDER[%d]=%d\n",                             ** DEBUG */
/*             pk_cnt-1, key_data[pk_cnt-1].order);              ** DEBUG */
/*      printf("KEY-CUTNUM[%d]=%d\n",                            ** DEBUG */
/*             pk_cnt-1, key_data[pk_cnt-1].cutnum);             ** DEBUG */
    }
/* DEBUG */
/*
    printf("pk_cnt=%d\n", pk_cnt);
    for(icnt = 0; icnt < pk_cnt; icnt++){
        printf("KEY-ORDER[%d]=%d\n", icnt, key_data[icnt].order);
        printf("KEY-TBL[%d]=%d\n", icnt, key_tbl[icnt]);
    }
*/
/* DEBUG */
    /* キーの最小値と出力順を得るためソートする                             */
    qsort(key_tbl, (size_t) pk_cnt, sizeof(int), icmp);

/* DEBUG */
/*
    printf("pk_cnt=%d\n", pk_cnt);
    for(icnt = 0; icnt < pk_cnt; icnt++){
        printf("KEY-TBL[%d]=%d\n", icnt, key_tbl[icnt]);
    }
*/
/* DEBUG */

    return NRML_END;
}

/****************************************************************************/
/*      関数名称：      match                                               */
/*      処理概要：      トランザクションとマスターのマッチング行う          */
/*      入  力  ：      キーＴＢＬ                                          */
/*      入  力  ：      マスターファイルポインタ                            */
/*      入  力  ：      トランザクションファイルポインタ                    */
/*      戻り値  ：      NRML_END／                                          */
/****************************************************************************/
int match(FILE * mas_fp, FILE * trn_fp)
{
    rec_split mst_split;        /* マスターレコード構造体             */
    rec_split trn_split;        /* トランザクションレコード構造体     */
    int break_cnt;              /* キーブレイク値                     */

    /*  マスターリード処理                                                  */
    if (m_read(mas_fp, &mst_split)) {
        return 2;
    }

    /*  トランザクションリード処理                                          */
    if (t_read(trn_fp, &trn_split)) {
        return 3;
    }

    /*  マッチング処理                                                      */
    while (trn_split.break_flg != 1) {

        /*  キーブレイクチェック                                            */
        break_cnt = strcmp(mst_split.cat_key, trn_split.cat_key);

        /*  マスターファイルＥＯＦチェック                                  */
        if (mst_split.break_flg == 1) {
            break_cnt = 1;
        }

        if (break_cnt < 0) {
            /*  マスターリード処理                                          */
            if (m_read(mas_fp, &mst_split)) {
                return 2;
            }
        } else if (break_cnt > 0) {
            /*  オプション指定があればエラーとして出力する                  */
            if (opt_flg != 0) {
                /*  出力（トランオンリー）                                  */
#ifdef  NEW_TOKEN
                fwrite(trn_split.all_rec, strlen(trn_split.all_rec), 1, stderr);
#else
                fprintf(stderr, "%s", trn_split.all_rec);
#endif
            }

            /*  トランザクションリード処理                                  */
            if (t_read(trn_fp, &trn_split)) {
                return 3;
            }
        } else {
            /*  出力（マッチ）                                              */
#ifdef  NEW_TOKEN
            fwrite(trn_split.all_rec, strlen(trn_split.all_rec), 1, stdout);
#else
            fprintf(stdout, "%s", trn_split.all_rec);
#endif

            /*  トランザクションリード処理                                  */
            if (t_read(trn_fp, &trn_split)) {
                return 3;
            }
        }
    }
    return NRML_END;
}

/****************************************************************************/
/*      関数名称：      m_read                                              */
/*      処理概要：      マスターリード処理                                  */
/*      入  力  ：      ファイルポインタ                                    */
/*      出  力  ：      フィールド格納構造体                                */
/*      入  力  ：      パラメータキー                                      */
/*      戻り値  ：      NRML_END／FREAD_ERR                                 */
/****************************************************************************/
int m_read(FILE * mas_fp, rec_split * mst_split)
{
#ifndef NEW_TOKEN
    char master_buf[MAX_RECLEN];        /* レコードバッファ               */
#endif
    int master_flg = 1;         /* マスタ用のフィールド分割フラグ */
    /* トラン用：0                    */

#ifdef  NEW_TOKEN
    if (fgets(mst_split->all_rec, MAX_RECLEN, mas_fp) != NULL) {
        if (f_split(mst_split->all_rec, &mst_split->cat_key[0], &master_flg)) {
            return FREAD_ERR;
        }
        mst_split->break_flg = 0;
    }
#else
    if (fgets(master_buf, MAX_RECLEN, mas_fp) != NULL) {
        strcpy(mst_split->all_rec, master_buf);
        if (f_split(master_buf, &mst_split->cat_key[0], &master_flg)) {
            return FREAD_ERR;
        }
        mst_split->break_flg = 0;
    }
#endif
    else {
        if (ferror(mas_fp)) {
            return FREAD_ERR;
        }
        mst_split->break_flg = 1;
    }
    return NRML_END;
}

/****************************************************************************/
/*      関数名称：      t_read                                              */
/*      処理概要：      トランリード処理                                    */
/*      入  力  ：      ファイルポインタ                                    */
/*      出  力  ：      フィールド格納構造体                                */
/*      入  力  ：      パラメータキー                                      */
/*      戻り値  ：      NRML_END／FREAD_ERR                                 */
/****************************************************************************/
int t_read(FILE * trn_fp, rec_split * trn_split)
{
#ifndef NEW_TOKEN
    char tran_buf[MAX_RECLEN];  /* レコードバッファ               */
#endif
    int master_flg = 0;         /* トラン用のフィールド分割フラグ */
    /* マスタ用：1                    */

#ifdef  NEW_TOKEN
    if (fgets(trn_split->all_rec, MAX_RECLEN, trn_fp) != NULL) {
        if (f_split(trn_split->all_rec, &trn_split->cat_key[0], &master_flg)) {
            return FREAD_ERR;
        }
        trn_split->break_flg = 0;
    }
#else
    if (fgets(tran_buf, MAX_RECLEN, trn_fp) != NULL) {
        strcpy(trn_split->all_rec, tran_buf);
        if (f_split(tran_buf, &trn_split->cat_key[0], &master_flg)) {
            return FREAD_ERR;
        }
        trn_split->break_flg = 0;
    }
#endif
    else {
        if (ferror(trn_fp)) {
            return FREAD_ERR;
        }
        trn_split->break_flg = 1;
    }
    return NRML_END;
}

/****************************************************************************/
/*      関数名称：      f_split                                             */
/*      処理概要：      フィールドスプリット                                */
/*      入  力  ：      入力レコード                                        */
/*      出  力  ：      フィールド格納構造体                                */
/*      入  力  ：      パラメータキー                                      */
/*      戻り値  ：      NRML_END／                                          */
/****************************************************************************/
int f_split(char *rec_buf, char *cat_char, int *mt_flg)
{
    int loop_cnt = 0;           /* ループカウンタ               */
    int cat_fld_cnt = 0;        /* 連結キーカウンタ             */
    char *char_point;           /* 文字列ポインタ               */
    char wk_buf[MAX_FSNUM][MAX_FSLEN];  /* マスターフィールドＴＢＬ     */
#ifdef  NEW_TOKEN
    int flen;
#endif

    /*  初期処理                                                            */
    cat_char[0] = '\0';

    /*  フィールド分割                                                      */
    while (1) {
#ifdef  NEW_TOKEN
        if ((char_point =
             new_strtok(cat_fld_cnt ? NULL : rec_buf, FLD_SEP_STR, &flen)) == NULL) {
            break;
        }
        memcpy(wk_buf[cat_fld_cnt], char_point, flen);
        wk_buf[cat_fld_cnt][flen] = 0;
#else
        if ((char_point = strtok(cat_fld_cnt ? NULL : rec_buf, FLD_SEP_STR)) == NULL) {
            break;
        }
        strcpy(wk_buf[cat_fld_cnt], char_point);
#endif
        cat_fld_cnt++;
    }
/*  printf("cat_fld_cnt=%d\n",cat_fld_cnt);                   ** DEBUG */

    /*  トランザクション用                                                  */
    for (loop_cnt = 0; loop_cnt < pk_cnt; loop_cnt++) {
        if (!*mt_flg) {
/*          printf("key_data[%d].order=%d\n", loop_cnt,            ** DEBUG */
/*                  key_data[loop_cnt].order);                     ** DEBUG */
            /* キー比較文字数指定判定                                       */
            if (key_data[loop_cnt].cutnum == 0) {
                strcat(cat_char, wk_buf[key_data[loop_cnt].order - 1]);
            } else {
                /* 比較文字数が指定されていれば、該当文字数分コピー         */
                strncat(cat_char, wk_buf[key_data[loop_cnt].order - 1],
                        key_data[loop_cnt].cutnum);
            }
/*          printf("trn_char=%s\n",cat_char);                 ** DEBUG */
        }
        /*  マスター用                                                      */
        else {
/*          printf("key_tbl[1]=%d\n", key_tbl[0]);            ** DEBUG */
            /* キー比較文字数指定判定                                       */
            if (key_data[loop_cnt].cutnum == 0) {
                strcat(cat_char, wk_buf[key_data[loop_cnt].order - key_tbl[0]]);
            } else {
                /* 比較文字数が指定されていれば、該当文字数分コピー         */
                strncat(cat_char, wk_buf[key_data[loop_cnt].order - key_tbl[0]],
                        key_data[loop_cnt].cutnum);
            }
/*          printf("mst_char=%s\n",cat_char);                 ** DEBUG */
        }
    }
    return NRML_END;
}

/****************************************************************************/
/*  関数名称：  icmp                                                        */
/*  処理概要：  qsort() 用整数比較関数                                      */
/*  入  力  ：  比較データ１                                                */
/*  入  力  ：  比較データ２                                                */
/*  戻り値  ：  比較結果（0 ... 等しい、正 ... key1>key2、負 ... key1<key2）*/
/****************************************************************************/
int icmp(const void *key1, const void *key2)
{
    return (*(int *) key1 - *(int *) key2);
}

/****************************************************************************/
/*      関数名称：      usage（使用方法表示）                               */
/*      処理概要：      エラー処理                                          */
/*      戻り値  ：      ARG_ERR                                             */
/****************************************************************************/
void usage()
{
    puts("使用方法: v_s1match [+stderr] pk=$$ masterfile < tranfile > outputfile");
    exit(ARG_ERR);
}

#ifdef  NEW_TOKEN
/****************************************************************************/
/*  関数名称：  new_strtok（トークン取得）                                  */
/*  処理概要：  strtokの代替関数。                                          */
/*              指定文字列中のトークン先頭アドレスとトークン長を取得する    */
/*              トークンが検出されない場合は、NULL を返却する。             */
/*  入  力  ：  src      ：  検索対象文字列先頭アドレス                     */
/*  入  力  ：  dc       ：  デリミッタ文字列先頭アドレス                   */
/*  入  力  ：  len      ：  トークン長返却先アドレス                       */
/*  戻り値  ：  トークン先頭アドレス                                        */
/*                                                                          */
/*  strtok 外部仕様からの変更点/制限事項                                    */
/*      ・ソース文字列を本関数内で変更しない。この仕様により、              */
/*        後々ソース文字列が必要な場合も退避する必要が無くなる。            */
/*        コンスタント領域を処理対象にすることも可能となる。                */
/*      ・トークン長を指定変数に返却する。事後にトークン長算出不要となる    */
/*      ・デリミッタ文字は、１種類のみに限定。制限事項。                    */
/*  変更に伴う注意点                                                        */
/*      ・返却するアドレスは、ソース文字列中のアドレスを指しており、        */
/*        独立したトークン文字列のポインターで無い事に注意。トークン末尾に  */
/*        0 は付加されていない。                                            */
/*      ・デリミッタの指定は、strtok と置き換える都合上、文字列として       */
/*        受け取るが、有効な文字は、先頭1Byteのみである。                   */
/*        処理簡略化(速度確保)の為の制限。複数のデリミッタ指定はできない。  */
/*      ・1行イメージの文字列に対する処理ケースは多いが、" \n" のように     */
/*        複数のデリミッタを指定できないため、行末改行処理を例外として      */
/*        扱っている。最終トークンの末尾が改行コードだった場合は、改行      */
/*        コードは、トークン長に含まない。                                  */
/*        1行イメージ処理向けの例外の為、文字列途中の改行コードは考慮されて */
/*        いない。トークンの一部として扱われることに注意。                  */
/****************************************************************************/
char *new_strtok(char *src, char *dc, int *len)
{
    static char *sptr = NULL;
    static int d_code = 0;
    char *rptr = NULL;

    if (dc == NULL || dc[0] == 0 || len == NULL)
        return NULL;
    if (src == NULL) {
        if (sptr == NULL)
            return NULL;
        if (d_code == (int) (dc[0])) {
            sptr++;
        } else {
            d_code = (int) (dc[0]);
        }
    } else {
        sptr = src;
        d_code = (int) (dc[0]);
    }

    /* デリミッタ連続検出はスキップする */
    for (; *sptr == d_code; sptr++);
/* デリミッタ連続を空トークンとして検出したい場合(strsep相当)このスキップを削除する(未テスト) */

    if (*sptr) {
        rptr = sptr;
        sptr = strchr(sptr, d_code);
        if (sptr) {
            *len = sptr - rptr;
        } else {
            *len = strlen(rptr);
            if (rptr[*len - 1] == '\n')
                *len -= 1;
        }
    }

    return rptr;
}
#endif
