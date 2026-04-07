/****************************************************************************/
/*  システム名称：  ＤＩＳユーティリティー                                  */
/*  コマンド名称：  レコードマッチングコマンド（v_r3match）               */
/*  機能        ：                                                          */
/*    ・マスタ上の同一キーを持つ行を結合し、標準出力へ出力する。            */
/*      この時、トランのキーと次の項目の間にマスタの該当行の第２項目以降を、*/
/*      半角スペースで区切り挿入する。                                      */
/*    ・マスタ上に同一キーを持つ行が存在しない場合は、マスタの項目が        */
/*      挿入されるフィールドにアンマッチ出力文字列が指定された場合は、      */
/*      該当文字列を、未指定の場合は '*' を出力する。                       */
/*      この場合の挿入するマスタの項目数は、直前に出力した際のマスタ挿入    */
/*      項目数とする。（アンマッチ出力文字列未指定の場合は桁数も合せる）    */
/*      先頭行の出力の場合はマスタの先頭行の項目数、桁数に合せる。          */
/*  制約事項    ：                                                          */
/*    ・マスタ、トラン共にフィールドセパレータは半角スペースとする。        */
/*      （二つ以上の半角スペースで区切られている場合も出力は１つとなる）    */
/*    ・マスタのキーは先頭項目固定とする。                                  */
/*    ・マスタはユニークキーで昇順ソート済とする。                          */
/*    ・マスタ、トラン共にレコード（行）レングスはＬＦを含めて最大６１４４  */
/*      バイトとする。                                                      */
/*  使用法      ：  v_r3match [+$$] pk=$$ master [trans]                  */
/*                    +$$    : アンマッチ出力文字列                         */
/*                               ex) +000                                   */
/*                    pk=$$  : トランのキーの項目Ｎｏ（１～）               */
/*                               ex) pk=2                                   */
/*                    master : マスタのファイルパス                         */
/*                    trans  : トランのファイルパス（省略時は標準入力）     */
/*  ステータス  ：  0 : 正常終了                                            */
/*                  1 : 警告終了（出力失敗データ有り）                      */
/*                  2 : アーギュメント不正                                  */
/*                  3 : ファイルオープンエラー                              */
/*                  4 : ファイルリードエラー                                */
/*                  5 : メモリ確保エラー                                    */
/*  作成者      ：  T.Akiyama (DIS)                                         */
/*  作成日      ：  2001/12/5                                               */
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
static int matching_tran_record(int keynum, char *keytbl,
                                char *oitmtbl, int oitmlen,
                                FILE * fp, int kitmno, char *umostr);
#ifdef  NEW_TOKEN
char *new_strtok(char *src, char *dc, int *len);
#endif

/*--------------------------------------------------------------------------*/
/*  グローバル変数定義                                                      */
/*--------------------------------------------------------------------------*/
/*  外部グローバル変数定義                                                  */
int G_mstkeylen = -1;           /* マスタキーレングス            */

/****************************************************************************/
/*  関数名称：  main（レコードマッチングコマンドメイン処理）                */
/*  処理概要：  マスタ上の同一キーを持つ行を結合し、標準出力へ出力する。    */
/*              この時、トランのキーと次の項目の間にマスタの該当行の        */
/*              第２項目以降を、半角スペースで区切り挿入する。              */
/*              マスタ上に同一キーを持つ行が存在しない場合は、マスタの      */
/*              項目が挿入されるフィールドにアンマッチ出力文字列が          */
/*              指定された場合は、該当文字列を、未指定の場合は '*' を       */
/*              出力する。                                                  */
/*              この場合の挿入するマスタの項目数は、直前に出力した際の      */
/*              マスタ挿入項目数とする。（アンマッチ出力文字列未指定の      */
/*              場合は桁数も合せる）                                        */
/*              先頭行の出力の場合はマスタの先頭行の項目数、桁数に合せる。  */
/****************************************************************************/
main(int argc, char **argv)
{

    int sts;                    /* 終了ステータス              */
    int ret;                    /* リターン値                  */
    char *umostr;               /* アンマッチ出力文字列        */
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
    if (sts = cm_check_rjoin_arg(argc, argv, ARGMODE_UMOSTR,
                                 &umostr, &key, &master, &trans)) {
        fprintf(stderr, "Usage: v_r3match [+$$] pk=$$ master [trans]\n");
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

    /*  トランのレコードをマスタとマッチングして出力                        */
    sts = matching_tran_record(recnum_mst, keytbl, oitmtbl, oitmlen, fp_trn, key, umostr);

    /*  各ＴＢＬのエリアを解放                                              */
    if (keytbl != NULL)
        free(keytbl);           /* マスタキー   */
    if (oitmtbl != NULL)
        free(oitmtbl);          /* その他項目   */

    /*  トランが指定されている場合はクローズ                                */
    if (trans != NULL)
        (void) fclose(fp_trn);

  END:
    exit(sts);
}

/****************************************************************************/
/*  関数名称：  matching_tran_record（トランレコードマッチング）            */
/*  処理概要：  トラン上の指定されたキーがキーＴＢＬ上に存在するレコードと  */
/*              該当キーに対応するマスタのレコードを結合して標準出力へ出力  */
/*              する。                                                      */
/*              この時、トランのキー項目と次の項目の間に、該当キーに        */
/*              対応するその他項目ＴＢＬ上の項目を半角スペースを            */
/*              セパレータとして挿入する。                                  */
/*              キーＴＢＬに存在しない場合は、挿入フィールドにアンマッチ    */
/*              出力文字列が指定された場合は、該当文字列を、未指定の場合は  */
/*              '*' を出力する。                                            */
/*              この場合の挿入項目数は、直前に出力した際の挿入項目数とする。*/
/*              （アンマッチ出力文字列未指定の場合は桁数も合せる）          */
/*              先頭行の出力の場合はその他項目ＴＢＬの先頭行の項目数、      */
/*              桁数に合せる。                                              */
/*              注）出力順はトランのレコード順とする。                      */
/*  入  力  ：  keynum    ：  キー件数                                      */
/*  入  力  ：  keytbl    ：  キーＴＢＬ                                    */
/*  入  力  ：  oitmtbl   ：  その他項目ＴＢＬ                              */
/*  入  力  ：  oitmlen   ：  その他項目レングス                            */
/*  入  力  ：  fp        ：  ファイルポインタ                              */
/*  入  力  ：  kitmno    ：  キー項目Ｎｏ                                  */
/*  入  力  ：  umostr    ：  アンマッチ出力文字列                          */
/*  戻り値  ：  NRML_END／WARNING／FREAD_ERR                                */
/****************************************************************************/
int
matching_tran_record(int keynum, char *keytbl, char *oitmtbl,
                     int oitmlen, FILE * fp, int kitmno, char *umostr)
{

    char *hitptr_b;             /* 前回該当キーTBLポインタ      */
    static char buf[MAX_RECLEN];        /* リードバッファ               */
    int match;                  /* マッチフラグ                 */
    static char outbuf[MAX_RECLEN * 2]; /* 結合用ライトバッファ         */
    char *obptr;                /* ライトバッファポインタ       */
    register i;                 /* ループカウンター             */
    char *itmptr;               /* キー項目ポインタ             */
    int itmlen;                 /* 項目レングス                 */
    static char wk[MAX_RECLEN]; /* ワークバッファ               */
    char *fsptr;                /* フィールドセパレータポインタ */
    char *hitptr;               /* 該当キーＴＢＬポインタ       */
    int sts = NRML_END;         /* 終了ステータス               */

/*--------------------------------------------------------------------------*/

    /*  ＥＯＦ迄繰返す                                                      */
    hitptr_b = keytbl;
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

        match = 0;              /* マッチフラグOFFで初期化            */
        obptr = outbuf;         /* 結合用ライトバッファポインタを設定 */
        for (i = 0; i < kitmno; i++) {  /* 項目分割し､キー項目ポインタ取得    */
#ifdef	NEW_TOKEN
            if ((itmptr = new_strtok(i ? NULL : buf, FLD_SEP_STR, &itmlen)) == NULL)
                break;          /* 項目が足りない場合  */
#else
            if ((itmptr = strtok(i ? NULL : buf, FLD_SEP_STR)) == NULL)
                break;          /* 項目が足りない場合  */
            itmlen = strlen(itmptr);    /* 項目レングス取得    */
#endif
            /* 先頭項目で無ければフィールドセパレータをライトバッファへ     */
            if (i) {
                *obptr = FLD_SEP;
                obptr++;
            }
            memcpy(obptr, itmptr, itmlen);      /* マスタ項目をライトバッファへ */
            obptr += itmlen;
        }
        if (i >= kitmno && keynum) {    /* キー項目が存在する場合         */
            /* レングスがマスタキーレングス以下の場合                       */
            if (itmlen <= G_mstkeylen) {
                memcpy(wk, itmptr, itmlen);     /* 比較用キー項目へコピー   */
                if (itmlen < G_mstkeylen)       /* 右空きスペース詰め       */
                    memset(&wk[itmlen], ' ', G_mstkeylen - itmlen);
                /* キーTBLをバイナリサーチ  */
                if ((hitptr = bsearch(wk, keytbl, keynum, G_mstkeylen,
                                      cm_compare_master_key)) != NULL) {
                    match = 1;
                    hitptr_b = hitptr;
                }
            }
        }

        if (match) {            /* マッチした場合             */

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
        }
        /*  アンマッチ且つマスタデータ有りの場合    */
        else if (hitptr_b != NULL && G_mstkeylen) {

            /*--------------------------------------------------------------*/
            /*  アンマッチ出力文字列及び直前挿入（先頭の場合はマスタ先頭行） */
            /*  したマスタの内容に応じてマスタのキー項目以降の項目に対し、  */
            /*  アンマッチ文字列をライトバッファへ設定                      */
            /*--------------------------------------------------------------*/
            itmptr = oitmtbl + (hitptr_b - keytbl) / G_mstkeylen * oitmlen;
            memcpy(wk, itmptr, oitmlen);
            itmptr = wk;
            if (umostr != NULL)
                itmlen = strlen(umostr);
            while ((fsptr = strchr(itmptr, FLD_SEP)) != NULL) {
                if (obptr != outbuf) {
                    *obptr = FLD_SEP;
                    obptr++;
                }
                if (umostr == NULL) {
                    /* *fsptr = NULL ; */
                    *fsptr = '\0';
                    itmlen = strlen(itmptr);
                    memset(obptr, DEF_OUTCHR_UNMATCH, itmlen);
                    itmptr += (itmlen + 1);
                } else {
                    memcpy(obptr, umostr, itmlen);
                    itmptr = fsptr + 1;
                }
                obptr += itmlen;
            }
            if (*itmptr != '\0') {
                if (obptr != outbuf) {
                    *obptr = FLD_SEP;
                    obptr++;
                }
                if (umostr == NULL) {
                    itmlen = strlen(itmptr);
                    memset(obptr, DEF_OUTCHR_UNMATCH, itmlen);
                } else
                    memcpy(obptr, umostr, itmlen);
                obptr += itmlen;
            }
        }

        /*------------------------------------------------------------------*/
        /*  トランのキー項目以降の項目を分割してライトバッファへ設定        */
        /*------------------------------------------------------------------*/
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
        *obptr++ = '\n';

        /*  マスタとトランを結合したレコードを標準出力へ出力                */
        if (fwrite(outbuf, obptr - outbuf, 1, stdout) < 1) {
            if (sts == NRML_END) {
                fprintf(stderr, "fwrite() Write Error!! errno=%d:%s\n",
                        errno, cm_get_errmsg(errno));
                sts = WARNING;
            }
        }
#else
        *obptr = '\0';          /* NULLターミネート             */

        /*  マスタとトランを結合したレコードを標準出力へ出力                */
        if (fprintf(stdout, "%s\n", outbuf) < 0) {
            if (sts == NRML_END) {
                fprintf(stderr, "fprintf() Write Error!! errno=%d:%s\n",
                        errno, cm_get_errmsg(errno));
                sts = WARNING;
            }
        }
#endif
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
