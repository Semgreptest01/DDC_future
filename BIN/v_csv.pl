#!/usr/bin/perl
################################################################################
# 
# システム名：横断集計
# 
# プログラム名：v_csv.pl
#
# 機能概要：
#       ＣＳＶファイルを、空白区切りファイルへ変換し出力する。
#       尚、項目データ内の空白はアンダースコア(_)に置換する。
#
# 作成日：2014/11/18
#
# 作成者：K.Shimada
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


use Text::CSV_XS qw( csv );

print STDERR "[INFO]$0 start\n";

#----------------------------------------------------------------------
# 引数の解析
#----------------------------------------------------------------------
my $file = '-';
my $delim = ',';
my $tabsplit=0;

my $in_cnt=0;
my $ot_cnt=0;

GetOptions(
    'tab'    => \$tabsplit,
) or die("[ERROR]missing parameter");

$delim = "\t" if $tabsplit;

die("[ERROR]Missing to many file parameter.\n") if @ARGV > 1 ;
$file = $ARGV[0]                             if @ARGV == 1;

#----------------------------------------------------------------------
# main 
#----------------------------------------------------------------------
open ( my $fh, '<'.$file)  or die("[ERROR]can't open file.($file)\n",);
binmode $fh  => ":encoding(console_in)";

my $csv = Text::CSV_XS->new ({ binary => 1 , 'sep_char' => $delim });
my $prev=0;
my $current=0;

while ( my $row = $csv->getline($fh)){ 
      $in_cnt++;
      $current = scalar(@$row);
   
      # 直前行とカラム数のつき合わせチェック
      if ($in_cnt==1){
          $prev = $current;
      }else{
          if ($prev!=$current){
              die("[ERROR]Number of columns bean unmatch.prev=$prev current=$current LINE=$in_cnt\n");
          }
          $prev = $current;
      }
      
      foreach my $var (@$row){
        $var =~ s/^ *$/_/g;  #空文字列は１文字空白
        $var =~ s/ /_/g;     #項目中空白は文字数分空白
      }
      print join(" ",@$row)."\n";
      $ot_cnt++;
}

close $fh;

print STDERR "input=$in_cnt output=$ot_cnt\n";
print STDERR "[INFO]$0 normal end\n";
exit
