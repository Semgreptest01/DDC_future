#!/usr/bin/perl
################################################################################
#
# システム名：横断集計
#
# プログラム名：v_seqrank3.pl
#
# 機能概要：
#       指定した入力ファイルの先頭３項目までのそれぞれのレコード順位を、
#       先頭項目の前に付与して標準出力に書きだす。
#       出力イメージ：第一項目順位 第二項目順位 第一項目順位 入力ファイルそのまま。
#
# 作成日：2014/11/27
#
# 作成者：K.Takagi
#
# 修正履歴：
#   yyyy/mm/dd K.Takagixxxxxx
#     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
#     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
#
################################################################################
use strict;
#use warnings;

#binmode STDIN, ':encoding(cp932)';
#binmode STDOUT, ':encoding(cp932)';
#binmode STDERR, ':encoding(cp932)';



my @strlin = {};
my $rank1=0;
my $rank2=0;
my $rank3=0;
my $chg1=0;
my $chg2=0;
my $chg3=0;
my @prestr={};
my @preprestr={};
local *FILE;

my $linecnt=0;
my $orgline=0;
my $opn=0;
my $inf_fil = $ARGV[0];
if(@ARGV >0) {
	if($inf_fil eq "-") {
		$opn=0
	}
	else {
		if (!(-f $inf_fil)){
			print "入力ファイルが見つかりません\n";
			die "file not found\n";
		}
		else {
			open(DATAFILE, "< $inf_fil") or die("Error:$!");
			$opn=1;
		}
	}
}
else {
	$opn = 0;
	#DATAFILE = <STDIN>;
}


#open(DATAFILE, "< $inf_fil") or die("Error:$!");
#open(DATAFILE, "< inf.txt") or die("Error:$!");


if($opn==1) {
	(*FILE) = (*DATAFILE);
}
else {
	(*FILE) = (*STDIN);
}
#local (*FILE) = (*DATAFILE);
# (*FILE) = (*DATAFILE);
#sub seqrank{
	#local(*FILE) = @_;
while(my $line = <FILE>) {
	chomp($line);
	$orgline = $line;
	$line=~s/ +/ /g;
	$line=~s/^ //;
	my @strlin = split(/ /,$line);

#	foreach my $aaa (@strlin) {
#		print "f=$aaa\n";



	if($linecnt == 0) {
 		$rank1=1;
 		$rank2=1;
 		$rank3=1;
 		$chg1=0;
 		$chg2=0;
 		$chg3=0;
	}
	else {
	if($prestr[0] eq $strlin[0]) {
 			$chg1=0;
 
	}
	else {
		#if($chg1==0) {
 			$chg1=1;
 		#}
			$rank1++;
	}
	
	if($chg1>0) {		#第一項目がブレイクしたら１からスタート
		$rank2=1;
		$chg2=2;
	}
	#第一項目と第二項目が前行と同一の場合同じ順位を付与する。
	elsif($prestr[0] eq $strlin[0] && $strlin[1] eq $prestr[1]) {
		#$rank2 = $rank1;
		$chg2=1;
	}
	#第二項目前行とブレイクしたら順位をプラスする。
	elsif($prestr[1] ne $strlin[1]) {
 		$rank2++;
		$chg2=3;
	}
	#	第一項目もしくは第二項目がブレークしたら1からスタート
	if($chg1>0 || $chg2>1) {
		$rank3 = 1;
	}
	if($prestr[0] eq $strlin[0] && $strlin[1] eq $prestr[1]) {
		$rank3++;
	}
	else {
		$rank3=1;
	}
	
	}

	$linecnt++;
#	open(OUTFILE, ">> out.txt") or die("error $!");
#	print OUTFILE "$rank1 $rank2 $rank3 ";
#	print OUTFILE $orgline."\n";
	print STDOUT "$rank1 $rank2 $rank3 ";
	print STDOUT $orgline."\n";

#	 @preprestr = @prestr;
	 @prestr = @strlin;

}
#}
