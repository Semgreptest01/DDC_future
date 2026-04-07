/****************************************************************************/
/*	システム名称	：	ＤＩＳユーティリティー								*/
/*	ライブラリ名称	：	共通ライブラリ（libdistool.a）						*/
/*	関数名称		：	 1.マスタファイルインコア（cm_load_master_file）	*/
/*	作成者			：	T.Akiyama (DIS)										*/
/*	作成日			：	2001/12/04											*/
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
#include		<stdlib.h>
#include		<string.h>
#include		<errno.h>

/*	User Headder File Include												*/
#include		"libdistool.inc"

/*--------------------------------------------------------------------------*/
/*	内部関数定義															*/
/*--------------------------------------------------------------------------*/
static	int		cm_set_other_item_tbl(char *ktptr, int recnum,
									  char **oitblptr, int *oitemlen) ;

/****************************************************************************/
/*	関数名称：	cm_load_master_file（マスタファイルインコア）				*/
/*	処理概要：	マスタ上の全レコードをインコアし、キー（第１項目固定）と	*/
/*				その他の項目（第２項目以降）へ分割する。					*/
/*				注）キー及びその他項目レングスは実データの最大レングスと	*/
/*					する。													*/
/*				注）指定されたその他項目レングスがＮＵＬＬポインタの場合は	*/
/*					キーのみインコアする。									*/
/*	入	力	：	fp		：	ファイルポインタ								*/
/*	出	力	：	recnum	：	レコード件数									*/
/*	出	力	：	ktblptr	：	キーＴＢＬポインタ								*/
/*							（キーＴＢＬ領域確保及び設定も当関数内で行う）	*/
/*	出	力	：	keylen	：	キーレングス									*/
/*	出	力	：	oitblptr：	その他項目ＴＢＬ（キー以外の項目）ポインタ		*/
/*							（項目ＴＢＬ領域確保及び設定も当関数内で行う）	*/
/*	出	力	：	oitemlen：	その他項目レングス								*/
/*	戻り値	：	NRML_END／FREAD_ERR／MALLOC_ERR								*/
/****************************************************************************/
int
cm_load_master_file(FILE *fp, int *recnum,
					char **ktblptr, int *keylen, char **oitblptr, int *oitemlen)
{

	static	char	buf[MAX_RECLEN]	;		/*	リードバッファ				*/
	char			*keyptr			;		/*	キー						*/
	char			*ktptr			;		/*	キーターミネータ			*/
	int				keylen_rd		;		/*	リードレコードキーレングス	*/
	char			*wkptr			;		/*	ワークバッファ				*/
	register		i				;		/*	ループカウンター			*/
	int				sts				;		/*	終了ステータス				*/

/*--------------------------------------------------------------------------*/

	/*----------------------------------------------------------------------*/
	/*	各変数を初期化														*/
	/*----------------------------------------------------------------------*/
	*recnum	=  0 ;
	*ktblptr = NULL ;
	*keylen	= -1 ;
	if(oitemlen != NULL){
		*oitblptr = NULL ;
		*oitemlen = -1 ;
	}

	/*	ＥＯＦ迄繰返す														*/
	while(1){

		/*	指定マスタから１レコード（行）をシーケンシャルリード			*/
		if(fgets(buf, MAX_RECLEN, fp) == NULL){
			if(ferror(fp)){									/*	Read Error	*/
				fprintf(stderr, "fgets(%d) Master Read Error!! errno=%d:%s\n",
						MAX_RECLEN, errno, cm_get_errmsg(errno)) ;
				return(FREAD_ERR) ;
			}
			break ;											/*	EOF			*/
		}

		/*------------------------------------------------------------------*/
		/*	キーポインタ、キーレングスを取得し、ＮＵＬＬターミネート		*/
		/*------------------------------------------------------------------*/
		for(keyptr = buf; *keyptr == FLD_SEP; keyptr++) ;	/*	前ｽﾍﾟｰｽｶｯﾄ	*/
		if((ktptr = strchr(keyptr, FLD_SEP)) == NULL){		/*	ｷｰのみ		*/
			keylen_rd = strlen(keyptr) - 1 ;	/*	LFを除くﾚｺｰﾄﾞﾚﾝｸﾞｽ		*/
			*(keyptr + keylen_rd) = '\0' ;		/*	LFをｶｯﾄしてNULLﾀｰﾐﾈｰﾄ	*/
		}
		else{												/*	複数ﾌｨｰﾙﾄﾞ	*/
			*ktptr = '\0' ;					/*	ｾﾊﾟﾚｰﾀをｶｯﾄしてNULLﾀｰﾐﾈｰﾄ	*/
			keylen_rd = strlen(keyptr) ;	/*	ｷｰﾚﾝｸﾞｽ取得					*/
		}

		/*------------------------------------------------------------------*/
		/*	リードレコードのキーレングスが現在のキーレングスを超えた場合、	*/
		/*	新キーレングスとし、キーＴＢＬを新キーレングスで再構築する。	*/
		/*	（初回時はキーＴＢＬエリアの確保のみ）							*/
		/*------------------------------------------------------------------*/
		if(keylen_rd > *keylen){

			if(*recnum)		/*	新ｷｰﾚﾝｸﾞｽでｷｰTBLｴﾘｱを再確保					*/
				*ktblptr = realloc(*ktblptr, keylen_rd * (((*recnum) - 1)
											 / MAUNIT_KTBL + 1) * MAUNIT_KTBL) ;
			else			/*	初回時ｷｰTBLｴﾘｱ確保(0Bならﾀﾞﾐｰで1B分確保)	*/
				*ktblptr = malloc(keylen_rd ? (keylen_rd * MAUNIT_KTBL) : 1) ;
			if(*ktblptr == NULL){
				fprintf(stderr, "%salloc(%d) Error!! errno=%d:%s\n",
						*recnum ? "re" : "m",
						keylen_rd ? (keylen_rd * ((*recnum - 1) / MAUNIT_KTBL
						+ 1) * MAUNIT_KTBL) : 1, errno, cm_get_errmsg(errno)) ;
				return(MALLOC_ERR) ;
			}

			if(*recnum){					/*	ｷｰTBLｴﾘｱを再確保した場合	*/

				/*	新キーレングスに従い、キーをシフト（左詰め、空きＳＰ）	*/
				for(i = *recnum - 1; i > 0; i--){
					memmove(*ktblptr + (keylen_rd * i),
							*ktblptr + (*keylen * i), *keylen) ;
					memset(*ktblptr + (keylen_rd * i) + *keylen,
						   ' ', keylen_rd - *keylen) ;
				}
				memset(*ktblptr + *keylen, ' ', keylen_rd - *keylen) ;
			}

			*keylen = keylen_rd ;						/*	ｷｰﾚﾝｸﾞｽを変更	*/
		}

		/* 必要に応じてキーＴＢＬエリアを再確保								*/
		if((*recnum) && !((*recnum) % MAUNIT_KTBL)){
			*ktblptr = realloc(*ktblptr, *keylen * ((*recnum) / MAUNIT_KTBL + 1)
										 * MAUNIT_KTBL) ;
			if(*ktblptr == NULL){
				fprintf(stderr, "realloc(%d) Error!! errno=%d:%s\n",
						*keylen * ((*recnum) / MAUNIT_KTBL + 1) * MAUNIT_KTBL,
						errno, cm_get_errmsg(errno)) ;
				return(MALLOC_ERR) ;
			}
		}

		/* リードしたキーをキーＴＢＬへ設定									*/
		memcpy(*ktblptr + (*keylen * (*recnum)), keyptr, keylen_rd) ;
		if(keylen_rd < *keylen)
			memset(*ktblptr + (*keylen * (*recnum)) + keylen_rd,
				   ' ', *keylen - keylen_rd) ;

		if(oitemlen != NULL){				/*	その他項目TBL設定要の場合	*/

			/*	その他項目ＴＢＬを設定										*/
			if(sts = cm_set_other_item_tbl(ktptr, *recnum, oitblptr, oitemlen))
				return(sts) ;
		}

		(*recnum) ++ ;								/*	ﾚｺｰﾄﾞ件数を加算		*/
	}

	return(NRML_END) ;
}

/****************************************************************************/
/*	関数名称：	cm_set_other_item_tbl（その他項目ＴＢＬ設定）				*/
/*	処理概要：	キー項目以降の項目をフィールドセパレータで分割し、その他	*/
/*				項目ＴＢＬへ設定する。										*/
/*				注）その他項目レングスは実データの最大レングスとする。		*/
/*	入	力	：	ktptr	：	キー項目ターミネータポインタ					*/
/*	入	力	：	recnum	：	レコード件数									*/
/*	入出力	：	oitblptr：	その他（キー以外）項目ＴＢＬポインタ			*/
/*							（その他項目ＴＢＬ領域確保も当関数内で行う）	*/
/*	入出力	：	oitemlen：	その他項目レングス								*/
/*	戻り値	：	NRML_END／MALLOC_ERR										*/
/****************************************************************************/
int
cm_set_other_item_tbl(char *ktptr, int recnum, char **oitblptr, int *oitemlen)
{

	int			oitemlen_rd	;		/*	リードレコードその他項目レングス	*/
	register	i			;		/*	ループカウンター					*/
	char		*itmptr		;		/*	項目ポインタ						*/
	char		*tblptr		;		/*	その他項目ＴＢＬポインタ			*/
	int			setlen		;		/*	その他項目ＴＢＬ設定レングス		*/
	int			itmlen		;		/*	項目レングス						*/

/*--------------------------------------------------------------------------*/

	/*----------------------------------------------------------------------*/
	/*	リードレコードのその他項目レングスを取得							*/
	/*----------------------------------------------------------------------*/
	if(ktptr == NULL) oitemlen_rd = 1 ;		/*	ｷｰのみの場合は1B(NULL)		*/
	else{									/*	その他項目有りの場合		*/
		ktptr ++ ;							/*	その他項目の先頭へ			*/
		oitemlen_rd = strlen(ktptr) ;		/*	その他項目ﾚﾝｸﾞｽを設定		*/
	}

	/*----------------------------------------------------------------------*/
	/*	リードレコードのその他項目のレングスが現在のレングスを超えた場合、	*/
	/*	新レングスとし、その他項目ＴＢＬを新レングスで再構築する。			*/
	/*	（初回時はその他項目ＴＢＬエリアの確保のみ）						*/
	/*----------------------------------------------------------------------*/
	if(oitemlen_rd > *oitemlen){
		if(recnum)					/*	新ﾚﾝｸﾞｽでその他項目TBLｴﾘｱを再確保	*/
			*oitblptr = realloc(*oitblptr, oitemlen_rd * ((recnum - 1)
										   / MAUNIT_ITBL + 1) * MAUNIT_ITBL) ;
		else						/*	初回時その他項目TBLｴﾘｱ確保			*/
			*oitblptr = malloc(oitemlen_rd * MAUNIT_ITBL) ;
		if(*oitblptr == NULL){
			fprintf(stderr, "%salloc(%d) Error!! errno=%d:%s\n",
					recnum ? "re" : "m",
					oitemlen_rd * ((recnum - 1) / MAUNIT_ITBL + 1)
					* MAUNIT_ITBL, errno, cm_get_errmsg(errno)) ;
			return(MALLOC_ERR) ;
		}

		if(recnum){					/*	その他項目TBLｴﾘｱを再確保した場合	*/

			/*	新レングスに従い、その他項目をシフト（左詰め、空きＮＵＬＬ）*/
			for(i = recnum - 1; i > 0; i--){
				memmove(*oitblptr + (oitemlen_rd * i),
						*oitblptr + (*oitemlen * i), *oitemlen) ;
				memset(*oitblptr + (oitemlen_rd * i) + *oitemlen,
					   '\0', oitemlen_rd - *oitemlen) ;
			}
			memset(*oitblptr + *oitemlen, '\0', oitemlen_rd - *oitemlen) ;
		}

		*oitemlen = oitemlen_rd ;				/*	その他項目ﾚﾝｸﾞｽを変更	*/
	}

	/* 必要に応じてその他項目ＴＢＬエリアを再確保							*/
	if((recnum) && !((recnum) % MAUNIT_ITBL)){
		*oitblptr = realloc(*oitblptr, *oitemlen * (recnum / MAUNIT_ITBL + 1)
									   * MAUNIT_ITBL) ;
		if(*oitblptr == NULL){
			fprintf(stderr, "realloc(%d) Error!! errno=%d:%s\n",
					*oitemlen * (recnum / MAUNIT_ITBL + 1) * MAUNIT_ITBL,
					errno, cm_get_errmsg(errno)) ;
			return(MALLOC_ERR) ;
		}
	}

	/*----------------------------------------------------------------------*/
	/* リードレコードをフィールドセパレータで分割しその他項目ＴＢＬへ設定	*/
	/*----------------------------------------------------------------------*/
	tblptr = *oitblptr + *oitemlen * recnum ;
	setlen = 0 ;
	if(ktptr != NULL){						/*	ｷｰ以外の項目が存在する場合	*/
		for(i = 0, itmptr = ktptr;
			(itmptr = strtok(i ? NULL : ktptr, FLD_SEP_STR)) != NULL; i++){
			itmlen = strlen(itmptr) ;
			if(i){
				*tblptr = FLD_SEP ;
				tblptr ++ ;
				setlen	++ ;
			}
			memcpy(tblptr, itmptr, itmlen) ;
			tblptr += itmlen ;
			setlen += itmlen ;
		}
		if(setlen < *oitemlen)
			memset(tblptr, '\0', *oitemlen - setlen) ;
	}
	else *tblptr = '\0' ;					/*	ｷｰ項目のみの場合			*/

	return(NRML_END) ;
}
/*** End Of File ***/

