#!/usr/bin/perl
################################################################################
# 
# システム名：横断集計
# 
# プログラム名：v_jobwait.pl
#
# 機能概要：
#       対象の処理の先行終了確認をする。
#
# 作成日：2014/12/09
#
# 作成者：Y.Hayashi
#
# 修正履歴：
#   yyyy/mm/dd K.Shimadaxxxxxx 
#     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
#     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
#
################################################################################
use strict;
use warnings;
use utf8;

#use open ":utf8";

use Data::Dumper; #  e.x. print STDERR Dumper $var;

use FindBin;
use lib "$FindBin::Bin/lib"; # スクリプト格納パス配下のlibをライブラリに追加
use lib "$FindBin::Bin/arch"; # スクリプト格納パス配下のarchをライブラリに追加


use Getopt::Long;
use Encode::Locale;
Encode::Locale::decode_argv; # 引数を decode 不要にする

binmode STDIN  => ":encoding(console_in)";  # 標準入力を decode 不要にする
binmode STDOUT => ":encoding(console_out)"; # 標準出力を encode 不要にする
binmode STDERR => ":encoding(console_out)"; # 標準エラー出力を encode 不要にする


#use Text::CSV_XS qw( csv );

#print STDERR "[INFO]$0 start\n";

#my $PROD_DIR = "/home/lawson";
#my $JOB_LIMIT_TIME_FILE = "$PROD_DIR/TBL/TIME_LIMIT_TBL.20141024";

my $ret=0;

unless ($ENV{'JOB_LIMIT_TIME_FILE'} and $ENV{'PROD_DIR'}) {
  print STDERR "環境変数が未定義です。\n";
  $ret = 1; #エラーとする
  
} else {

#----------------------------------------------------------------------
# 引数の解析
#----------------------------------------------------------------------
#my $file = '-';
#my $delim = ',';
#my $tabsplit=0;

#my $in_cnt=0;
#my $ot_cnt=0;

#GetOptions(
#    'tab'    => \$tabsplit,
#) or die("[ERROR]missing parameter");

#$delim = "\t" if $tabsplit;

#die("[ERROR]Missing to many file parameter.\n") if @ARGV > 1 ;
#$file = $ARGV[0]                             if @ARGV == 1;

#引数が3以外ならエラー
if ($#ARGV != 2) {
  print STDERR "[ERROR]引数の指定が正しくありません。 引数=<@ARGV>\n";
  $ret = 1; #エラーとする
} else {

#  print STDOUT "test中...\n";
#  print STDOUT "param=$#ARGV\n";

#  print STDOUT "path=$JOB_LIMIT_TIME_FILE\n";
#----------------------------------------------------------------------
# main 
#----------------------------------------------------------------------
#open ( my $fh, '<'.$file)  or die("[ERROR]can't open file.($file)\n",);
#binmode $fh  => ":encoding(console_in)";
#
#my $csv = Text::CSV_XS->new ({ binary => 1 , 'sep_char' => $delim });
#
#while ( my $row = $csv->getline($fh)){ 
#      $in_cnt++;
#      foreach my $var (@$row){
#        $var =~ s/^ *$/_/g;
#        $var =~ s/ /_/g;
#      }
#      print join(" ",@$row)."\n";
#      $ot_cnt++;
#}
#
#close $fh;

#print STDERR "input=$in_cnt output=$ot_cnt\n";
#print STDERR "[INFO]$0 normal end\n";
  my @tg=(); #grep結果
  my @sh; #テキストファイル内容
  my $ct=0; #現在の検証レコード数
  my $mx=0; #grep最大数
  my @tm; #時間

  #外部ファイルの存在チェック
  if (-f $ENV{'JOB_LIMIT_TIME_FILE'}) {
    #配列にセット
    if (open (my $fh, '<', $ENV{'JOB_LIMIT_TIME_FILE'})) {
      @sh = <$fh>;
      close($fh);
      #ファイルをgrep
      @tg = grep(/^$ARGV[0]\s/, @sh);
      $mx = @tg;
    } else {
      print STDERR "JOB_LIMIT_TIME_TBLを開けません。\n";
      $ret = 1; #エラーとする
    }
    
    #grep結果がある場合、チェック開始
    foreach my $list (@tg) {
      $ct = $ct + 1; #カウントアップ
      my @prm = split(/\s+/, $list); #パラメータの分割(余分な空白を削除)
      my $fst = 0; #初回フラグ
      #無限ループ(ファイルがあったら抜ける)
      while(1) {
        @tm = localtime; #現在時刻取得
        #ファイル<ファイル名>＋引数2<ホスト名>＋引数3<日付>
        my $fn = "$prm[2].$ARGV[1].END.$ARGV[2]";
        $fn =~ s/\r//; #改行コードの削除
        $fn =~ s/\n//; #改行コードの削除
        my $fn2 = "$prm[2].$ARGV[1].WAIT.$ARGV[2]";
        $fn2 =~ s/\r//; #改行コードの削除
        $fn2 =~ s/\n//; #改行コードの削除

        #print STDOUT "path=$fn\n";
        #if (-f $ENV{'PROD_DIR'}/job_cntl/$fn) {
        if (-f "$ENV{'PROD_DIR'}/JOB_CTRL/$fn") {
          print STDERR "先行JOB終了 $prm[0] 先行数=$ct/$mx $fn\n";
          last; #終了ファイルを見つけたので次へ
        }
        #現在時刻(時分)
        my $nw = sprintf '%02d%02d', $tm[2], $tm[1];
        #print STDOUT "time=$nw";
        #ファイル<制限時間>＜＝現在時刻
        if ($prm[1] <= $nw) {
          #時間超過
          print STDERR "先行JOB_LIMIT_ERR $prm[0] 先行数=$ct/$mx LIMIT$prm[1] $fn2\n";
          $ret = 1; #エラーとする
          last; #タイムリミットなのでエラーを出力して次へ
        } else {
          #初回のみログ出力
          if ($fst == 0) {
            print STDERR "先行JOBwait $prm[0] 先行数=$ct/$mx 開始$nw $fn2\n";
            $fst = 1; #初回フラグON
          }
          sleep 1; #スリープ処理
        }
      }
    }
  } else {
    print STDERR "JOB_LIMIT_TIME_TBLが存在しません。\n";
    $ret = 1; #エラーとする
  }
}
}

exit ($ret);
