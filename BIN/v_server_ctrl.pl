#!/usr/bin/perl
################################################################################
# 
# システム名：横断集計
# 
# プログラム名：v_server_ctrl.pl
#
# 機能概要：
#       コントロールファイルから抽出条件に該当するレコードを出力する。
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

#die("作成中....");

unless ($ENV{'SERVER_CTRL_FILE'} and $ENV{'PROD_DIR'}) {
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

  my $ht=""; #ホスト名
  my $jb=""; #ジョブ名
  my $fl=""; #正副フラグ
  my $ct=""; #稼動判定
  my @pr=(); #表示項目

  my @hi=@ARGV; #引数のバックアップ
#(getOptを使うとオプションにしていしたものが出力されないため)

#引数が3以外ならエラー
#GetOptions(
#    "print=s" => \@pr,
#    "host:s"  => \$ht,
#    "job:s"   => \$jb,
#    "msflg:s" => \$fl, 
#    "ctrl:s"  => \$ct,
#) or die("引数の指定が正しくありません。引数=<@hi>\n");

  my $md="";
  my $hp=0;
  my $jp=0;
  my $fp=0;
  my $cp=0;
  my $ap=0;

  foreach my $hk (@hi) {
    if ($hk eq "-print") {
      $md = "p";
    } elsif ($hk eq "-host") {
      $md = "h";
      $hp = 1; #オプション指定
    } elsif ($hk eq "-job") {
      $md = "j";
      $jp = 1; #オプション指定
    } elsif ($hk eq "-msflg") {
      $md = "f";
      $fp = 1; #オプション指定
    } elsif ($hk eq "-ctrl") {
      $md = "c";
      $cp = 1; #オプション指定
    } else {
      if (0 == index($hk, "-")) {
        $ap = 1; #指定外オプション
      } elsif ($md eq "p") {
        $pr[@pr] = $hk;
      } elsif ($md eq "h") {
        $ht = $hk;
      } elsif ($md eq "j") {
        $jb = $hk;
      } elsif ($md eq "f") {
        $fl = $hk;
      } elsif ($md eq "c") {
        $ct = $hk;
      }
    } 
  
  }

  #表示項目が指定されていない場合エラー
  if (@pr == 0) {
    print STDERR "出力項目を指定して下さい。\n";
    $ret = 1; #エラーとする
  } 
  if ($ap == 1) {
    print STDERR "抽出条件がありません。引数=<@hi>\n";
    $ret = 1; #エラーとする
  } elsif ($hp == 1 and $ht eq "") {
    print STDERR "抽出条件がありません。引数=<@hi>\n";
    $ret = 1; #エラーとする
  } elsif ($jp == 1 and $jb eq "") {
    print STDERR "抽出条件がありません。引数=<@hi>\n";
    $ret = 1; #エラーとする
  } elsif ($fp == 1 and $fl eq "") {
    print STDERR "抽出条件がありません。引数=<@hi>\n";
    $ret = 1; #エラーとする
  } elsif ($cp == 1 and $ct eq "") {
    print STDERR "抽出条件がありません。引数=<@hi>\n";
    $ret = 1; #エラーとする
  } else {

  #print STDOUT "print=<@pr>\n";
  #print STDOUT "host=<$ht>\n";
  #print STDOUT "job=<$jb>\n";
  #print STDOUT "msflg=<$fl>\n";
  #print STDOUT "ctrl=<$ct>\n";

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
  #my $ct=0; #現在の検証レコード数
  #my $mx=0; #grep最大数
  #my @tm; #時間

  #my $pt="$ENV{'PROD_DIR'}/PRM/SERVER_CTRL_FILE";
  #my $pt="$ENV{'PROD_DIR'}/TBL/MSCTRLTABLE";
  my $pt="$ENV{'SERVER_CTRL_FILE'}";
  #print STDOUT "path=$pt\n";
    if ($ret != 1) {    
      #外部ファイルの存在チェック
      if (-f $pt) {
        #配列にセット
        if (open (my $fh, '<', $pt)) {
          @sh = <$fh>;
          close($fh);
          #条件が未指定の場合、任意の文字列とする
          if ($ht eq "") {
            $ht = "\\w\+"; #任意の文字列
          }
          if ($jb eq "") {
            $jb = "\\w\+"; #任意の文字列
          }
          if ($fl eq "") {
            $fl = "."; #任意の1文字
          }
          if ($ct eq "") {
            $ct = "."; #任意の1文字
          }
          #ファイルをgrep
          @tg = grep(/$ht\s$jb\s$fl\s$ct/, @sh);

          #print STDOUT "tg=@tg\n";
        } else {
          print STDERR "SERVER_CTRL_FILEを開けません。\n";
          $ret = 1; #エラーとする
        }
    
        #grep結果がある場合、チェック開始
        foreach my $list (@tg) {
          #$ct = $ct + 1; #カウントアップ
          my @prm = split(/\s+/, $list); #パラメータの分割(余分な空白を削除)
          #print STDOUT "prm=<@prm>\n";
          my $ot=""; #出力用
          my $sp=''; #セパレート
          foreach my $ky (@pr) {
            if ($ky eq "host") {
              $ot = $ot . $sp . $prm[0];
            }
            if ($ky eq "job") {
              $ot = $ot . $sp . $prm[1];
            }
            if ($ky eq "msflg") {
              $ot = $ot . $sp . $prm[2];
            }
            if ($ky eq "ctrl") {
              $ot = $ot . $sp . $prm[3];
            }
            $sp = ' '; #2項目目以降は、スペースを間に入れる。
          }
          print STDOUT "$ot\n";
        }
      } else {
        print STDERR "SERVER_CTRL_FILEが存在しません。\n";
        $ret = 1; #エラーとする
      }
    }
  }
}

exit ($ret);
