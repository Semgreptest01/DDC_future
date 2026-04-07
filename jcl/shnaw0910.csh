#!/bin/csh
#------------------------------------------------------------------------#
# JOB NAME      : hnaw0910
# SHELL NAME    : shnaw0910.csh
# 機能          : 週次パラメータバックアップ
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
# 作成者        : G.Sasaki
# 作成日        : 2010/03/23
# 修正履歴      :
# NO  修正日付     修正者      修正内容
#  1  2025/11/18   K.Takashima LMDC移行対応：オリジナルexportからデータポンプexportへ変更
#------------------------------------------------------------------------#
# 初期処理部
#------------------------------------------------------------------------#
#(1)シェル共通部品が存在するディレクトリ名をフルパスでセットする。
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
#   週次バックアップ対象テーブル指定
#------------------------------------------------------------------------#
#LMDC移行#set LIST = (\
#LMDC移行#hnt033\
#LMDC移行#hnt037\
#LMDC移行#)
#LMDC移行##------------------------------------------------------------------------#
#LMDC移行##   EXPORT ( 週次パラメータバックアップ )
#LMDC移行##------------------------------------------------------------------------#
#LMDC移行#foreach LINE ( $LIST )
#LMDC移行#    set DMP_FILE   = "$BKUPDIR/${LINE}_exp_tbl_${ORAUID}.dmp"
#LMDC移行#    set EXPORT_SQL = "$EXPORT_DIR/${LINE}_exp_tbl_${ORAUID}.ctl"
#LMDC移行##
#LMDC移行#    rm  "$DMP_FILE"
#LMDC移行##
#LMDC移行#    set CHECKFILE = $EXPORT_SQL
#LMDC移行#    source $SUBSRCDIR/filechk.src
#LMDC移行#    if ( $status != $NORMAL )  then
#LMDC移行#        exit $ABEND
#LMDC移行#    endif
#LMDC移行##
#LMDC移行#    source $SUBSRCDIR/export2.src
#LMDC移行#    if ( $status != $NORMAL ) then
#LMDC移行#        sync    #メモリとＤＩＳＫの同期化実施
#LMDC移行#        echo "$SHELLNAME:t :  ABEND time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
#LMDC移行#        exit $ABEND
#LMDC移行#    endif
#------------------------------------------------------------------------#
#       対象ＴＢＬリスト作成
#------------------------------------------------------------------------#
set TMPFILE="/tmp/`basename $0`-$$.txt"
sed 's/^ *#.*$//g' << EOF | egrep -v '^\s*$' >  $TMPFILE
#
#cshのforeachは、空白を行区切り文字とする(項目区切りとしない)為使用不可。コメントなら使用可能。
#【書き方】
#xxxnnn:テーブル名 #コメント
#
hnt033,HNT_計画終了対象商品＿週次
hnt037,HNT_計画終了対象商品＿週次帳票
EOF
#------------------------------------------------------------------------#
#       データポンプexport実行
#------------------------------------------------------------------------#
set DMP_DIROBJ=${DBOBJDMP}
set DMP_DIROBJPATH=${DBDMPDIR}
set DMP_FILEPATH=${BKUPDIR}

foreach line(`cat $TMPFILE`)
    set id=`echo ${line} | awk -F, '{print $1}'`

    set DMP_TBL=`echo ${line} | awk -F, '{print $2}'`
    set DMP_FILE="${id}_exp_tbl_${ORAUID}.dmp"

    echo "***" >> $LOG_FILE
    echo "*** ID=${id} TBL=${DMP_TBL} DMP=${DMP_FILEPATH}/${DMP_FILE}" | tee -a $LOG_FILE
    echo "***" >> $LOG_FILE
    source $SUBSRCDIR/exportdp.src
    if ( $status != $NORMAL ) then
        echo "$SHELLNAME:t : exportdp.src ABEND time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
        exit $ABEND
    endif
#
   chmod 666 "$BKUPDIR/$DMP_FILE".gz
#
end
#------------------------------------------------------------------------#
#       対象ＴＢＬリスト削除
#------------------------------------------------------------------------#
/bin/rm $TMPFILE
#------------------------------------------------------------------------#
#   DATE出力
#------------------------------------------------------------------------#
echo "$SHELLNAME:t : end  time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
exit $NORMAL
