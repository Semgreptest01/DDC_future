#!/bin/csh
#------------------------------------------------------------------------#
# JOB NAME      : shnad0000
# SHELL NAME    : shnad0000.csh
# 機能          : ＮＥＴ開始処理　ＤＵＭＭＹ
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
# 作成者        : G.Saito
# 作成日        : 2009/09/28
# 修正履歴      :
# NO  修正日付     修正者      修正内容
#  1  2022/05/09   N.terasawa  検証ＩＦ受信処理組込
#  2  2025/12/12   R.someya    DUMMY化
#------------------------------------------------------------------------#
# 初期処理部
#------------------------------------------------------------------------#
#(1)シェル共通部品が存在するディレクトリ名をフルパスでセットする。
#set SUBSRCDIR = "/wk_apl/apl/prod/jcl/sub"
#LMDC移行に伴い、ディレクトリ名をフルパスから変数化
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
echo "dummy" | tee -a $LOG_FILE
#------------------------------------------------------------------------#
#   DATE出力
#------------------------------------------------------------------------#
echo "$SHELLNAME:t : end  time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
exit $NORMAL
