/****************************************************************************/
/*	システム名称	：	ＤＩＳユーティリティー								*/
/*	ライブラリ名称	：	共通ライブラリ（libdistool.a）						*/
/*	関数名称		：	 1.レコードジョインコマンド							*/
/*						　 起動アーギュメントチェック（cm_check_rjoin_arg）	*/
/*	作成者			：	T.Akiyama (DIS)										*/
/*	作成日			：	2001/12/05											*/
/*	修正履歴		：														*/
/*	No.	修正日付	修正者			修正内容								*/
/*	001	    /  /  															*/
/****************************************************************************/

/*--------------------------------------------------------------------------*/
/*	ヘッダーファイルインクルード定義										*/
/*--------------------------------------------------------------------------*/
/*	System Headder File Include												*/
#include		<stdio.h>
#include		<strings.h>
#include		<string.h>
#include		<stdlib.h>

/*	User Headder File Include												*/
#include		"libdistool.inc"

/****************************************************************************/
/*	関数名称：	cm_check_rjoin_arg											*/
/*				（レコードジョインコマンド起動アーギュメントチェック）		*/
/*	処理概要：	指定されたレコードジョインコマンドの起動アーギュメントを	*/
/*				起動アーギュメント区分に応じてチェックする。				*/
/*	入	力	：	argc	：	起動アーギュメントの数							*/
/*	入	力	：	argv	：	起動アーギュメント								*/
/*	入	力	：	argmode	：	起動アーギュメント区分							*/
/*							（ARGMODE_NGFLG	:ｱﾝﾏｯﾁ出力ﾌﾗｸﾞ／				*/
/*							  ARGMODE_UMOSTR:ｱﾝﾏｯﾁ出力文字列）				*/
/*	出	力	：	opt		：	アンマッチ出力フラグ（0:未出力／1:出力）／		*/
/*							アンマッチ出力文字列（未指定の場合はNULLﾎﾟｲﾝﾀ）	*/
/*	出	力	：	key		：	トランキー項目Ｎｏ								*/
/*	出	力	：	master	：	マスタファイルパスポインタ						*/
/*	出	力	：	trans	：	トランファイルパスポインタ						*/
/*							（標準入力時はＮＵＬＬポインタ）				*/
/*	戻り値	：	NRML_END／ARG_ERR											*/
/****************************************************************************/
int
cm_check_rjoin_arg(int argc, char **argv, int argmode,
                   void *opt, int *key, char **master, char **trans)
{

    register i;                 /*      起動アーギュメントインデックス   */
    int *ngflg;                 /*      アンマッチ出力オプション            */
    char **umostr;              /*      アンマッチ出力文字列                  */
    int len;                    /*      レングス                                            */

/*--------------------------------------------------------------------------*/

    /*      アーギュメントの数をチェック                                                                              */
    if (argc < MIN_ARGS || MAX_ARGS < argc) {
        fprintf(stderr, "アーギュメントの数が不正です。\n");
        return (ARG_ERR);
    }

        /*----------------------------------------------------------------------*/
    /*      区分に応じてオプションを設定                                                                              */
        /*----------------------------------------------------------------------*/
    i = 1;
    if (argmode == ARGMODE_NGFLG) {
        ngflg = (int *) opt;
        *ngflg = strcmp(OPT_UNMATCH_OUT, argv[i]) ? 0 : 1;
        if (*ngflg)
            i++;
    } else if (argmode == ARGMODE_UMOSTR) {
        umostr = (char **) opt;
        len = strlen(OPT_UNMATCH_OUTSTR);
        *umostr = memcmp(OPT_UNMATCH_OUTSTR, argv[i], len)
            ? NULL : &argv[i][len];
        if (*umostr != NULL) {
            if (strlen(*umostr) < 1) {
                fprintf(stderr,
                        "アンマッチ出力文字列が指定されていません。\n");
                return (ARG_ERR);
            }
            i++;
        }
    }

        /*----------------------------------------------------------------------*/
    /*      トランのキー項目Ｎｏを取得                                                                                 */
        /*----------------------------------------------------------------------*/
    len = strlen(OPT_TRN_KEY);
    if (memcmp(OPT_TRN_KEY, argv[i], len)) {
        fprintf(stderr, "%s。\n", argv[i][0] != '+' ?
                "トランのキー項目Ｎｏが指定されていません" :
                "アンマッチ出力オプションの指定が誤っています");
        return (ARG_ERR);
    }
    if ((*key = atoi(&argv[i][len])) < 1) {
        fprintf(stderr, "トランのキー項目Ｎｏ（１～）が不正です。\n");
        return (ARG_ERR);
    }
    i++;

    /* DEL 2014.12.16 Hien.TT */
//      *master = argv[i] ;                                                     /*      ﾏｽﾀﾌｧｲﾙﾊﾟｽﾎﾟｲﾝﾀを設定  */
//      i ++ ;

//      *trans = i < argc ? argv[i] : NULL ;            /*      ﾄﾗﾝﾌｧｲﾙﾊﾟｽﾎﾟｲﾝﾀを設定  */
    /* DEL 2014.12.16 Hien.TT */
    /* ADD 2014.12.16 Hien.TT */
    if (strcmp(argv[i], "-") == 0) {
        *master = NULL;         /*      ﾏｽﾀﾌｧｲﾙﾊﾟｽﾎﾟｲﾝﾀを設定  */
    } else {
        *master = argv[i];      /*   ﾏｽﾀﾌｧｲﾙﾊﾟｽﾎﾟｲﾝﾀを設定  */
    }
    i++;

    if (i < argc) {
        if (strcmp(argv[i], "-") == 0)
            *trans = NULL;      /*       ﾄﾗﾝﾌｧｲﾙﾊﾟｽﾎﾟｲﾝﾀを設定  */
        else {
            *trans = argv[i];   /*    ﾄﾗﾝﾌｧｲﾙﾊﾟｽﾎﾟｲﾝﾀを設定  */
        }
    } else {
        *trans = NULL;          /*       ﾄﾗﾝﾌｧｲﾙﾊﾟｽﾎﾟｲﾝﾀを設定  */
    }

    if (*master == NULL && *trans == NULL) {
        fprintf(stderr,
                "異常終了 標準入力から複数指定することができません。\n");
        return (ARG_ERR);
    }
    /* ADD 2014.12.16 Hien.TT */
    return (NRML_END);
}

/*** End Of File ***/
