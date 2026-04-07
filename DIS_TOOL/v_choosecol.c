/****************************************************************************/
/*  システム名称：  汎用ＣＩＳツール                                        */
/*  コマンド名称：  v_choosecol                                           */
/*  機能        ：    指定したフィールドだけを出力する                      */
/*　　・                                                                    */
/*  制約事項    ：                                                          */
/*　　・インプットはフィールドセパレータは半角スペースとする。              */
/*　　・インプットのレコード(行)レングスはLFを含めて最大6144バイトとする。  */
/*                                                                          */
/*  使用法      ：  v_choosecol $ < input > outputfile                    */
/*                  input                                                   */
/*  ステータス  ：  0 : 正常終了                                            */
/*                  1 : 警告終了（出力失敗データ有り）                      */
/*                  2 : アーギュメント不正                                  */
/*                  3 : ファイルオープンエラー                              */
/*                  4 : ファイルリードエラー                                */
/*                  5 : メモリ確保エラー                                    */
/*  作成者      ：  北条                                                    */
/*  作成日      ：  2001/11/05                                              */
/*  修正履歴    ：                                                          */
/*  No. 修正日付    修正者            修正内容                              */
/*  001 2001/11/23  北条        標準入力対応                                */
/*  002 2002/02/06  木下        フィールドの文字指定の時に存在しない開始    */
/*                              位置を指定した場合は処理しないようにした    */
/*　003 2002/10/28　川村　     最大レコードサイズ変更(1024Byte=>3072byte)   */
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
#define     ARGS_MIN        2
#define     END_MARK        "END"
#define     FS_HIFUN        "-"
#define     FS_CORON        ":"

#ifdef	NEW_GETFMT
#define     MAX_FSNUM       400     /* 最大フィールド数 (フィールド定義事前決定の為に追加) */
#endif	/* NEW_GETFMT */

/*--------------------------------------------------------------------------*/
/*  内部関数定義                                                            */
/*--------------------------------------------------------------------------*/
void usage();

#ifdef  EXTERNAL_TABLE
/* 環境によっては、スタックサイズを超える場合もあるため、外部に移動 */
char    fld_tbl[MAUNIT_ITBL][MAX_RECLEN];/* フィールドＴＢＬ            */
#endif  /* EXTERNAL_TABLE */

main(int argc,char *argv[])
{
    char    read_buf[MAX_RECLEN];           /* リードバッファ               */
    char    parm_buf[MAX_RECLEN];           /* パラメータバッファ           */
    char    *start_parm_buf;                /* パラメータ先頭(範囲指定)     */
    char    *end_parm_buf;                  /* パラメータ最終(範囲指定)     */
    char    output_buf[MAX_RECLEN];         /* 出力バッファ                 */
    char    char_buf[MAX_RECLEN];           /* 文字列格納ＴＢＬ             */
    char    temp_buf[MAX_RECLEN];           /* 中間バッファ                 */
    char    *fld_cut;                       /* フィールドカットポインタ     */
#ifndef EXTERNAL_TABLE
/* 環境によっては、スタックサイズを超える場合もあるため、外部に移動 */
    char    fld_tbl[MAUNIT_ITBL][MAX_RECLEN];/* フィールドＴＢＬ            */
#endif  /* !EXTERNAL_TABLE */
    char    *key_point;                     /* キーフィールドポインタ       */
    int     loop_cnt;                       /* ループカウンタ               */
    int     parm_cnt;                       /* パラメータカウンタ           */
    int     fld_point;                      /* フィールド位置               */
    int     char_s;                         /* 文字の開始位置               */
    int     char_cnt;                       /* 文字数                       */
    int     fld_cnt;                        /* フィールドカウンタ           */
    int     fld_cnt_buf;                    /* 減値フィールド               */
    int     start_fld;                      /* 開始フィールド数             */
    int     end_fld;                        /* 終了フィールド数             */
    int     output_len;                     /* アウトプットバッファレングス */
    int     char_len;                       /* 文字列レングス               */
    int     fld_len;                        /* フィールドレングス           */
    int     char_get_flg;                   /* 文字取得フラグ               */
    int     parm_max;                       /* パラメータ最大数             */
    FILE    *fp;                            /* ファイルポインタ             */

#ifdef	NEW_GETFMT
    /* 出力フィールド定義テーブル */
    struct {
        int cnt;        /* フィールド定義数 */
        struct {
            int no;         /* フィールド番号 */
            int start;      /* 抽出開始位置 */
            int end;        /* 抽出終了位置 */
        } fld[MAX_FSNUM];
    } Out_fmt;
#endif	/* NEW_GETFMT */

    /*  アーギュメントの数をチェック                                        */
    if(argc < ARGS_MIN ){
        usage();
    }

    /*  オプションチェック                                                  */
    if (NULL != strchr(argv[1], 'd')){
        fld_point = 0;
        char_s = 0;
        char_cnt = 0;

        /*  表示方法の切り出し                                              */
        fld_point = atoi(strtok(argv[2], "."));
        if (NULL != (key_point = strtok(NULL, "."))){
            char_s = atoi(key_point);
            if (NULL != (key_point = strtok(NULL, "."))){
                char_cnt = atoi(key_point);
            }
        }

        /*  指定フィールドがなければ終了                                    */
        if (fld_point == 0){
            fprintf(stderr, "パラメータ指定が異常です。\n");
            return ARG_ERR;
        }

        /*  文字列をコピー                                                  */
        strcpy(char_buf, argv[3]);

        /*  初期設定                                                        */
        output_buf[0] = '\0';

        /*  文字抽出指定がないためそのまま出力バッファにコピー              */
        if (0 == char_s){
            strcpy(output_buf, char_buf);
        }
        /*  文字数の指定がない場合                                          */
        else if (0 == char_cnt){
            char_len = strlen(char_buf);
            for (loop_cnt = 0; loop_cnt <= char_len - char_s + 1; loop_cnt++){
                output_buf[loop_cnt] = char_buf[loop_cnt + char_s - 1];
            }
        }
        /*  開始位置＆文字数指定あり                                        */
        else {
            /*  文字数が文字列の範囲内の場合                                */
            char_len = strlen(char_buf);
            if (char_len > char_s + char_cnt - 1){
                for (loop_cnt = 0; loop_cnt <= char_cnt - 1; loop_cnt++){
                    output_buf[loop_cnt] = char_buf[loop_cnt + char_s - 1];
                }
                output_buf[loop_cnt] = '\0';
            }
            /*  文字数が文字列より多い場合                                  */
            else {
                char_len = strlen(char_buf);
                for (loop_cnt = 0; loop_cnt <= char_len - char_s + 1; loop_cnt++){
                    output_buf[loop_cnt] = char_buf[loop_cnt + char_s - 1];
                }
            }
        }
        fprintf(stdout, "[%s]\n", output_buf);
    }
    else{
        /*  ファイルオープン                                                */
        fp = fopen(argv[argc - 1], "r");
        if (fp == NULL){
            fp = stdin;
            parm_max = argc - 1;
        }
        else{
            parm_max = argc - ARGS_MIN;
        }
        /*
        if(argc > 2){
            fp = fopen(argv[argc - 1],"r");
            if (fp == NULL){
                fprintf(stderr,"File Open Error\n");
                return FOPEN_ERR;
            }
        }
        else {
            fp = stdin;
        }
        */

        /*  初期処理                                                        */
        char_get_flg = 0;

#ifdef	NEW_GETFMT
      /* パラメータ解析(出力フォーマット取得) */
        for (Out_fmt.cnt = 0; Out_fmt.cnt < parm_max; Out_fmt.cnt++) {

            /*  引数をバッファにコピー */
            strcpy(parm_buf, argv[Out_fmt.cnt + 1]);

            if (NULL != strstr(parm_buf, FS_HIFUN)){ /* XXX-XXX 形式指定 */

                Out_fmt.fld[Out_fmt.cnt].no = -1;   /* 範囲指定 */

                /* 開始/終了位置抽出 */
                start_parm_buf = strtok(parm_buf, FS_HIFUN);
                end_parm_buf = strtok(NULL, FS_HIFUN);

                if (NULL != strstr(start_parm_buf, END_MARK)){
                  /* [ＥＮＤ]指定から開始フィールドを指定 */
                    if (NULL != strstr(start_parm_buf, FS_CORON)){
                      /* 最終フィールドから－ｎフィールド指定 */
                        fld_cnt_buf = atoi(strtok(strtok(start_parm_buf, END_MARK), FS_CORON));
                        Out_fmt.fld[Out_fmt.cnt].start = (fld_cnt_buf > 0 ? fld_cnt_buf * -1 : 0);
                    } else {    /* 最終フィールドのみの指定 */
                        Out_fmt.fld[Out_fmt.cnt].start = 0;
                    }
                } else {    /* 開始フィールド直接指定 */
                    fld_cnt_buf = atoi(start_parm_buf);
                    Out_fmt.fld[Out_fmt.cnt].start = (fld_cnt_buf > 0 ? fld_cnt_buf : 1);
                }

                if (NULL != strstr(end_parm_buf, END_MARK)){
                  /* [ＥＮＤ]指定から終了フィールドを指定 */
                    if (NULL != strstr(end_parm_buf, FS_CORON)){
                      /* 最終フィールドから－ｎフィールド指定 */
                        fld_cnt_buf = atoi(strtok(strtok(end_parm_buf, END_MARK), FS_CORON));
                        Out_fmt.fld[Out_fmt.cnt].end = (fld_cnt_buf > 0 ? fld_cnt_buf * -1 : 0);
                    } else {    /* 最終フィールドのみの指定 */
                        Out_fmt.fld[Out_fmt.cnt].end = 0;
                    }
                } else {    /* 終了フィールド直接指定 */
                    fld_cnt_buf = atoi(end_parm_buf);
                    Out_fmt.fld[Out_fmt.cnt].end = (fld_cnt_buf > 0 ? fld_cnt_buf : 0);
                }
            } else if (NULL != strstr(parm_buf, END_MARK)){ /* [ＥＮＤ]付き指定 */
                Out_fmt.fld[Out_fmt.cnt].no = 0;    /* 最終フィールドからの相対位置指定 */
                Out_fmt.fld[Out_fmt.cnt].end = 0;

                if (NULL != strstr(parm_buf, FS_CORON)){
                  /* 最終フィールドから－ｎフィールド指定 */
                    fld_cnt_buf = atoi(strtok(strtok(parm_buf, END_MARK), FS_CORON));
                    Out_fmt.fld[Out_fmt.cnt].start = (fld_cnt_buf > 0 ? fld_cnt_buf * -1 : 0);
                } else {    /* 最終フィールドのみの指定 */
                    Out_fmt.fld[Out_fmt.cnt].start = 0;
                }
            } else { /* フィールド番号直接指定 */
                fld_point = 0;
                char_s = 0;
                char_cnt = 0;

                /* 表示方法の切り出し */
                fld_point = atoi(strtok(parm_buf, "."));
                if (NULL != (key_point =strtok(NULL, "."))){
                    char_s = atoi(key_point);
                    if (NULL != (key_point =strtok(NULL, "."))){
                        char_cnt = atoi(key_point);
                    }
                }

                if (fld_point == 0){
                  /* 指定フィールドがなければ終了 */
                    fprintf(stderr, "パラメータ指定が異常です。\n");
                    return ARG_ERR;
                }
                Out_fmt.fld[Out_fmt.cnt].no    = (fld_point > 0 ? fld_point : 1);
                Out_fmt.fld[Out_fmt.cnt].start = (char_s > 0 ? char_s : 1);
                Out_fmt.fld[Out_fmt.cnt].end   = (char_cnt > 0 ? char_cnt : 0);
            }
        }
#endif	/* NEW_GETFMT */

        /*  レコード文字列取得                                              */
        while(1){
            /*  リード処理                                                  */
            if (NULL == fgets(read_buf, MAX_RECLEN, fp)){
                break;
            }

            /*  初期設定                                                    */
            output_buf[0] = '\0';
            temp_buf[0] = '\0';
            fld_cnt = 0;

            /*  項目切り出し                                                */
            while(1){
                fld_cut = strtok(fld_cnt ? NULL : read_buf, FLD_SEP_STR);
                if(fld_cut == NULL){
                    break;
                }
                strcpy(fld_tbl[fld_cnt++], fld_cut);
            }

            /*  フィールド数チェック(０カウントならcontinue)                */
            if(fld_cnt == 0){
                continue;
            }

#ifdef	NEW_GETFMT
            for (parm_cnt = 0; parm_cnt < parm_max; parm_cnt++){
                if (Out_fmt.fld[parm_cnt].no == -1) {   /* XXX-XXX 形式指定 */
                  /* 開始フィールド */
                    if (Out_fmt.fld[parm_cnt].start > 0){   /* 直接指定 */
                        start_fld = (Out_fmt.fld[parm_cnt].start - 1);
                        start_fld = (start_fld < fld_cnt ? start_fld : fld_cnt - 1);
                    } else {            /* 最終フィールドからの相対指定 */
                        start_fld = (fld_cnt - 1) + Out_fmt.fld[parm_cnt].start;
                        start_fld = (start_fld < 0 ? 0 : start_fld);
                    }
                  /* 終了フィールド */
                    if (Out_fmt.fld[parm_cnt].end > 0){     /* 直接指定 */
                        end_fld = (Out_fmt.fld[parm_cnt].end - 1);
                        end_fld = (end_fld < fld_cnt ? end_fld : fld_cnt - 1);
                    } else {            /* 最終フィールドからの相対指定 */
                        end_fld = (fld_cnt - 1) + Out_fmt.fld[parm_cnt].end;
                        end_fld = (end_fld < 0 ? 0 : end_fld);
                    }
                  /* 大小チェック */
                    if(start_fld > end_fld) {   /* 大小反転 */
                        int tmp;
                        tmp = start_fld; start_fld = end_fld; end_fld = tmp;
                    }
                  /* アウトプットバッファに格納 */
                    for (loop_cnt = start_fld; loop_cnt <= end_fld; loop_cnt++) {
                        strcat(output_buf, fld_tbl[loop_cnt]);
                        strncat(output_buf, " ", 1);
                    }
                } else if (Out_fmt.fld[parm_cnt].no == 0){  /* [ＥＮＤ]相対指定 */
                    start_fld = (fld_cnt - 1) + Out_fmt.fld[parm_cnt].start;
                    start_fld = (start_fld < 0 ? 0 : start_fld);
                    strcat(output_buf, fld_tbl[start_fld]);
                    strncat(output_buf, " ", 1);
                } else {                            /* フィールド番号直接指定 */
                    fld_point = Out_fmt.fld[parm_cnt].no;
                    char_s = Out_fmt.fld[parm_cnt].start;
                    char_cnt = Out_fmt.fld[parm_cnt].end;

                    if (0 == char_s) {          /* 開始位置指定無し */
                        if(fld_point <= fld_cnt){
                            strcat(output_buf, fld_tbl[fld_point - 1]);
                            strncat(output_buf, " ", 1);
                        }
                    } else if (0 == char_cnt){  /* 文字数指定無し */
                        if(fld_point <= fld_cnt){
                            fld_len = strlen(fld_tbl[fld_point - 1]);
                            if(char_s <= fld_len){
                                char *tptr = &(fld_tbl[fld_point - 1][0]);
                                strcat(output_buf, &(tptr[char_s - 1]));
                                strncat(output_buf, " ", 1);
                            }
                        }
                    } else {            /* 開始位置＆文字数指定あり */
                        if(fld_point <= fld_cnt){
                            fld_len = strlen(fld_tbl[fld_point - 1]);
                            if(char_s <= fld_len){
                                char *tptr = &(fld_tbl[fld_point - 1][0]);
                                if (fld_len < char_s + char_cnt - 1){
                                    char_cnt = fld_len - char_s + 1;
                                }
                                strncat(output_buf, &(tptr[char_s - 1]), char_cnt);
                                strncat(output_buf, " ", 1);
                            }
                        }
                    }
                }
            }
#else	/* !NEW_GETFMT */
            /*  パラメータの数だけループ                                    */
            /*for (parm_cnt = 1; parm_cnt <= argc - ARGS_MIN; parm_cnt++){*/
            for (parm_cnt = 1; parm_cnt <= parm_max; parm_cnt++){

                /*  引数をバッファにコピー                                  */
                strcpy(parm_buf, argv[parm_cnt]);

                
                /*  [：]付きの引数                                          */
                /*if (NULL != strstr(parm_buf, FS_CORON)){*/
                if (NULL != strstr(parm_buf, FS_HIFUN)){

                    /*  開始項抽出                                          */
                    /*start_parm_buf = strtok(parm_buf, FS_CORON);*/
                    start_parm_buf = strtok(parm_buf, FS_HIFUN);
                    /*end_parm_buf = strtok(NULL, FS_CORON);*/
                    end_parm_buf = strtok(NULL, FS_HIFUN);

                    /*  [ＥＮＤ]指定からフィールドを指定                    */
                    if (NULL != strstr(start_parm_buf, END_MARK)){
                        /*  最終フィールドから－ｎフィールド指定            */
                        /*if (NULL != strstr(start_parm_buf, FS_HIFUN)){*/
                        if (NULL != strstr(start_parm_buf, FS_CORON)){
                            /*fld_cnt_buf = atoi(strtok(strtok(start_parm_buf, END_MARK), FS_HIFUN));*/
                            fld_cnt_buf = atoi(strtok(strtok(start_parm_buf, END_MARK), FS_CORON));
                            start_fld = (fld_cnt - 1) - fld_cnt_buf;
                        }
                        /*  最終フィールドのみの指定                        */
                        else {
                            start_fld = fld_cnt - 1;
                        }
                    }
                    /*  フィールド指定                                      */
                    else{
                        start_fld = atoi(start_parm_buf) - 1;
                    }

                    /*  終了項抽出                                          */
                    /*  [ＥＮＤ]指定からフィールドを指定                    */
                    if (NULL != strstr(end_parm_buf, END_MARK)){
                        /*  最終フィールドから－ｎフィールド指定            */
                        /*if (NULL != strstr(end_parm_buf, FS_HIFUN)){*/
                        if (NULL != strstr(end_parm_buf, FS_CORON)){
                            /*fld_cnt_buf = atoi(strtok(strtok(end_parm_buf, END_MARK), FS_HIFUN));*/
                            fld_cnt_buf = atoi(strtok(strtok(end_parm_buf, END_MARK), FS_CORON));
                            end_fld = (fld_cnt - 1) - fld_cnt_buf;
                        }
                        /*  最終フィールドのみの指定                        */
                        else {
                            end_fld = fld_cnt - 1;
                        }
                    }
                    /*  フィールド指定                                      */
                    else{
                        end_fld = atoi(end_parm_buf) - 1;
                    }

                    /*  指定フィールドと最終フィールドの大小チェック        */
                    if(end_fld > fld_cnt - 1){
                        end_fld = fld_cnt - 1;
                    }

                    /*  アウトプットバッファに格納                          */
                    for (loop_cnt = start_fld; loop_cnt <= end_fld; loop_cnt++){
                        strcat(output_buf, fld_tbl[loop_cnt]);
                        strncat(output_buf, " ", 1);
                    }
                }
                /*  [ＥＮＤ]付きの引数                                      */
                else if (NULL != strstr(parm_buf, END_MARK)){

                    /*  最終フィールドから－ｎフィールド指定                */
                    /*if (NULL != strstr(parm_buf, FS_HIFUN)){*/
                    if (NULL != strstr(parm_buf, FS_CORON)){
                        /*fld_cnt_buf = atoi(strtok(strtok(parm_buf, END_MARK), FS_HIFUN));*/
                        fld_cnt_buf = atoi(strtok(strtok(parm_buf, END_MARK), FS_CORON));
                        strcat(output_buf, fld_tbl[(fld_cnt - 1) - fld_cnt_buf]);
                        char_get_flg = 1;
                    }
                    /*  最終フィールドのみの指定                            */
                    else {
                        strcat(output_buf, fld_tbl[fld_cnt - 1]);
                        char_get_flg = 1;
                    }
                }
                else{

                    fld_point = 0;
                    char_s = 0;
                    char_cnt = 0;

                    /*  表示方法の切り出し                                  */
                    fld_point = atoi(strtok(parm_buf, "."));
                    if (NULL != (key_point =strtok(NULL, "."))){
                        char_s = atoi(key_point);
                        if (NULL != (key_point =strtok(NULL, "."))){
                            char_cnt = atoi(key_point);
                        }
                    }

                    /*  指定フィールドがなければ終了                        */
                    if (fld_point == 0){
                        fprintf(stderr, "パラメータ指定が異常です。\n");
                        return ARG_ERR;
                    }

                    /*  文字抽出指定がないためそのまま出力バッファにコピー  */
                    else if (0 == char_s){
                        if(fld_point <= fld_cnt){
                            strcpy(temp_buf, fld_tbl[fld_point - 1]);
                            char_get_flg = 1;
                        }
                    }

                    /*  文字数の指定がない場合                              */
                    else if (0 == char_cnt){
                        if(fld_point <= fld_cnt){
                            fld_len = strlen(fld_tbl[fld_point - 1]);
                            if(char_s <= fld_len){
                                for (loop_cnt = 0; loop_cnt <= fld_len - char_s + 1; loop_cnt++){
                                    temp_buf[loop_cnt] = fld_tbl[fld_point - 1][loop_cnt + char_s - 1];
                                }
                                char_get_flg = 1;
                            }
                        }
                    }
                    /*  開始位置＆文字数指定あり                            */
                    else {
                        /*  文字数が文字列の範囲内の場合                    */
                        if(fld_point <= fld_cnt){
                            fld_len = strlen(fld_tbl[fld_point - 1]);
                            if (fld_len > char_s + char_cnt - 1){
                                for (loop_cnt = 0; loop_cnt <= char_cnt - 1; loop_cnt++){
                                    temp_buf[loop_cnt] = fld_tbl[fld_point - 1][loop_cnt + char_s - 1];
                                }
                                temp_buf[loop_cnt] = '\0';
                            }
                            /*  文字数が文字列より多い場合                  */
                            else {
                                for (loop_cnt = 0; loop_cnt <= fld_len - char_s + 1; loop_cnt++){
                                    temp_buf[loop_cnt] = fld_tbl[fld_point - 1][loop_cnt + char_s - 1];
                                }
                            }
                            char_get_flg = 1;
                        }
                    }
                    if(char_get_flg == 1){
                        strcat(output_buf, temp_buf);
                    }
                }
                /*  文字取得できた時のみスペースをセットする                */
                if(char_get_flg == 1){
                    strncat(output_buf, " ", 1);
                    char_get_flg = 0;
                }
            }
#endif	/* !NEW_GETFMT */
            output_len = strlen(output_buf) - 1;
            output_buf[output_len] = '\0';
#ifdef	NEW_GETFMT
            puts(output_buf);
#else	/* !NEW_GETFMT */
            fprintf(stdout, "%s\n", output_buf);
#endif	/* !NEW_GETFMT */
        }
    }

    return NRML_END;
}
/****************************************************************************/
/*  関数名称：  usage（使用方法表示）                                       */
/*  処理概要：  エラー処理                                                  */
/*  戻り値  ：  ARG_ERR                                                     */
/****************************************************************************/
void usage()
{
    puts("使用方法: @choosecol $1 $2 ．．．< inputfile > outputfile");
    exit (1);
}
