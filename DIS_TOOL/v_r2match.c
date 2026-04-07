/****************************************************************************/
/*  システム名称：  ＤＩＳユーティリティー                                  */
/*  コマンド名称：  レコードセレクトマッチングコマンド（v_r2match）       */
/*  機能        ：                                                          */
/*    ・トラン上のキーがマスタ上に存在する行を抽出し、標準出力へ出力する。  */
/*      この時、キーと次の項目の間にマスタの該当行の第２項目以降を、半角    */
/*      スペースで区切り挿入する。                                          */
/*    ・アンマッチ出力オプションが指定された場合は、存在しない行を          */
/*      標準エラー出力へ出力する。                                          */
/*  制約事項    ：                                                          */
/*    ・マスタ、トラン共にフィールドセパレータは半角スペースとする。        */
/*      （二つ以上の半角スペースで区切られている場合も出力は１つとなる）    */
/*    ・マスタのキーは先頭項目固定とする。                                  */
/*    ・マスタはユニークキーで昇順ソート済とする。                          */
/*    ・マスタ、トラン共にレコード（行）レングスはＬＦを含めて最大６１４４  */
/*      バイトとする。                                                      */
/*  使用法      ：  v_r2match [+stderr] pk=$$ master [trans]              */
/*                    +stderr  : アンマッチ出力オプション                   */
/*                    pk=$$    : トランのキーの項目Ｎｏ（１～）             */
/*                              ex) pk=2                                    */
/*                    master   : マスタのファイルパス                       */
/*                    trans    : トランのファイルパス（省略時は標準入力）   */
/*  ステータス  ：  0 : 正常終了                                            */
/*                  1 : 警告終了（出力失敗データ有り）                      */
/*                  2 : アーギュメント不正                                  */
/*                  3 : ファイルオープンエラー                              */
/*                  4 : ファイルリードエラー                                */
/*                  5 : メモリ確保エラー                                    */
/*  作成者      ：  T.Akiyama (DIS)                                         */
/*  作成日      ：  2001/12/3                                               */
/*  修正履歴    ：                                                          */
/*  No. 修正日付    修正者            修正内容                              */
/*　001 2002/11/08　川村 　 最大レコードサイズ変更(1024Byte=>3072byte)      */
/*　002 2003/02/05　寺澤　　　 最大レコードサイズ変更(3072Byte=>6144byte)   */
/****************************************************************************/

/*--------------------------------------------------------------------------*/
/*  ヘッダーファイルインクルード定義                                        */
/*--------------------------------------------------------------------------*/
/*  System Headder File Include                                             */
#include        <stdio.h>
#include        <string.h>
#include        <strings.h>
#include        <stdlib.h>
#include        <errno.h>

/*  System Headder File Include                                             */
#include        "libdistool.inc"

/*--------------------------------------------------------------------------*/
/*  内部関数定義                                                            */
/*--------------------------------------------------------------------------*/
static int select_matching_tran_record(int keynum, char *keytbl,
                                       char *oitmtbl, int oitmlen,
                                       FILE * fp, int kitmno, int ngflg);
#ifdef  NEW_TOKEN
char *new_strtok(char *src, char *dc, int *len);
#endif

/*--------------------------------------------------------------------------*/
/*  グローバル変数定義                                                      */
/*--------------------------------------------------------------------------*/
/*  外部グローバル変数定義                                                  */
int G_mstkeylen = -1;           /* マスタキーレングス         */

/****************************************************************************/
/*  関数名称：  main（レコードセレクトマッチングコマンドメイン処理）        */
/*  処理概要：  キー項目がマスタ上に存在するトラン上のレコードを抽出して、  */
/*              標準出力へ出力する。                                        */
/*              この時、トランのキー項目と次の項目の間に、マスタの該当      */
/*              レコードの第２項目以降を半角スペースをセパレータとして      */
/*              挿入する。                                                  */
/*              キー項目がマスタ上に存在しないレコードはアンマッチ出力      */
/*              オプションが指定されている場合のみ、標準エラー出力へ        */
/*              出力する。                                                  */
/****************************************************************************/
main(int argc, char **argv)
{

    int sts;                    /* 終了ステータス              */
    int ret;                    /* リターン値                  */
    int ngflg;                  /* アンマッチ出力フラグ        */
    int key;                    /* トランキー項目Ｎｏ          */
    char *master;               /* マスタファイルパス          */
    char *trans;                /* トランファイルパス          */
    FILE *fp_mst;               /* マスタファイルポインタ      */
    FILE *fp_trn;               /* トランファイルポインタ      */
    int recnum_mst;             /* マスタレコード件数          */
    char *keytbl;               /* マスタキーＴＢＬ            */
    char *oitmtbl;              /* その他項目ＴＢＬ            */
    int oitmlen;                /* その他項目レングス          */

/*--------------------------------------------------------------------------*/

    /*  起動アーギュメントをチェックし、各パラメタを取得                    */
    if (sts = cm_check_rjoin_arg(argc, argv, ARGMODE_NGFLG,
                                 &ngflg, &key, &master, &trans)) {
        fprintf(stderr, "Usage: v_r2match [+stderr] pk=$$ master [trans]\n");
        goto END;
    }

    /*  指定されたマスタをオープン                                          */
    /* ADD 2014.12.16 Chin.TV */
    if (master == NULL)
        fp_mst = stdin;
    else {
        /* ADD 2014.12.16 Chin.TV */
        if ((fp_mst = fopen(master, "r")) == NULL) {
            if (errno == ENOENT) {
                fprintf(stderr, "指定されたマスタは存在しません。\n");
                sts = ARG_ERR;
            } else {
                fprintf(stderr, "fopen(\"%s\", \"r\") Error!! errno=%d:%s\n",
                        master, errno, cm_get_errmsg(errno));
                sts = FOPEN_ERR;
            }
            goto END;
        }
    }


    /*  トランが指定されている場合はオープン                                */
    if (trans == NULL)
        fp_trn = stdin;
    else {
        if ((fp_trn = fopen(trans, "r")) == NULL) {
            if (errno == ENOENT) {
                fprintf(stderr, "指定されたトランは存在しません。\n");
                sts = ARG_ERR;
            } else {
                fprintf(stderr, "fopen(\"%s\", \"r\") Error!! errno=%d:%s\n",
                        trans, errno, cm_get_errmsg(errno));
                sts = FOPEN_ERR;
            }
            goto END;
        }
    }

    /*  マスタの全レコードをインコアし、キーとその他の項目へ分割            */
    if (sts = cm_load_master_file(fp_mst, &recnum_mst, &keytbl, &G_mstkeylen,
                                  &oitmtbl, &oitmlen))
        goto END;

    /*  マスタをクローズ                                                    */
    /* ADD 2014.12.16 Chin.TV */
    if (master != NULL)
        (void) fclose(fp_mst);
    /* ADD 2014.12.16 Chin.TV */

    /*  トランのレコードをセレクトマッチングして出力                        */
    sts = select_matching_tran_record(recnum_mst, keytbl, oitmtbl, oitmlen,
                                      fp_trn, key, ngflg);

    /*  各ＴＢＬのエリアを解放                                              */
    if (keytbl != NULL)
        free(keytbl);           /* マスタキー        */
    if (oitmtbl != NULL)
        free(oitmtbl);          /* その他項目        */

    /*  トランが指定されている場合はクローズ                                */
    if (trans != NULL)
        (void) fclose(fp_trn);

  END:
    exit(sts);
}

/****************************************************************************/
/*  関数名称：  select_matching_tran_record                                 */
/*            （トランレコードセレクトマッチング）                          */
/*  処理概要：  トラン上の指定されたキーがキーＴＢＬ上に存在するレコードを  */
/*              抽出して標準出力へ出力する。                                */
/*              この時、トランのキー項目と次の項目の間に、マスタの該当      */
/*              レコードの第２項目以降を半角スペースをセパレータとして      */
/*              挿入する。                                                  */
/*              存在しないレコードはアンマッチ出力フラグがＯＮの場合のみ    */
/*              標準エラー出力へ出力する。                                  */
/*              注）出力順はトランのレコード順とする。                      */
/*  入  力  ：  keynum   ：  キー件数                                       */
/*  入  力  ：  keytbl   ：  キーＴＢＬ                                     */
/*  入  力  ：  oitmtbl  ：  その他項目ＴＢＬ                               */
/*  入  力  ：  oitmlen  ：  その他項目レングス                             */
/*  入  力  ：  fp       ：  ファイルポインタ                               */
/*  入  力  ：  kitmno   ：  キー項目Ｎｏ                                   */
/*  入  力  ：  ngflg    ：   アンマッチ出力フラグ                          */
/*  戻り値  ：  NRML_END／WARNING／FREAD_ERR                                */
/****************************************************************************/
int
select_matching_tran_record(int keynum, char *keytbl, char *oitmtbl,
                            int oitmlen, FILE * fp, int kitmno, int ngflg)
{
    static char buf[MAX_RECLEN];        /* リードバッファ          */
    int match;                  /* マッチフラグ            */
#ifndef  NEW_TOKEN
    static char sepbuf[MAX_RECLEN];     /* 項目分割用バッファ      */
#endif
    static char outbuf[MAX_RECLEN * 2]; /* 結合用ライトバッファ    */
    char *obptr;                /* ライトバッファポインタ  */
    register i;                 /* ループカウンター        */
    char *itmptr;               /* キー項目ポインタ        */
    int itmlen;                 /* 項目レングス            */
    static char key[MAX_RECLEN];        /* キー項目                */
    char *hitptr;               /* 該当キーＴＢＬポインタ  */
    int sts = NRML_END;         /* 終了ステータス          */

/*--------------------------------------------------------------------------*/

    /*  ＥＯＦ迄繰返す                                                      */
    while (1) {

        /*  指定トランから１レコード（行）をシーケンシャルリード            */
        if (fgets(buf, MAX_RECLEN, fp) == NULL) {
            if (ferror(fp)) {   /* Read Error    */
                fprintf(stderr, "fgets(%d) Tran Read Error!! errno=%d:%s\n",
                        MAX_RECLEN, errno, cm_get_errmsg(errno));
                return (FREAD_ERR);
            }
            break;              /* EOF           */
        }

        match = 0;              /* マッチフラグOFFで初期化          */
#ifdef  NEW_TOKEN
        obptr = outbuf;         /* 結合用ライトバッファポインタを設定 */
        for (i = 0; i < kitmno; i++) {  /* 項目分割し､キー項目ポインタ取得  */
            if ((itmptr = new_strtok(i ? NULL : buf, FLD_SEP_STR, &itmlen)) == NULL)
                break;          /* 項目が足りない場合      */
            /* 先頭項目で無ければフィールドセパレータをライトバッファへ     */
            if (i) {
                *obptr = FLD_SEP;
                obptr++;
            }
            memcpy(obptr, itmptr, itmlen);      /* マスタ項目をライトバッファへ */
            obptr += itmlen;
        }
#else
        strcpy(sepbuf, buf);    /* 項目分割用バッファへリードレコードコピー */
        obptr = outbuf;         /* 結合用ライトバッファポインタを設定 */
        for (i = 0; i < kitmno; i++) {  /* 項目分割し､キー項目ポインタ取得  */
            if ((itmptr = strtok(i ? NULL : sepbuf, FLD_SEP_STR)) == NULL)
                break;          /* 項目が足りない場合      */
            itmlen = strlen(itmptr);    /* 項目レングス取得        */
            /* 先頭項目で無ければフィールドセパレータをライトバッファへ     */
            if (i) {
                *obptr = FLD_SEP;
                obptr++;
            }
            memcpy(obptr, itmptr, itmlen);      /* マスタ項目をライトバッファへ */
            obptr += itmlen;
        }
#endif
        if (i >= kitmno && keynum) {    /* マスタキー及びキー項目が存在する場合 */
            /* レングスがマスタキーレングス以下の場合                       */
            if (itmlen <= G_mstkeylen) {
                memcpy(key, itmptr, itmlen);    /* 比較用キー項目へコピー   */
                if (itmlen < G_mstkeylen)       /* 右空きスペース詰め       */
                    memset(&key[itmlen], ' ', G_mstkeylen - itmlen);
                /* キーTBLをバイナリサーチ  */
                if ((hitptr = bsearch(key, keytbl, keynum, G_mstkeylen,
                                      cm_compare_master_key)) != NULL)
                    match = 1;
            }
        }

        /*  アンマッチ且つアンマッチ出力無しは破棄                          */
        if (!match && !ngflg)
            continue;

        if (match) {            /* マッチした場合        */

            /*--------------------------------------------------------------*/
            /*  マッチしたマスタのキー項目以降の項目をライトバッファへ設定  */
            /*--------------------------------------------------------------*/
            itmptr = oitmtbl + (hitptr - keytbl) / G_mstkeylen * oitmlen;
            itmlen = strlen(itmptr);
            if (itmlen) {
                *obptr = FLD_SEP;
                obptr++;
                memcpy(obptr, itmptr, itmlen);
                obptr += itmlen;
            }

            /*--------------------------------------------------------------*/
            /*  トランのキー項目以降の項目を分割してライトバッファへ設定    */
            /*--------------------------------------------------------------*/
            while (1) {
#ifdef  NEW_TOKEN
                if ((itmptr = new_strtok(NULL, FLD_SEP_STR, &itmlen)) == NULL)
                    break;
#else
                if ((itmptr = strtok(NULL, FLD_SEP_STR)) == NULL)
                    break;
                itmlen = strlen(itmptr);
#endif
                *obptr = FLD_SEP;
                obptr++;
                memcpy(obptr, itmptr, itmlen);
                obptr += itmlen;
            }
#ifdef  NEW_TOKEN
            /*  マスタとトランを結合したレコードを標準出力へ出力            */
            *obptr++ = '\n';
            if (fwrite(outbuf, obptr - outbuf, 1, stdout) < 1) {
                if (sts == NRML_END) {
                    fprintf(stderr, "fwrite() Write Error!! errno=%d:%s\n",
                            errno, cm_get_errmsg(errno));
                    sts = WARNING;
                }
            }
#else
            *obptr = '\0';      /* NULLターミネート    */

            /*  マスタとトランを結合したレコードを標準出力へ出力            */
            if (fprintf(stdout, "%s\n", outbuf) < 0) {
                if (sts == NRML_END) {
                    fprintf(stderr, "fprintf() Write Error!! errno=%d:%s\n",
                            errno, cm_get_errmsg(errno));
                    sts = WARNING;
                }
            }
#endif
        } else {                /* アンマッチの場合    */

            /*  リードトランレコードをそのまま標準エラーへ出力              */
#ifdef  NEW_TOKEN
            if (fwrite(buf, strlen(buf), 1, stderr) < 1) {
                if (sts == NRML_END) {
                    fprintf(stderr, "fwrite() Write Error!! errno=%d:%s\n",
                            errno, cm_get_errmsg(errno));
                    sts = WARNING;
                }
            }
#else
            if (fprintf(stderr, "%s", buf) < 0) {
                if (sts == NRML_END) {
                    fprintf(stderr, "fprintf() Write Error!! errno=%d:%s\n",
                            errno, cm_get_errmsg(errno));
                    sts = WARNING;
                }
            }
#endif
        }
    }

    return (sts);
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

/*** End Of File ***/
