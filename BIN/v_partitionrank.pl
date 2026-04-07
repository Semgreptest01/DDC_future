#!/usr/bin/perl
################################################################################
#
# システム名：横断集計
#
# プログラム名：v_partitionrank.pl
#
# 機能概要：
#       指定した入力ファイルのレコード順位を、先頭項目の前に付与して標準出力に書きだす。
#       出力イメージ：レコード順位 入力ファイルそのまま。
#       レコード順位は、区画ごとに順位付けし、KEYが同一の場合は同一レコード順を出力する。
# 作成日：2014/12/04
#
# 作成者：N.Takagi
#
# 修正履歴：
#   yyyy/mm/dd N.Takagixxxxxx
#     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
#     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
#
################################################################################
use strict;
use File::Basename;
#use warnings;

#binmode STDIN, ':encoding(cp932)';
#binmode STDOUT, ':encoding(cp932)';
#binmode STDERR, ':encoding(cp932)';



my @strlin = {};
my $rank1=0;
my $chg1=0;
my @prestr={};
my @preprestr={};
local *FILE;

my $linecnt=0;
my $orgline=0;
my $opn=0;
my $inf_fil;
my $ref=0;
my $key=0;
my $ref_flg=0;
my $key_flg=0;
my $inf_fil;
my $refitem;
my $keyitem;
my $grobcnt=0;
my @refs;
my $strno;
my $endno;
my $strref;
my $prestrref;
my $myname = basename($0);
my $argvcnt = @ARGV;
if($argvcnt >= 2){
	 $inf_fil = $ARGV[0];
	if($inf_fil=~/^ref=/) {
		$ref = $inf_fil;
		$ref=~s/ref=//;
		$ref_flg=1;
		@refs= split(/-/,$ref);
		$strno = $refs[0];
		if(@refs >1) {
			$endno = $refs[1];
		}
		else {
			$endno = $strno;
		}
	}
	else {
		print "Error[$myname] : refフィールドが入力されていません。\n";
		exit(1)
	}

	 $inf_fil = $ARGV[1];
	if($inf_fil=~/^key=/) {
			$key =$inf_fil;
			$key=~s/key=//;
			$key_flg = 1;
	}
	else {
		print "Error[$myname] : keyフィールドが入力されていません\n";
		exit(1);
	}
	
	if($argvcnt >= 3) {
		$inf_fil = $ARGV[2];
		if($inf_fil eq "-") {
			$opn=0;
		}
		elsif (!(-f $inf_fil)){
			print "Error[$myname] : $inf_fil入力ファイルが見つかりません\n";
			exit(1);
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

$keyitem = $key - 1;

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
my $itemcnt =0;
while(my $line = <FILE>) {
	chomp($line);
	$orgline = $line;
	$line=~s/ +/ /g;
	$line=~s/^ //;
	my @strlin = split(/ /,$line);
	$itemcnt = @strlin;

	if($strno >$itemcnt || $endno >$itemcnt) {
		print STDERR "Error[$myname] :指定したrefフィールドが入力ファイルに存在しません。\n";
		exit(1);
	}
	elsif($key >$itemcnt) {
		print STDERR "Error[$myname] :指定したkeyフィールドが入力ファイルに存在しません。\n";
		exit(1);
	}
	$strref="";
	for(my $ii=$strno-1;$ii<$endno;$ii++) {
		$strref=$strref.$strlin[$ii]." ";
	}
	

	if($linecnt == 0) {
 		$rank1=1;
 		$chg1=0;
 		$grobcnt=1;
	}
	else {
	
#print "pre $prestrref ";
#print "now $strref\n";

		if($prestrref eq $strref) {
			$chg1=0;
			$grobcnt++;
		}
		else { #前行と異なる
			$rank1=1;
			$chg1=1;
 			$grobcnt=1;
		}
		
#print "keiitem$keyitem   prestr = $prestr[$keyitem] strlin= $strlin[$keyitem]\n";
		if($prestr[$keyitem] ne $strlin[$keyitem]) {
			$rank1 = $grobcnt;
		}
	}

	$linecnt++;
#	open(OUTFILE, ">> out.txt") or die("error $!");
#	print OUTFILE "$rank1 ";
#	print OUTFILE $orgline."\n";
	print STDOUT "$rank1 ";
	print STDOUT $orgline."\n";

	 @prestr = @strlin;
	 $prestrref = $strref;
}
