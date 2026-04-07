#!/bin/csh
#------------------------------------------------------------------------#
# JOB NAME      : JHNAD01JME40
# SHELL NAME    : shnad0140.csh
# 機能          : アドレスマスタ不要データ削除
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
# 作成者        : S.Oyama
# 作成日        : 2013/07/30
# 修正履歴      :
# NO  修正日付     修正者      修正内容
#  1  2025/11/14   K.Takashima LMDC移行対応：オリジナルexportからデータポンプexportへ変更
#  2  2025/12/12   R.Someya    共通ソースパスを変更
#  3  2025/12/25   M.Yamamoto  テーブル名の文字化け対応
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
#LMDC移行##------------------------------------------------------------------------#
#LMDC移行##   EXPORT ( HNT_アドレスマスタバックアップ )
#LMDC移行##------------------------------------------------------------------------#
#LMDC移行#set DAY_NO    = `date +%d`
#LMDC移行#set DMP_FILE   = "$BKUPDIR/hnt038_exp_tbl_${DAY_NO}.dmp"
#LMDC移行#set EXPORT_SQL = "$EXPORT_DIR/hnt038_exp_tbl_${ORAUID}.ctl"
#LMDC移行##
#LMDC移行#rm  $DMP_FILE
#LMDC移行##
#LMDC移行#set CHECKFILE = $EXPORT_SQL
#LMDC移行#source $SUBSRCDIR/filechk.src
#LMDC移行#if ( $status != $NORMAL )  then
#LMDC移行#    exit $ABEND
#LMDC移行#endif
#LMDC移行##
#LMDC移行#source $SUBSRCDIR/export2.src
#LMDC移行#if ( $status != $NORMAL ) then
#LMDC移行#    sync    #メモリとＤＩＳＫの同期化実施
#LMDC移行#    echo "$SHELLNAME:t :  ABEND time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
#LMDC移行#    exit $ABEND
#LMDC移行#endif
#LMDC移行追加分開始#
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
hnt038,HNT_アドレスマスタ
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
#LMDC移行追加分終了#
#LMDC移行##------------------------------------------------------------------------#
#LMDC移行##   ファイル権限変更
#LMDC移行##------------------------------------------------------------------------#
#LMDC移行#chmod 666 $DMP_FILE
#------------------------------------------------------------------------#
#   PLSQL ( hnadpl550.sql ) アドレスマスタ不要データ削除
#------------------------------------------------------------------------#
set JN_SQL = $PLSQLDIR/hnadpl550.sql
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
#
#------------------------------------------------------------------------#
# ANALYZE TABLE (HNT038) HNT_アドレスマスタ
#------------------------------------------------------------------------#
set SQL_PGM  = "$PLSQLDIR/hnadpl560.sql"
#LMDC移行# Start 2025.12.25
#LMDC移行#set TAB_NAME = "HNT_アドレスマスタ"
set TAB_NAME = `echo "HNT_アドレスマスタ" | iconv -f utf8 -t sjis`
#LMDC移行# End 2025.12.25
set SQL_PARM = "-s $ORAUID/$ORAPWD @$SQL_PGM $ORAUID $TAB_NAME 5"
set CHECKFILE=$SQL_PGM
source $SUBSRCDIR/filechk.src
if ( $status != $NORMAL )  then
        exit $ABEND
endif
source $SUBSRCDIR/execsql.src
if ( $status != $NORMAL ) then
        exit $ABEND
endif
echo "$SHELLNAME:t : end  time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
exit $NORMAL
#------------------------------------------------------------------------#
#   DATE出力
#------------------------------------------------------------------------#
echo "$SHELLNAME:t : end  time : `date +%y/%m/%d:%H:%M:%S`" | tee -a $LOG_FILE
exit $NORMAL
