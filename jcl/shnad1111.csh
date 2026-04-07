#!/bin/csh
#------------------------------------------------------------------------#
# ネット-名     : HNM_メニュー情報受信ファイル取出し＆ロード
# JOB-名        : HNM_メニュー情報load
# Shell-名      : shnad1111.csh
# 機能          : １．メニューＩＤ抽出
#               : ２．ＳＱＬ ＬＯＡＤ
#               : ３．ＡＮＡＬＹＺＥ
# 作成者        : M.Yamamoto
# 作成日        : 2025/12/01
# 修正履歴      : 
# Ｎｏ   修正日付   修正者     修正内容
#------------------------------------------------------------------------#
#------------------------------------------------------------------------#
#     ０． 初期処理部
#------------------------------------------------------------------------#
set SUBSRCDIR = "/prod/jcl/sub"
#----  ワークディレクトリの設定
source $SUBSRCDIR/hnad.src

#----  共通変数設定
source $SUBSRCDIR/common.src
if ( $status != $NORMAL ) then
  exit $ABEND
endif

#------------------------------------------------------------------------#
#     １．メニューＩＤを抽出
#------------------------------------------------------------------------#
#---   ファイル存在チェック
set CHECKFILE=$WKDIR/hnt401p_001.tsv
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL )  then
    sync    #メモリとＤＩＳＫの同期化実施
    echo "$SHELLNAME:t :  ABEND time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
    exit $ABEND
endif

/bin/rm -f $WKDIR/hnw401p_001.tsv

awk '/^.....HN_/{print }' $WKDIR/hnt401p_001.tsv > $WKDIR/hnw401p_001.tsv
if ( $status != $NORMAL ) then
    sync    #メモリとＤＩＳＫの同期化実施
    echo "$SHELLNAME:t :  ABEND time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
   	exit $ABEND
endif
#------------------------------------------------------------------------#
#     ２．ＳＱＬ ＬＯＡＤ
#------------------------------------------------------------------------#
set SQL_NAME=sqlplus
set CTL_FILE="${CTLDIR}/hnm001.ctl"
set DAT_FILE=$WKDIR/hnw401p_001.tsv

#---   ロード用変数定義
set USER_ID = $ORAUID
set ORA_PASSWD = $ORAPWD

#---   ＳＱＬ実行
source $SUBSRCDIR/sqldload3.src
if ( $status != $NORMAL ) then
    sync    #メモリとＤＩＳＫの同期化実施
    echo "$SHELLNAME:t :  ABEND time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
   	exit $ABEND
endif

#------------------------------------------------------------------------#
#     ３．ＡＮＡＬＹＺＥ
#------------------------------------------------------------------------#
set SQL_PARM=(-s $ORAUID/$ORAPWD @${SQLDIR}/hnm001_alz_tbl_$ORAUID.sql)

#---   ＳＱＬ実行
source $SUBSRCDIR/execsql.src
if ( $status != $NORMAL ) then
    sync    #メモリとＤＩＳＫの同期化実施
    echo "$SHELLNAME:t :  ABEND time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
   	exit $ABEND
endif

#------------------------------------------------------------------------#
#     ４．ワークファイル削除
#------------------------------------------------------------------------#
/bin/rm -f $WKDIR/hnw401p_001.tsv

#------------------------------------------------------------------------#
#      正常終了
#------------------------------------------------------------------------#
echo "$SHELLNAME:t :  end  time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
exit $NORMAL

