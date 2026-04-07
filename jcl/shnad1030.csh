#!/bin/csh
#------------------------------------------------------------------------#
# JOB NAME      : hnad1030
# SHELL NAME    : shnad1030.csh
# 機能          : 在庫切れ速報更新通知(12時締め)
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
# 作成者        : H.Nagata
# 作成日        : 2010/03/04
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
#   PLSQL ( hnadpl460.sql ) 在庫切れ速報更新通知１２時締め
#------------------------------------------------------------------------#
set JN_SQL = $PLSQLDIR/hnadpl460.sql
set SQL_NAME = sqlplus
set SQL_PARM = "-s $ORAUID/$ORAPWD @$JN_SQL"
set CHECKFILE=$JN_SQL
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
source $SUBSRCDIR/execsql2.src
if ( $status != $NORMAL ) then
        exit $ABEND
endif
#------------------------------------------------------------------------#
#   DATE出力
#------------------------------------------------------------------------#
echo "$SHELLNAME:t : end  time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
exit $NORMAL
