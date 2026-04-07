/****************************************************************************/
/*  システム名称    ：  ＤＩＳユーティリティー                              */
/*  ファイル名称    ：  共通ライブラリインクルードファイル（libdistool.inc）*/
/*  作成者          ：  T.Akiyama (DIS)                                     */
/*  作成日          ：  2001/12/03                                          */
/*  修正履歴        ：                                                      */
/*  No.   修正日付    修正者          修正内容                              */
/*  001   2002/10/28  H.Kawamura    最大レコード長変更 1024 → 3072         */
/*  002   2003/02/05  N.Terasawa    最大レコード長変更 3072 → 6144         */
/****************************************************************************/

#ifndef    _LIBDISTOOL_INC
#define    _LIBDISTOOL_INC

/*--------------------------------------------------------------------------*/
/*  ヘッダーファイルインクルード定義                                        */
/*--------------------------------------------------------------------------*/
#include    <stdio.h>

/*--------------------------------------------------------------------------*/
/*  ｄｅｆｉｎｅ定義                                                        */
/*--------------------------------------------------------------------------*/
                                        /* 終了ステータス一覧               */
#define     NRML_END              0               /* 正常終了               */
#define     WARNING               1               /* 警告終了               */
#define     ARG_ERR               2               /* アーギュメント不正     */
#define     FOPEN_ERR             3               /* ファイルオープンエラー */
#define     FREAD_ERR             4               /* ファイルリードエラー   */
#define     MALLOC_ERR            5               /* メモリ確保エラー       */
                                        /* 引数の数一覧                     */
#define     MIN_ARGS              3               /* 最小                   */
#define     MAX_ARGS              5               /* 最大                   */
                                        /* 起動アーギュメント区分           */
#define     ARGMODE_NGFLG         1        /* アンマッチ出力オプション付    */
#define     ARGMODE_UMOSTR        2     /* アンマッチ出力文字列オプション付 */
                                        /* オプション一覧                   */
#define     OPT_UNMATCH_OUT       "+stderr"       /* アンマッチ出力         */
#define     OPT_NOPRINT           "+horiz"        /* 水平出力               */
#define     OPT_UNMATCH_OUTSTR    "+"             /* アンマッチ出力文字列   */
#define     OPT_TRN_KEY           "pk="           /* トランキー項目Ｎｏ     */
                                        /* レングス一覧                     */
#define     MAX_RECLEN            6144            /* 最大レコード長＋１     */
                                        /* フィールドセパレータ一覧         */
#define     FLD_SEP               ' '             /* 文字                   */
#define     FLD_SEP_STR           " \n"           /* 文字列（ＬＦ含む）     */
                                        /* メモリアロケーション単位一覧     */
#define     MAUNIT_KTBL           10000           /* キーＴＢＬ             */
#define     MAUNIT_ITBL           1000            /* その他項目ＴＢＬ       */
                                        /* デフォルト値一覧                 */
#define     DEF_OUTCHR_UNMATCH    '*'             /* アンマッチ出力文字     */

/*--------------------------------------------------------------------------*/
/*  関数プロトタイプ宣言                                                    */
/*--------------------------------------------------------------------------*/
char        *cm_get_errmsg(int errnum) ;
int         cm_check_rjoin_arg(int argc,    char **argv,
                               int argmode, void *opt,
                               int *key,  char **master, char **trans) ;
int         cm_load_master_file(FILE *fp, int  *recnum,       char **ktblptr,
                               int *keylen, char **oitblptr, int *oitemlen) ;
int         cm_compare_master_key(const void *key1, const void *key2) ;

#endif
/*** End Of File ***/

