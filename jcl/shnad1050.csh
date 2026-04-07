#!/bin/csh
#------------------------------------------------------------------------#
# JOB NAME      : hnad1050
# SHELL NAME    : shnad1050.csh
# 機能          : 受発注在庫切れ情報ロード（22時締め）
#                 
# 必須条件      : SHELLNAME  起動パラメータ
#               : ORAUID     起動パラメータ
#               : ORAPWD     起動パラメータ
#               : SUBSRCDIR  シェル部品パス
#               : NORMAL     common.srcで定義
#               : ABEND      common.srcで定義
#               : LOG_FILE   common.srcで定義
# 備考          : 
# リランジョブ  : 
# 作成者        : VIXUS)G.Sasaki
# 作成日        : 2010/03/23
# 修正履歴      : 
# NO  修正日付     修正者      修正内容
#  1  2025/12/16   K.Takashima 共通ソースパスを変更
#------------------------------------------------------------------------#
# 初期処理部
#------------------------------------------------------------------------#
#(1)シェル共通部品が存在するディレクトリ名をフルパスでセットする。
#set SUBSRCDIR = "/wk_apl/apl/prod/jcl/sub"
#LMDC移行に伴い、ディレクトリを変更
set SUBSRCDIR = "/prod/jcl/sub"
#(2)シェル変数定義モジュールを実行する。
source $SUBSRCDIR/hnad.src
#(3)シェル共通処理を実行する。
source $SUBSRCDIR/common.src
#(4)判定処理
if ( $status != $NORMAL )  then
    echo "$SHELLNAME:t : common.src = end ERR" | tee -a $LOG_FILE
    exit $ABEND
endif
#------------------------------------------------------------------------#
# ロードファイル存在チェック処理
#------------------------------------------------------------------------#
set DAT_FILE = "$WKDIR/hnt081p_002.dat"
set CHECKFILE=$DAT_FILE
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL ) then
        exit $ABEND
endif
#------------------------------------------------------------------------#
# DROP PRIMARY KEY HNT_受発注在庫切れ情報
#------------------------------------------------------------------------#
set DDL_FILE    = $SQLDIR/hnt035_pk_drp_con_${ORAUID}.sql
set SQL_NAME    = sqlplus
set SQL_PARM    = (-s $ORAUID/$ORAPWD @$DDL_FILE)
set CHECKFILE   = $DDL_FILE
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
source $SUBSRCDIR/execsql2.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
#------------------------------------------------------------------------#
# TRUNCATE HNT_受発注在庫切れ情報
#------------------------------------------------------------------------#
set DDL_FILE    = $SQLDIR/hnt035_trc_tbl_${ORAUID}.sql
set SQL_NAME    = sqlplus
set SQL_PARM    = (-s $ORAUID/$ORAPWD @$DDL_FILE)
set CHECKFILE   = $DDL_FILE
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
source $SUBSRCDIR/execsql2.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
#------------------------------------------------------------------------#
# ロード用変数定義
#------------------------------------------------------------------------#
set USER_ID = $ORAUID
set ORA_PASSWD = $ORAPWD
#------------------------------------------------------------------------#
# SQL*Loader HNT_受発注在庫切れ情報
#------------------------------------------------------------------------#
set CTL_FILE = "$CTLDIR/hnt035.ctl"
set CHECKFILE=$CTL_FILE
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL ) then
        exit $ABEND
endif
source $SUBSRCDIR/sqldload3.src
if ( $status != $NORMAL ) then
        exit $ABEND
endif
#------------------------------------------------------------------------#
# ADD PRIMARY KEY HNT_受発注在庫切れ情報
#------------------------------------------------------------------------#
set DDL_FILE    = $SQLDIR/hnt035_pk_add_con_${ORAUID}.sql
set SQL_NAME    = sqlplus
set SQL_PARM    = (-s $ORAUID/$ORAPWD @$DDL_FILE)
set CHECKFILE   = $DDL_FILE
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
source $SUBSRCDIR/execsql2.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
#------------------------------------------------------------------------#
# ANALYZE HNT_受発注在庫切れ情報
#------------------------------------------------------------------------#
set DDL_FILE    = $SQLDIR/hnt035_alz_tbl_${ORAUID}.sql
set SQL_NAME    = sqlplus
set SQL_PARM    = (-s $ORAUID/$ORAPWD @$DDL_FILE)
set CHECKFILE   = $DDL_FILE
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
source $SUBSRCDIR/execsql2.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
#
#------------------------------------------------------------------------#
#   DATE出力
#------------------------------------------------------------------------#
echo "$SHELLNAME:t : end  time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
exit $NORMAL
