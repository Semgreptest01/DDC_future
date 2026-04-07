#!/bin/csh
#------------------------------------------------------------------------#
# JOB NAME      : hnad1040
# SHELL NAME    : shnad1040.csh
# 機能          : 在庫切れ速報受信（22時締め）
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
#  1  2025/11/17   R.SOMEYA    LMDC移行対応：ftpass=>IFP
#  2  2025/12/16   K.Takashima 共通ソースパスを変更
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
#   ファイル削除処理
#------------------------------------------------------------------------#
rm -f $WKDIR/hnt081p_002.dat
#
#------------------------------------------------------------------------#
#   ファイル受信
#------------------------------------------------------------------------#
#20251117 LMDC移行
#set F_FILE="$FTPASSTYPE.WJT081P.002"
#set D_FILE="$WKDIR/hnt081p_002.dat"
#set N_NO="1"
##
#source $SUBSRCDIR/ftpget.src
#if ( $status != $NORMAL ) then
#    exit $ABEND
#endif
#
#------------------------------------------------------------------------#
#      IFP集信ファイル取出し部
#------------------------------------------------------------------------#
set BAKRECV_DIR = $BKUPDIR # 集信ファイルのバックアップ先
set F_FILE    =  ${IFTYPE}HN-AD00-WJT081P002    # 論理ファイル名
set D_FILE    =  $WKDIR/hnt081p_002.dat          # 取出し後ファイル名
#
set N_NO = 1 # 取出し件数　※省略時は1
set P_PROCEDURE = 1 # 処理区分　※省略時は1
                    #   1:ファイルがN_NO数到着する迄待ち、監視打ち切りは行わない
                    #   3:ファイルがN_NO数到着する迄待つが、監視回数で打切る。その時取出しファイルゼロバイト出力。
#set E_ERASE = -e # 取出し後ファイルの事前削除指定　※省略時は削除しない
#set SERCH_CNT = 10 # 監視回数　※処理区分=3の時必須指定
#set SERCH_INTERVAL = 60 # 監視間隔（秒）　※省略時は180秒
#
source $SUBSRCDIR/ifpRecv.src
if ( $status != $NORMAL )  then
    echo "$SHELLNAME:t :  abnormal end time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
    exit $ABEND
endif
#------------------------------------------------------------------------#
#   ファイル権限変更
#------------------------------------------------------------------------#
chmod 666 $WKDIR/hnt081p_002.dat
#
#------------------------------------------------------------------------#
#   DATE出力
#------------------------------------------------------------------------#
echo "$SHELLNAME:t : end  time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
exit $NORMAL
