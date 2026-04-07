#!/bin/csh
#------------------------------------------------------------------------#
# ネット-名     : HNM_セキュリティ取引相手先受信ファイル取出し＆ロード
# JOB-名        : HNM_セキュリティ取引相手先load
# Shell-名      : shnad1121.csh
# 機能          : １．ファイル存在チェック
#               : ２．ＳＱＬ ＬＯＡＤ
#               : ３．ＡＮＡＬＹＺＥ
# 作成者        : M.Yamamoto
# 作成日        : 2025/12/02
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
#     １．ファイル存在チェック
#------------------------------------------------------------------------#
set CHECKFILE=$WKDIR/hnt402p_001.dat
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL )  then
    sync    #メモリとＤＩＳＫの同期化実施
    echo "$SHELLNAME:t :  ABEND time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
    exit $ABEND
endif
#------------------------------------------------------------------------#
#     ２．ＳＱＬ ＬＯＡＤ
#------------------------------------------------------------------------#
set SQL_NAME=sqlplus
set CTL_FILE="${CTLDIR}/hnm002.ctl"
set DAT_FILE=$WKDIR/hnt402p_001.dat

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
set SQL_PARM=(-s $ORAUID/$ORAPWD @${SQLDIR}/hnm002_alz_tbl_$ORAUID.sql)

#---   ＳＱＬ実行
source $SUBSRCDIR/execsql.src
if ( $status != $NORMAL ) then
    sync    #メモリとＤＩＳＫの同期化実施
    echo "$SHELLNAME:t :  ABEND time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
   	exit $ABEND
endif

#------------------------------------------------------------------------#
#      正常終了
#------------------------------------------------------------------------#
echo "$SHELLNAME:t :  end  time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
exit $NORMAL

