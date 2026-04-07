/****************************************************************************/
/*  システム名称：  汎用ＣＩＳツール                                        */
/*  コマンド名称：  v_cutcol                                              */
/*  機能        ：  指定したフィールド以外を出力する                        */
/*　　・                                                                    */
/*  制約事項    ：                                                          */
/*　　・インプットはフィールドセパレータは半角スペースとする。              */
/*　　・インプットのレコード（行）レングスはＬＦを含めて最大６１４４        */
/*      バイトとする。                                                      */
/*  使用法      ：  v_cutcol $ < input > outputfile                       */
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
/*  No. 修正日付    修正者          修正内容                                */
/*  001 2001/11/22  北条            標準入力対応                            */
/*  002 2002/01/16  前川            エラー処理対応                          */
/*  002 2002/02/06  木下            フィールドの文字指定の時に存在しない開始*/
/*                                  位置を指定した場合は処理しないようにした*/
/*  004 2002/10/28  H.Kawamura      標準入力修正                            */
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
#define     ARGS_MIN        2
#define     END_MARK        "END"
#define     FS_HIFUN        "-"
#define     FS_CORON        ":"

/*--------------------------------------------------------------------------*/
/*  内部関数定義                                                            */
/*--------------------------------------------------------------------------*/
void usage();

main(int argc,char *argv[])
{
    int     loop_cnt;                        /* ループカウンタ              */
    int     loop_cnt2;                       /* ループカウンタ              */
    int     fld_cnt;                         /* フィールドカウンタ          */
    int     notdisp_cnt;                     /* 出力するフィールドカウンタ  */
    int     notdisp_tbl[MAX_RECLEN];         /* 出力しないフィールドＴＢＬ  */
    int     output_len;                      /* アウトプットバッファレングス*/
    int     fld_cnt_buf;                     /* 減値フィールド              */
    int     start_fld;                       /* 開始フィールド数            */
    int     end_fld;                         /* 終了フィールド数            */
    int     parm_cnt;                        /* パラメータカウンタ          */
    int     match_flg;                       /* 出力フラグ                  */
    int     first_flg;                       /* １レコード目通過フラグ      */
    char    read_buf[MAX_RECLEN];            /* リードバッファ              */
    char    output_buf[MAX_RECLEN];          /* アウトプットバッファ        */
    char    fld_tbl[MAUNIT_ITBL][MAX_RECLEN];/* フィールドＴＢＬ            */
    char    parm_buf[MAX_RECLEN];            /* パラメータバッファ          */
    char    *fld_cut;                        /* フィールドカットポインタ    */
    char    *start_parm_buf;                 /* パラメータ先頭(範囲指定)    */
    char    *end_parm_buf;                   /* パラメータ最終(範囲指定)    */
    int     parm_max;                        /* パラメータ最大数            */
    FILE    *fp;                             /* ファイルポインタ            */

    /*  アーギュメントの数をチェック                                        */
    if(argc < ARGS_MIN ){
        usage();
    }    

    /*  ファイルオープン                                                */
    /*  最後のパラメータをＩＮＰＵＴファイルとしてＯＰＥＮ              */
    /*  ファイルが存在しなければ標準入力より読込む                      */
    fp = fopen(argv[argc - 1], "r");
    if (fp == NULL){
        fp = stdin;
        parm_max = argc - 1;
    }
    else{
        parm_max = argc - ARGS_MIN;
    }

    /* DELETE 2002.10.28 H.Kawamura
    if(argc > ARGS_MIN){
        fp = fopen(argv[argc - 1], "r");
        if (NULL == fp){
            fprintf(stderr,"File Open Error\n");
            return FOPEN_ERR;
        }
    }
    else{
        fp = stdin;
    }
    DELETE 2002.10.28 H.Kawamura */

    /*  初期設定                                                            */
    match_flg = 0;
    notdisp_cnt = 0;
    first_flg = 0;

    /*  レコード文字列取得                                                  */
    while(1){
        if (fgets(read_buf, MAX_RECLEN, fp) == NULL){
            break;
        }

        /*  初期設定                                                        */
        fld_cnt = 0;
        output_buf[0] = '\0';

        /*  項目切り出し                                                    */
        while(1){
            fld_cut = strtok(fld_cnt ? NULL : read_buf, FLD_SEP_STR);
            if(fld_cut == NULL){
                break;
            }
            strcpy(fld_tbl[fld_cnt++], fld_cut);
        }

        /*  パラメータの数だけループ                                    */
        if(first_flg == 0){
        /*  DELETE 2002.10.28 H.Kawamura
            for (parm_cnt = 1; parm_cnt <= argc - ARGS_MIN; parm_cnt++){ 
            DELETE 2002.10.28 H.Kawamura  */
            for (parm_cnt = 1; parm_cnt <= parm_max; parm_cnt++){

                /*    引数をバッファにコピー                                */
                strcpy(parm_buf, argv[parm_cnt]);
                
                /*    [：]付きの引数                                        */
                /*if (NULL != strstr(parm_buf, FS_CORON)){*/
                if (NULL != strstr(parm_buf, FS_HIFUN)){

                    /*    開始項抽出                                        */
                    /*start_parm_buf = strtok(parm_buf, FS_CORON);*/
                    start_parm_buf = strtok(parm_buf, FS_HIFUN);
                    /*end_parm_buf = strtok(NULL, FS_CORON);*/
                    end_parm_buf = strtok(NULL, FS_HIFUN);

                    /*    [ＥＮＤ]指定からフィールドを指定                */
                    if (NULL != strstr(start_parm_buf, END_MARK)){
                        /*    最終フィールドから－ｎフィールド指定        */
                        /*if (NULL != strstr(start_parm_buf, FS_HIFUN)){*/
                        if (NULL != strstr(start_parm_buf, FS_CORON)){
                            /*fld_cnt_buf = atoi(strtok(strtok(start_parm_buf, END_MARK), FS_HIFUN));*/
                            fld_cnt_buf = atoi(strtok(strtok(start_parm_buf, END_MARK), FS_CORON));
                            start_fld = (fld_cnt - 1) - fld_cnt_buf;
                        }
                        /*    最終フィールドのみの指定                    */
                        else {
                            start_fld = fld_cnt - 1;
                        }
                    }
                    /*    フィールド指定                                    */
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

                    /*  アウトプットバッファに格納                          */
                    for (loop_cnt = start_fld; loop_cnt <= end_fld; loop_cnt++){
                        notdisp_tbl[notdisp_cnt++] = loop_cnt;
                    }
                }
                /*  通常引数                                                */
                else{
                    /*    [ＥＮＤ]付きの引数                                    */
                    if (NULL != strstr(parm_buf, END_MARK)){
                        /*  最終フィールドから－ｎフィールド指定            */
                        /*if (NULL != strstr(parm_buf, FS_HIFUN)){*/
                        if (NULL != strstr(parm_buf, FS_CORON)){
                            /*fld_cnt_buf = atoi(strtok(strtok(parm_buf, END_MARK), FS_HIFUN));*/
                            fld_cnt_buf = atoi(strtok(strtok(parm_buf, END_MARK), FS_CORON));
                            notdisp_tbl[notdisp_cnt++] = (fld_cnt - 1) - fld_cnt_buf;
                            /*char_get_flg = 1;*/
                        }
                        /*  最終フィールドのみの指定                        */
                        else {
                            notdisp_tbl[notdisp_cnt++] = fld_cnt - 1;
                            /*char_get_flg = 1;*/
                        }
                    }
                    else{
                        notdisp_tbl[notdisp_cnt++] = atoi(argv[parm_cnt]) - 1;
                    }
                }
            }
            /*  １レコード目通過                                            */
            first_flg = 1;
        }

        /*    表示処理                                                    */
        for(loop_cnt = 0; loop_cnt <= fld_cnt - 1; loop_cnt++){
            for(loop_cnt2 = 0; loop_cnt2 < notdisp_cnt; loop_cnt2++){
                if(loop_cnt == notdisp_tbl[loop_cnt2]){
                    match_flg = 1;
                    break;
                }
            }
            if(match_flg == 0){
                strcat(output_buf, fld_tbl[loop_cnt]);
                strncat(output_buf, " ", 1);
            }
            match_flg = 0;
        }

        output_len = strlen(output_buf) - 1;
        output_buf[output_len] = '\0';
        fprintf(stdout, "%s\n", output_buf);        
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
    puts("使用方法: v_cutcol $1 $2 $3 < inputfile > outputfile");
    exit (1);
}
