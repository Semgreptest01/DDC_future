/****************************************************************************/
/*  システム名称：  汎用ＣＩＳツール                                        */
/*  コマンド名称：  v_uniqbytop                                           */
/*  機能        ：  キーが同じ先頭行を表示する                              */
/*  制約事項    ：                                                          */
/*　　・                                                                    */
/*  使用法      ：  v_uniqbytop $1 $2 < inputfile > outputfile            */
/*  ステータス  ：  0 : 正常終了                                            */
/*                  1 : アーギュメント不正                                  */
/*  作成者      ：  北条                                                    */
/*  作成日      ：  2001/11/09                                              */
/*  修正履歴    ：                                                          */
/*  No. 修正日付    修正者          修正内容                                */
/*  001 2001/11/22   北条   標準入力対応                                    */
/*　002 2002/11/08　 川村　 最大レコードサイズ変更(1024Byte=>3072byte)      */
/*                          Usageメッセージ修正                             */
/*　003 2002/11/12　 川村　 最大キー指定数変更(20=>40)                      */
/*                          キーフィールドコンペア処理修正（１フィールド目  */
/*                          から比較を指定開始位置から比較に変更）          */
/*                          gets => fgets に変更                            */
/*　004 2003/01/29　 川村　 キー指定チェック(開始が終了より大きい場合ＮＧ)  */
/*　005 2003/02/05　寺澤　　　 最大レコードサイズ変更(3072Byte=>6144byte)   */
/****************************************************************************/
/*--------------------------------------------------------------------------*/
/*  ヘッダーファイルインクルード定義                                        */
/*--------------------------------------------------------------------------*/
/*  System Headder File Include                                             */
#include <stdio.h>
#include <string.h>

/*--------------------------------------------------------------------------*/
/*  ｄｅｆｉｅｎ定義                                                        */
/*--------------------------------------------------------------------------*/
#define     ARGS_MIN        3
#define     ARGS_MAX        4    
#define     MAX_RECLEN      6144                /*  最大レコード長＋１      */
#define     MAX_KEYNUM      40                  /*  最大指定キー数          */

void usage();

main(int argc,char *argv[])
{
    int     i0,iSTR,iEND,iKB = 0,iINPUT;
    int     keycnt;                            /* キーＴＢＬカウンタ        */
    char    wC0[MAX_RECLEN],wC1[MAX_RECLEN],wC2[81],*wC3,wKEY[MAX_KEYNUM][81];
    FILE    *wF1;

    /*  アーギュメントの数をチェック     */
    if(argc < ARGS_MIN || ARGS_MAX < argc){
        usage();
    }

    /*ＫＥＹ項目数取得*/
    iSTR = atoi(argv[1]);
    iEND = atoi(argv[2]);

/* ADD 2002.01.29 Kawamura */
    if (iSTR > iEND){
        usage();
    }
/* ADD 2002.01.29 Kawamura */

    if (iEND + 1 - iSTR > MAX_KEYNUM){
        printf("キー指定の最大数は[%d]\n", MAX_KEYNUM);
        exit(1);
    }

    /*ファイルオープン*/
    if (4 <= argc){
        if (NULL == (wF1 = fopen(argv[3],"r"))){
            fprintf(stderr,"File open error");
            return 1;
        }
/* DEL 2002.11.12 Kawamura --
        else{
            iINPUT = 0;
        }
-- DEL 2002.11.12 Kawamura -- */
    }
    else{
        wF1 = stdin;
/* DEL 2002.11.12 Kawamura --
        iINPUT = 1;
-- DEL 2002.11.12 Kawamura -- */
    }

    /*文字列取得*/
    while(1){
/* DEL 2002.11.12 Kawamura --
        if (0 == iINPUT){
-- DEL 2002.11.12 Kawamura -- */
            if (NULL == fgets(wC0,MAX_RECLEN,wF1)){
                break;
            }
/* DEL 2002.11.12 Kawamura --
        }
        else {
            if (NULL == gets(wC0)){
                break;
            }
        }
-- DEL 2002.11.12 Kawamura -- */

        /*項目切り出し*/
        strcpy(wC1,wC0);
        keycnt = 0;                           /* キーＴＢＬカウンター初期化 */
        for(i0 = 0;iEND > i0; i0++){
            if (0 == i0){
                strcpy(wC2,strtok(wC1," "));
            }
            else {
                strcpy(wC2,strtok(NULL," "));
            }
/* DELETE 2002.11.12 Kawamura
            if (iSTR - 1 <= i0 && 0 != strcmp(wKEY[i0],wC2)){
                iKB = 1;
            }
            strcpy(wKEY[i0],wC2);
   DELETE 2002.11.12 Kawamura */
            /* 開始位置以上ならキーを比較                                   */
            if (iSTR -1 <= i0){
                if (0 != strcmp(wKEY[keycnt],wC2)){
                    iKB = 1;
                }
                strcpy(wKEY[keycnt],wC2);
                keycnt++;
            }
        }

        /*キーブレーク時表示*/
        if (1 == iKB){
/* DEL 2002.11.12 Kawamura --
            if (0 == iINPUT){
-- DEL 2002.11.12 Kawamura -- */
                printf("%s",wC0);
/* DEL 2002.11.12 Kawamura --
            }
            else{
                printf("%s\n",wC0);
            }
-- DEL 2002.11.12 Kawamura -- */
            iKB = 0;
        }
    }
    return 0;
}

/****************************************************************************/
/*  関数名称：  usage（使用方法表示）                                       */
/*  処理概要：  エラー処理                                                  */
/*  戻り値  ：  ARG_ERR                                                     */
/****************************************************************************/
void usage()
{
    puts("使用方法: v_uniqbytop $1 $2 < inputfile > outputfile");
    exit (1);
}
