/****************************************************************************/
/*  システム名称：  汎用ＣＩＳツール                                        */
/*  コマンド名称：  v_s3match                                             */
/*  機能        ：  シーケンシャルマッチングかけはぎ                        */
/*　　・指定したフィールドをキーとし、マスターとトランザクションのマッチ    */
/*      ングを行い、マッチしたトランザクションのレコードにマスタのレコー    */
/*      ドを連結し、マッチしない場合はマスタのレコードがはいるべき場所に    */
/*      "*"を代入して出力する（トランザクションは全件出力する）             */
/*　　・複数のフィールドをキーに選択できる                                  */
/*  制約事項    ：                                                          */
/*　　・インプットはフィールドセパレータは半角スペースとする。              */
/*　　・インプットのレコード（行）レングスはＬＦを含めて最大６１４４        */
/*      バイトとする。                                                      */
/*    ・アンマッチ時の"*"の桁数はマスタの１レコード目の桁数を基準とする。   */
/*  使用法      ：  v_s3match [+charctor] pk=$$ master < tran > output    */
/*  ステータス  ：  0 : 正常終了                                            */
/*                  1 : 警告終了（出力失敗データ有り）                      */
/*                  2 : アーギュメント不正                                  */
/*                  3 : ファイルオープンエラー                              */
/*                  4 : ファイルリードエラー                                */
/*                  5 : メモリ確保エラー                                    */
/*  作成者        ：  木下                                                  */
/*  作成日        ：  2001/10/18                                            */
/*  修正履歴      ：                                                        */
/*  No.  修正日付   修正者    修正内容                                      */
/*  001 2002/01/25  木下    マッチング処理及び出力処理のバグ対応            */
/*  002 2002/01/28  木下    １レコード目からのトランオンリー時に出力終了    */
/*                          条件がトランのキー以外部分を出力する様に設定    */
/*                          していなかった                                  */
/*　003 2002/11/21　川村 　 最大レコードサイズ変更(1024Byte=>3072byte)。    */
/*                          １レコード最大フィールド数を４００に変更。      */
/*                          トランオンリー時にダミーフィールドが保証され    */
/*                          ない障害を修正。                                */
/*　004 2002/11/28　川村 　 マッチングキーソート順指定対応。                */
/*                          マッチングキー比較文字数指定対応。              */
/*　005 2003/02/05　寺澤　　　 最大レコードサイズ変更(3072Byte=>6144byte)   */
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
#define     MIN_ARGS        3   /* 最小引数の数                     */
#define     MAX_ARGS        5   /* 最大引数の数                     */
#define     OPT_CHAR        '+' /* オプション指定文字               */
#define     DMY_CHAR        '*' /* デフォルトダミー文字             */
#define     FS_HIFUN        "-" /* 連続パラメータ記号               */
#define     FS_COMMA        "," /* 個別パラメータ記号               */
#define     FS_DOT          "." /* 文字数指定パラメータ記号         */
#define     FS_SPACE        " " /* フィールドセパレータ             */
#define     MAX_FSNUM       400 /* 最大フィールド数                 */
#define     MAX_FSLEN       1024        /* 最大フィールドサイズ             */

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

int key_tbl[MAX_FSNUM] = { 0 }; /* パラメータＴＢＬ              */
int opt_flg = 0;                /* オプションフラグ[有：1]       */
int pk_cnt = 0;                 /* パラメータＴＢＬ要素数        */
int mst_output_cnt = 0;         /* マスタ出力カウンタ            */
int mst_otherlen_tbl[MAX_FSNUM] = { 0 };        /* マスタのキー以外のフィールド  */

                                           /*  レングス                     */
char dmy_opt_char[MAX_RECLEN];  /* オプションキャラクター        */
int mst_fld_cnt;                /* マスターフィールドカウンタ    */
int trn_fld_cnt;                /* トランフィールドカウンタ      */
char mst_buf[MAX_FSNUM][MAX_FSLEN];     /* マスターフィールドＴＢＬ      */
char trn_buf[MAX_FSNUM][MAX_FSLEN];     /* トランフィールドＴＢＬ        */

/*--------------------------------------------------------------------------*/
/*  内部関数定義                                                            */
/*--------------------------------------------------------------------------*/
int pk_parm_get(char *argv);
int match(FILE * mas_fp, FILE * trn_fp);
int m_read(FILE * mas_fp, rec_split * mst_split);
int t_read(FILE * trn_fp, rec_split * trn_split);
int f_split(char *rec_buf, char *cat_char, int *mt_flg);
int display(char *mst_all_rec, char *trn_all_rec);
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
    int sts, stdin_check = 0;   /* ステータス値                  */
    FILE *mas_fp;               /* マスターファイルポインタ      */
    FILE *trn_fp;               /* トランファイルポインタ        */

    /*  引数チェック                                                        */
    if (argc < MIN_ARGS || MAX_ARGS < argc) {
        usage();
    }

    /*  オプションチェック                                                  */
    if (strchr(argv[1], OPT_CHAR) != NULL) {
        strcpy(dmy_opt_char, strrchr(argv[1], OPT_CHAR) + 1);
        opt_flg = 1;
    } else {
        dmy_opt_char[1] = '\0';
        memset(dmy_opt_char, DMY_CHAR, 1);
    }

    /*  パラメータキー取得                                                  */
    if (pk_parm_get(strtok(argv[1 + opt_flg], OPT_TRN_KEY))) {
        usage();
    }

    /*  マスターファイルパス取得－ファイルオープン                          */
    /* ADD 2014.12.15 Phuong.VM */
    if ((argc > 2 + opt_flg) && strcmp(argv[2 + opt_flg], "-") == 0) {
        stdin_check = 1;
        mas_fp = stdin;
    }
    /* ADD 2014.12.15 Phuong.VM */
    else if ((mas_fp = fopen(argv[2 + opt_flg], "r")) == NULL) {
        fprintf(stderr, "%s\n", rtn_status[0].message);
        exit(rtn_status[0].code);
    }

    /*  トランザクション－ファイルオープン                                  */
    if (argc == 4 + opt_flg) {
        /* ADD 2014.12.15 Phuong.VM */
        if (strcmp(argv[3 + opt_flg], "-") == 0) {
            if (stdin_check == 0) {
                trn_fp = stdin;
            } else {
                puts("異常終了 標準入力から複数指定することができません");
                exit(ARG_ERR);
            }
        }
        /* ADD 2014.12.15 Phuong.VM */
        else if ((trn_fp = fopen(argv[3 + opt_flg], "r")) == NULL) {
            fprintf(stderr, "%s\n", rtn_status[1].message);
            exit(rtn_status[1].code);
        }
    } else {
        /* printf("ここに入りました\n"); */
        /* ADD 2014.12.15 Phuong.VM */
        if (stdin_check == 0) {
            trn_fp = stdin;
        } else {
            puts("異常終了 標準入力から複数指定することができません");
            exit(ARG_ERR);
        }
        /* ADD 2014.12.15 Phuong.VM */
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
/*      関数名称：   pk_parm_get                                            */
/*      処理概要：   "pk="に設定しているパラメータを取得する                */
/*      入  力  ：   入力文字列                                             */
/*      出  力  ：   パラメータＴＢＬ                                       */
/*      戻り値  ：   NRML_END／ARG_ERR                                      */
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

    /* printf("argv = [%s]\n", argv); */
    strcpy(pk_buf, argv);
    /* printf("pk_buf = [%s]\n", pk_buf); */

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
/*      関数名称：    match                                                 */
/*      処理概要：    トランザクションとマスターのマッチング                */
/*      入  力  ：    キーＴＢＬ                                            */
/*      入  力  ：    マスターファイルポインタ                              */
/*      入  力  ：    トランザクションファイルポインタ                      */
/*      戻り値  ：    NRML_END／                                            */
/****************************************************************************/
int match(FILE * mas_fp, FILE * trn_fp)
{
    rec_split mst_split;        /* マスターレコード構造体            */
    rec_split trn_split;        /* トランザクションレコード構造体    */
    int break_cnt;              /* キーブレイク値                    */

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
            /*  出力（トランオンリー）                                      */
            if (display(NULL, &trn_split.all_rec[0])) {
                return 6;
            }

            /*  トランザクションリード処理                                  */
            if (t_read(trn_fp, &trn_split)) {
                return 3;
            }
        } else {
            /*  出力（マッチ）                                              */
            if (display(&mst_split.all_rec[0], &trn_split.all_rec[0])) {
                return 6;
            }
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
/*      処理概要：      フィールドスプリット（マッチングキー設定用）        */
/*      入  力  ：      入力レコード                                        */
/*      出  力  ：      フィールド格納構造体                                */
/*      入  力  ：      パラメータキー                                      */
/*      戻り値  ：      NRML_END／                                          */
/****************************************************************************/
int f_split(char *rec_buf, char *cat_char, int *mt_flg)
{
    int loop_cnt = 0;           /* ループカウンタ               */
#ifdef  NEW_TOKEN
    char *fld_point;            /* フィールドポインタ */
    int flen;                   /* フィールドレングス */
#else
    char *mst_fld_point;        /* マスタフィールドポインタ     */
    char *trn_fld_point;        /* トランフィールドポインタ     */
    char mst_rec_buf[MAX_RECLEN];       /* マスターレコードバッファ     */
    char trn_rec_buf[MAX_RECLEN];       /* トランレコードバッファ       */
#endif

    /*  初期処理                                                            */
    cat_char[0] = '\0';

    /*  トランザクション用                                                  */
    if (!*mt_flg) {
        /*  フィールド分割                                                  */
        trn_fld_cnt = 0;
#ifdef  NEW_TOKEN
        while (fld_point = new_strtok(trn_fld_cnt ? NULL : rec_buf, FLD_SEP_STR, &flen)) {
            memcpy(trn_buf[trn_fld_cnt], fld_point, flen);
            trn_buf[trn_fld_cnt][flen] = 0;
            trn_fld_cnt++;
        }
#else
        strcpy(trn_rec_buf, rec_buf);
        while (1) {
            if ((trn_fld_point = strtok(trn_fld_cnt ? NULL : trn_rec_buf,
                                        FLD_SEP_STR)) == NULL) {
                break;
            }
            strcpy(trn_buf[trn_fld_cnt], trn_fld_point);
            trn_fld_cnt++;
        }
#endif
/*      printf("trn_fld_cnt=%d\n",trn_fld_cnt);                    ** DEBUG */

        for (loop_cnt = 0; loop_cnt < pk_cnt; loop_cnt++) {
/*          printf("key_data[%d].order=%d\n", loop_cnt,            ** DEBUG */
/*                  key_data[loop_cnt].order);                     ** DEBUG */
            /* キー比較文字数指定判定                                       */
            if (key_data[loop_cnt].cutnum == 0) {
                strcat(cat_char, trn_buf[key_data[loop_cnt].order - 1]);
            } else {
                /* 比較文字数が指定されていれば、該当文字数分コピー         */
                strncat(cat_char, trn_buf[key_data[loop_cnt].order - 1],
                        key_data[loop_cnt].cutnum);
            }
/*          printf("trn_char=%s\n",cat_char);                 ** DEBUG */
        }
    }
    /*  マスター用                                                          */
    else {
        /*  フィールド分割                                                  */
        mst_fld_cnt = 0;
#ifdef  NEW_TOKEN
        while (fld_point = new_strtok(mst_fld_cnt ? NULL : rec_buf, FLD_SEP_STR, &flen)) {
            memcpy(mst_buf[mst_fld_cnt], fld_point, flen);
            mst_buf[mst_fld_cnt][flen] = 0;
            /* オプションの文字列出力指定がなく、１レコード目であれば
               マスタの各フィールドのレングスをバックアップする */
            if (opt_flg == 0 && mst_output_cnt == 0) {
                mst_otherlen_tbl[mst_fld_cnt] = flen;
            }
            mst_fld_cnt++;
        }
#else
        strcpy(mst_rec_buf, rec_buf);
        while (1) {
            if ((mst_fld_point = strtok(mst_fld_cnt ? NULL : mst_rec_buf,
                                        FLD_SEP_STR)) == NULL) {
                break;
            }
            strcpy(mst_buf[mst_fld_cnt], mst_fld_point);
            /* オプションの文字列出力指定がなく、１レコード目であれば
               マスタの各フィールドのレングスをバックアップする */
            if (opt_flg == 0 && mst_output_cnt == 0) {
                mst_otherlen_tbl[mst_fld_cnt] = strlen(mst_fld_point);
/*              printf("mst_otherlen_tbl[%d]=%d\n",           ** DEBUG */
/*                     mst_fld_cnt, strlen(mst_fld_point));   ** DEBUG */
            }
            mst_fld_cnt++;
        }
#endif
/*      printf("mst_fld_cnt=%d\n",mst_fld_cnt);                    ** DEBUG */

        for (loop_cnt = 0; loop_cnt < pk_cnt; loop_cnt++) {
/*          printf("key_tbl[1]=%d\n", key_tbl[0]);            ** DEBUG */
            /* キー比較文字数指定判定                                       */
            if (key_data[loop_cnt].cutnum == 0) {
                strcat(cat_char, mst_buf[key_data[loop_cnt].order - key_tbl[0]]);
            } else {
                /* 比較文字数が指定されていれば、該当文字数分コピー         */
                strncat(cat_char,
                        mst_buf[key_data[loop_cnt].order - key_tbl[0]],
                        key_data[loop_cnt].cutnum);
            }
/*          printf("mst_char=%s\n",cat_char);                 ** DEBUG */
        }
    }
    return NRML_END;
}

/****************************************************************************/
/*      関数名称：      display                                             */
/*      処理概要：      出力                                                */
/*      入  力  ：      入力レコード                                        */
/*      出  力  ：      フィールド格納構造体                                */
/*      入  力  ：      パラメータキー                                      */
/*      戻り値  ：      NRML_END／                                          */
/****************************************************************************/
int display(char *mst_all_rec, char *trn_all_rec)
{
    int w_pk_cnt;               /* ワークキーカウンタ           */
    int loop_cnt;               /* ループカウンタ               */
    int parm_key_cnt;           /* パラメータキーカウンタ       */
    int key_after_flg;          /* キー出力後フラグ[後：1]      */
    int mst_after_flg;          /* マスタのキー以外のデータ     */
    /* 出力後フラグ[後：1]          */
    int output_len;             /* アウトプットバッファレングス */
    char output_buf[MAX_RECLEN];        /* アウトプットバッファ         */

#ifdef  NEW_TOKEN
    int itmlen;
    char *itmptr;
    char *outptr;
#endif

    /*  初期設定                                                            */
    key_after_flg = 0;
    mst_after_flg = 0;
#ifdef  NEW_TOKEN
    outptr = output_buf;
#else
    output_buf[0] = '\0';
#endif

    /*  マスタの１レコード目のフィールド数を基準とする                      */
    if (mst_output_cnt == 0) {
        mst_output_cnt = mst_fld_cnt;
    }
/*  printf("INIT-mst_output_cnt=%d\n", mst_output_cnt);        ** DEBUG */

/*  printf("pk_cnt=%d\n", pk_cnt);                             ** DEBUG */
    /* キー項目数が多ければマスタの項目数に合わせる                         */
    if (pk_cnt > mst_fld_cnt) {
        w_pk_cnt = mst_fld_cnt;
    } else {
        w_pk_cnt = pk_cnt;
    }
/*  printf("w_pk_cnt=%d\n", w_pk_cnt);                         ** DEBUG */

    /*  出力（マスターレコード連結）                                        */
    /*////////////////////////////////////////////////////////////////////// */
    /*  マッチ時                                                            */
    /*////////////////////////////////////////////////////////////////////// */
    if (mst_all_rec != NULL) {
        /*  出力バッファに格納                                              */
        for (loop_cnt = 0, parm_key_cnt = 0;
             loop_cnt < trn_fld_cnt + (mst_fld_cnt - w_pk_cnt); loop_cnt++) {

            /*  キーをバッファに格納                                        */
            if (loop_cnt + 1 == key_tbl[parm_key_cnt]) {
#ifdef  NEW_TOKEN
                itmptr = trn_buf[loop_cnt];
#else
                strcat(output_buf, trn_buf[loop_cnt]);
#endif
                parm_key_cnt++;
                key_after_flg = 1;
            } else {
                /*  トランのキー以外のデータをバッファに格納                */
                if (key_after_flg == 0 && mst_after_flg == 0) {
#ifdef  NEW_TOKEN
                    itmptr = trn_buf[loop_cnt];
#else
                    strcat(output_buf, trn_buf[loop_cnt]);
#endif
                }

                /*  キー出力が終了のため、マスタのキー以外を出力            */
                else if (key_after_flg == 1) {
/*                  printf("mst_fld_cnt=%d\n", mst_fld_cnt);    ** DEBUG */
                    if (parm_key_cnt < mst_fld_cnt) {
                        /* アンマッチ時のマスタのキー以外のレングスをバックアップ */
/* DEL 2002.11.21 Kawamura --
                        if(mst_output_cnt == 0 && opt_flg == 0){
                            mst_otherlen_tbl[parm_key_cnt] =
                                    strlen(mst_buf[parm_key_cnt]);
                        }
-- DEL 2002.11.21 Kawamura */

#ifdef  NEW_TOKEN
                        itmptr = mst_buf[parm_key_cnt++];
#else
                        strcat(output_buf, mst_buf[parm_key_cnt++]);
#endif
                    } else {
                        /*  連結指定                                        */
                        if (w_pk_cnt > 1) {
#ifdef  NEW_TOKEN
                            itmptr = trn_buf[loop_cnt - (mst_fld_cnt - w_pk_cnt)];
#else
                            strcat(output_buf,
                                   trn_buf[loop_cnt - (mst_fld_cnt - w_pk_cnt)]);
#endif
                        }
                        /*  単数指定                                        */
                        else {
#ifdef  NEW_TOKEN
                            itmptr = trn_buf[loop_cnt - (parm_key_cnt - 1)];
#else
                            strcat(output_buf, trn_buf[loop_cnt - (parm_key_cnt - 1)]);
#endif
                        }
                        key_after_flg = 0;
                        mst_after_flg = 1;
                    }
                } else if (mst_after_flg == 1) {
                    /*  連結指定                                            */
                    if (w_pk_cnt > 1) {
#ifdef  NEW_TOKEN
                        itmptr = trn_buf[loop_cnt - (mst_fld_cnt - w_pk_cnt)];
#else
                        strcat(output_buf, trn_buf[loop_cnt - (mst_fld_cnt - w_pk_cnt)]);
#endif
                    }
                    /*  単数指定                                            */
                    else {
#ifdef  NEW_TOKEN
                        itmptr = trn_buf[loop_cnt - (parm_key_cnt - 1)];
#else
                        strcat(output_buf, trn_buf[loop_cnt - (parm_key_cnt - 1)]);
#endif
                    }
                }
            }
#ifdef  NEW_TOKEN
            /* フィールドセパレートを付加してアウトプットバッファに格納 */
            itmlen = strlen(itmptr);
            memcpy(outptr, itmptr, itmlen);
            outptr += itmlen;
            *outptr++ = ' ';
#else
            /*  フィールドセパレートをアウトプットバッファに格納            */
            strcat(output_buf, FS_SPACE);
#endif
        }
    }
    /*////////////////////////////////////////////////////////////////////// */
    /*  トランオンリー                                                      */
    /*////////////////////////////////////////////////////////////////////// */
    else {
        /*  出力バッファに格納                                              */
        /* 2002/01/28 T.Kinoshita                                           */
        /*  １レコード目からトランオンリーの場合は終了条件の値を設定する    */
/*      printf("TRN-mst_output_cnt=%d\n", mst_output_cnt);   ** DEBUG */
/* DEL 2002.11.21 Kawamura --
        if(mst_output_cnt == 0){
            if(mst_output_cnt < w_pk_cnt){
                mst_output_cnt = w_pk_cnt + 1;
            }
        }
-- DEL 2002.11.21 Kawamura */

        for (loop_cnt = 0, parm_key_cnt = 0;
             loop_cnt < trn_fld_cnt + ((mst_output_cnt ? mst_output_cnt : 1) - w_pk_cnt);
             loop_cnt++) {

            /*  キーをバッファに格納                                        */
            if (loop_cnt + 1 == key_tbl[parm_key_cnt]) {
#ifdef  NEW_TOKEN
                itmptr = trn_buf[loop_cnt];
#else
                strcat(output_buf, trn_buf[loop_cnt]);
#endif
                parm_key_cnt++;
                key_after_flg = 1;
            } else {
                /*  トランのキー以外のデータをバッファに格納                */
                if (key_after_flg == 0 && mst_after_flg == 0) {
#ifdef  NEW_TOKEN
                    itmptr = trn_buf[loop_cnt];
#else
                    strcat(output_buf, trn_buf[loop_cnt]);
#endif
                }

                /*  キー出力が終了のため、マスタのキー以外を出力            */
                else if (key_after_flg == 1) {
                    if (mst_output_cnt > 1) {
/*                      printf("parm_key_cnt=%d\n",parm_key_cnt); ** DEBUG */
                        if (parm_key_cnt < mst_output_cnt) {
                            /*  ダミー文字列セット                          */
                            if (opt_flg == 0) {
/*              printf("mst_otherlen_tbl[%d]=%d\n",parm_key_cnt,   ** DEBUG */
/*                      mst_otherlen_tbl[parm_key_cnt]);           ** DEBUG */
                                memset(dmy_opt_char, DMY_CHAR,
                                       mst_otherlen_tbl[parm_key_cnt]);
                                dmy_opt_char[mst_otherlen_tbl[parm_key_cnt]] = '\0';
                            }
/*                      printf("dmy_opt_char=[%s]\n",dmy_opt_char); ** DEBUG */
#ifdef  NEW_TOKEN
                            itmptr = dmy_opt_char;
#else
                            strcat(output_buf, dmy_opt_char);
#endif
                            parm_key_cnt++;
                        } else {
                            /*  連結指定                                    */
                            if (w_pk_cnt > 1) {
#ifdef  NEW_TOKEN
                                itmptr = trn_buf[loop_cnt - (mst_output_cnt - w_pk_cnt)];
#else
                                strcat(output_buf,
                                       trn_buf[loop_cnt - (mst_output_cnt - w_pk_cnt)]);
#endif
                            }
                            /*  単数指定                                    */
                            else {
#ifdef  NEW_TOKEN
                                itmptr = trn_buf[loop_cnt - (parm_key_cnt - 1)];
#else
                                strcat(output_buf,
                                       trn_buf[loop_cnt - (parm_key_cnt - 1)]);
#endif
                            }
                            key_after_flg = 0;
                            mst_after_flg = 1;
                        }
                    } else {
#ifdef  NEW_TOKEN
                        itmptr = dmy_opt_char;
#else
                        strcat(output_buf, dmy_opt_char);
#endif
                        key_after_flg = 0;
                        mst_after_flg = 1;
                    }
                } else if (mst_after_flg == 1) {
                    if (mst_output_cnt >= 1) {
                        /*  連結指定                                        */
                        if (w_pk_cnt > 1) {
#ifdef  NEW_TOKEN
                            itmptr = trn_buf[loop_cnt - (mst_output_cnt - w_pk_cnt)];
#else
                            strcat(output_buf,
                                   trn_buf[loop_cnt - (mst_output_cnt - w_pk_cnt)]);
#endif
                        }
                        /*  単数指定                                        */
                        else {
#ifdef  NEW_TOKEN
                            itmptr = trn_buf[loop_cnt - (parm_key_cnt - 1)];
#else
                            strcat(output_buf, trn_buf[loop_cnt - (parm_key_cnt - 1)]);
#endif
                        }
                    } else {
#ifdef  NEW_TOKEN
                        itmptr = trn_buf[loop_cnt - 1];
#else
                        strcat(output_buf, trn_buf[loop_cnt - 1]);
#endif
                    }
                }
            }
#ifdef  NEW_TOKEN
            /* フィールドセパレートを付加してアウトプットバッファに格納 */
            itmlen = strlen(itmptr);
            memcpy(outptr, itmptr, itmlen);
            outptr += itmlen;
            *outptr++ = ' ';
#else
            /*  フィールドセパレートをアウトプットバッファに格納            */
            strcat(output_buf, FS_SPACE);
#endif
        }
    }
    /*  アンマッチ時のマスタのキー以外のフィールド数をバックアップ          */
/* DEL 2002.11.21 Kawamura --
    if(mst_output_cnt == 0 && mst_all_rec != NULL){
        mst_output_cnt = mst_fld_cnt;
    }
    else if(mst_output_cnt == 0 && mst_all_rec == NULL){
        mst_output_cnt = 1;
    }
-- DEL 2002.11.21 Kawamura */

    /* 最終文字のスペースを改行に変更                                       */
#ifdef  NEW_TOKEN
    output_len = outptr - output_buf;
    output_buf[output_len - 1] = '\n';
    fwrite(output_buf, output_len, 1, stdout);
#else
    output_len = strlen(output_buf) - 1;
    output_buf[output_len] = '\0';
    fprintf(stdout, "%s\n", output_buf);
#endif

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
    puts("使用方法: v_s3match [+charctor] pk=$$ masterfile < tranfile > outputfile");
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
