#!/usr/bin/perl
#------------------------------------------------------------------------#
# ProgramId     : lt_lex
# ProgramName   : lt_lex.pl
# Program Spec  : 横レイアウト変換（LawsonTool）
# Author        : Y.Takabayashi
# Create Date   : 2012/04/25
# Modify Date   :
# No.   ModifyDate  ModifyName Modify Information
#    1  yyyy/mm/dd  XXXXXXXXX  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
# -----------------------------------------------------------------------
# ＜仕様＞
# ・入力ファイルのKEYがブレークするまで、データ列を横レイアウトに変換する。
#   横繰り返し数は、引数指定するが、繰り返しの配置位置は、データ内の指定列の値を使用する。
# ・KEYの列指定（KEI指定）とデータ部の列指定（DAT指定）は、引数で指定する。また、横展開用の
# ・INDEX部（IDX指定）も列番号で指定する。このIDX指定は単独列指定です。
#   通常は、入力データ中の列番号（1～）で指定するが、ここに0を指定した場合は、
#   入力データ中にIDX値を持っていない場合、横展開をKEYブレークまでカウントアップ展開します。
# ・入力ファイルは、各列が一定の区切り文字(カンマ,半角スペース,またはTAB)で区切られていること。
# ・入出力ファイルの区切り指定は、引数で番号指定します。（以下の３種類）
#   （1:カンマ, 2:半角スペース, 3:TAB）
# ・KEY指定,DAT指定,IDX指定は、列番号（1始まり）で指定します。
# ・複数KEY、複数DAT指定は、1:2:3のように:区切りで１はじまりの列番号を指定します。
# ・有効少数桁数は、0～16の範囲で指定すること。
#   横展開時に同一KEYが重複した場合、たし込み処理になります。
#   その数値が少数値の場合、ここで指定した有効少数桁数で四捨五入します。
# ・指定されたKEYがブレークする毎にDAT指定列を横展開して出力する。
#
# ＜エラー処理について＞
# ・KEY部の属性チェックはしません。文字列扱いでKEYブレークの制御を行います。
# ・DAT指定の数値チェックを行います。
# ・IDX指定の数値チェックを行います。
#
# ＜ログ出力について＞
# ・ログ出力は、引数指定のログファイルにも出力されますが、標準出力にも全く同じ内容が出力されます。
# ・ログファイルは追加モードでオープンされますから、先行処理のログファイルを使用することも可能です。
# ・ログファイル名にdummyを指定すれば、ログファイルの出力はしません。
#------------------------------------------------------------------------#
use POSIX qw/ceil floor/;
$|=1;
$PGM_NM     = "lt_lex.pl";                               # Program Name
$HELP_TITLE = "＜lt_lex.pl＞ 横レイアウト変換（LawsonTool）";
$COM        = ",";                                       # カンマ区切り
$SPC        = " ";                                       # 半角スペース区切り
$TAB        = "\t";                                      # TAB区切り
$IFS        = "";                                        # Input Field Split Character
$OFS        = "";                                        # Output Field Split Character
#--------------------------------------------------------#
# 配列クリアー   0 ---> $LEX_ARRAY
#--------------------------------------------------------#
sub clear_LEX_ARRAY{
   for( $i=0;$i<$all_max;$i++){
      $LEX_ARRAY[$i] = 0;
   }
   return;
}
#--------------------------------------------------------#
# 配列へ配置 ---> $LEX_ARRAY[$y]
# すでにデータが展開されていたらたし込む。
#--------------------------------------------------------#
sub add_LEX_ARRAY{
   my $i  = shift;
   $y = $i * $dat_f_cnt;
   for( $j=0;$j<$dat_f_cnt;$j++){
      $wk1 = $A[$D[$j] - 1];
      $wk2 = $LEX_ARRAY[$y];
      if( num_chk($wk1) eq "false" )  { err_exit("DAT指定の列データが数値ではありません。REC:$in_cnt");}
      if( $marume == 0 ){
            #$wk3 = round_fix(sisu_conv($wk1 + $wk2),$keta);   # 切捨て
            $wk3 = round_fix2(sisu_conv($wk1 + $wk2),$keta);   # 切捨て
      }else{
         if( $marume == 1 ){
            #$wk3 = round_up(sisu_conv($wk1 + $wk2),$keta);    # 切上げ
            $wk3 = round_up2(sisu_conv($wk1 + $wk2),$keta);    # 切上げ
         }else{
            #$wk3 = round(sisu_conv($wk1 + $wk2),$keta);       # 四捨五入
            $wk3 = round2(sisu_conv($wk1 + $wk2),$keta);       # 四捨五入
         }
      }
      #printf("wk1:$wk1,wk2:$wk2,wk3:$wk3,keta:$keta\n");
      $LEX_ARRAY[$y] = $wk3;
      $y++;
   }
   return;
}
#--------------------------------------------------------#
# 配列取り出し $LEX_ARRAY ---> $LEX_DATA
#--------------------------------------------------------#
sub get_LEX_ARRAY{
   $y = 0;
   $LEX_DATA = "";
   for( $i=0;$i<$all_max;$i++){
      $wk = $LEX_ARRAY[$y];
      if( $i ==  ($all_max - 1) ){
          $LEX_DATA = $LEX_DATA . $wk;
      }else{
          $LEX_DATA = $LEX_DATA . $wk . $OFS;
      }
      $y++;
   }
   return $LEX_DATA;
}
#--------------------------------------------------------#
# 合計列データ取り出し
# 列数    :$dat_f_cnt
# OCCURS数:$f_max
# 列数×OCCURS数に分割する ---> $TOT_ARRAY[列数]
#--------------------------------------------------------#
sub get_total{
   my $in_data  = shift;
   my $out_data = "";;
   @TOT = split($OFS,$in_data);
   $t = 0;
   for($m=0;$m<$dat_f_cnt+1;$m++){
      $TOT_ARRAY[$m] = 0;
   }
   $t = 0;
   for($n=0;$n<$f_max;$n++){
      for($m=0;$m<$dat_f_cnt;$m++){
         $wk1 = $TOT[$t];
         $wk2 = $TOT_ARRAY[$m];
         if( $marume == 0 ){
            #$wk3 = round_fix(sisu_conv($wk1 + $wk2),$keta);      # 切捨
            $wk3 = round_fix2(sisu_conv($wk1 + $wk2),$keta);      # 切捨
         }else{
            if( $marume == 1 ){
               #$wk3 = round_up(sisu_conv($wk1 + $wk2),$keta);    # 切上
               $wk3 = round_up2(sisu_conv($wk1 + $wk2),$keta);    # 切上
            }else{
               #$wk3 = round(sisu_conv($wk1 + $wk2),$keta);       # 四捨五入
               $wk3 = round2(sisu_conv($wk1 + $wk2),$keta);       # 四捨五入
            }
         }
         #printf("合計列データ取り出し wk1:$wk1,wk2:$wk2,wk3:$wk3\n");
         $TOT_ARRAY[$m] = $wk3;
         $t++;
      }
   }
   $wk = "";
   $out_data = "";
   for($m=0;$m<$dat_f_cnt;$m++){
      if( $m == $dat_f_cnt - 1 ){
         $wk = $out_data . $TOT_ARRAY[$m];
      }else{
         $wk = $out_data . $TOT_ARRAY[$m] . $OFS;
      }
      $out_data = $wk;
   }
   return $out_data;
}
#------------------------------------------#
# Current Date Time ---> $dtime
#------------------------------------------#
sub get_dtime{
   ($w_sec,$w_min,$w_hour,$w_mday,$w_mon,$w_year,$wday)=localtime;
   $wday=('SUN','MON','TUE','WED','THU','FRI','SAT')[$wday];
   $w_year+=1900;
   $w_mon++;
   $sec  = sprintf("%02d",$w_sec);
   $min  = sprintf("%02d",$w_min);
   $hour = sprintf("%02d",$w_hour);
   $mday = sprintf("%02d",$w_mday);
   $mon  = sprintf("%02d",$w_mon);
   $year = sprintf("%04d",$w_year);
   $dtime="${year}/${mon}/${mday} (${wday}) ${hour}:${min}:${sec}";
   $yyyymmdd = "${year}${mon}${mday}";
   $hhmiss   = "${hour}${min}${sec}";
}
#------------------------------------------#
# 切上げ（少数桁指定でまるめ）
#------------------------------------------#
sub round_up {
    my $val = shift;    # 入力値
    my $col = shift;    # 小数点以下のどこまで残すか
    my $r = 10 ** $col;
    my $wk = ceil($val * $r) / $r;
    # ------------------------------------
    # 指数表記の数値変換
    # ------------------------------------
    $ret = sisu_conv($wk);
    #printf("*** 切上げ ret:$ret\n");
    return $ret;
}
#-------------------------------------------#
# 少数以下の切り上げ
#-------------------------------------------#
sub ceil {
    my $num = shift;
    my $val = 0;
    $val = 1 if($num > 0 and $num != int($num));
    return int($num + $val);
}
#------------------------------------------#
# 切捨て（少数桁指定でまるめ）
#------------------------------------------#
sub round_fix {
    my $val = shift;    # 入力値
    my $col = shift;    # 小数点以下のどこまで残すか
    my $r = 10 ** $col;
    $wk = int($val * $r) / $r;
    # ------------------------------------
    # 指数表記の数値変換
    # ------------------------------------
    $ret = sisu_conv($wk);
    #printf("*** 切捨て ret:$ret\n");
    return $ret;
}
#------------------------------------------#
# 四捨五入（少数桁指定でまるめ）
#------------------------------------------#
sub round {
    my $val = shift;    # 四捨五入する数
    my $col = shift;    # 小数点以下のどこまで残すか
    my $r = 10 ** $col;
    my $a = ($val > 0) ? 0.5 : -0.5;
    $wk = int($val * $r + $a) / $r;
    # ------------------------------------
    # 指数表記の数値変換
    # ------------------------------------
    $ret = sisu_conv($wk);
    #printf("*** 四捨五入 wk:$wk,ret:$ret\n");
    return $ret;
}
#------------------------------------------#
# 指数表記 ---> 数値
#------------------------------------------#
sub sisu_conv {
    my $val = shift;    # 入力数
    my $ret = "";
    # ------------------------------------
    # 指数表記になった場合の対応
    # ------------------------------------
    if( num_chk($val) eq "false" ) {
       # 指数を分解
       @S1 = split("e",$val);
       if( split("\\.",$S1[0]) > 0 ){
          @SU = split("\\.",$S1[0]);
          $a = $SU[0] . $SU[1];
       }else{
          $a = substr($S1[0],0,1);
       }
       $sign = substr($S1[1],0,1);
       $k = sprintf("%d",substr($S1[1],1));
       if( $sign eq "-" ){
          if( $k == 1  ){ $ret = "0." . $a; }
          if( $k == 2  ){ $ret = "0.0" . $a; }
          if( $k == 3  ){ $ret = "0.00" . $a; }
          if( $k == 4  ){ $ret = "0.000" . $a; }
          if( $k == 5  ){ $ret = "0.0000" . $a; }
          if( $k == 6  ){ $ret = "0.00000" . $a; }
          if( $k == 7  ){ $ret = "0.000000" . $a; }
          if( $k == 8  ){ $ret = "0.0000000" . $a; }
          if( $k == 9  ){ $ret = "0.00000000" . $a; }
          if( $k == 10 ){ $ret = "0.000000000" . $a; }
          if( $k == 11 ){ $ret = "0.0000000000" . $a; }
          if( $k == 12 ){ $ret = "0.00000000000" . $a; }
          if( $k == 13 ){ $ret = "0.000000000000" . $a; }
          if( $k == 14 ){ $ret = "0.0000000000000" . $a; }
          if( $k == 15 ){ $ret = "0.00000000000000" . $a; }
          if( $k == 16 ){ $ret = "0.000000000000000" . $a; }
          if( $k == 17 ){ $ret = "0.0000000000000000" . $a; }
          if( $k == 18 ){ $ret = "0.00000000000000000" . $a; }
          if( $k == 19 ){ $ret = "0.000000000000000000" . $a; }
          if( $k == 20 ){ $ret = "0.0000000000000000000" . $a; }
          if( $k == 21 ){ $ret = "0.00000000000000000000" . $a; }
       }else{
          $ret = $val;
       }
    }else{
       $ret = $val;
    }
    return $ret;
}
#------------------------------------------#
# 切上げ（少数桁指定でまるめ）
#------------------------------------------#
sub round_up2 {
    my $val = shift;    # 入力値
    my $col = shift;    # 小数点以下のどこまで残すか
    my $len = split("\\.",$val);
    my $sign = "";
    if( $len > 1 ){
       @SU = split("\\.",$val);
       $seisu = $SU[0];
       @SU1 = split("-",$SU[1]);
       #------------------
       # マイナス判定
       #------------------
       if( split("-",$SU[1]) > 1 ){
          $sign = "-";
          @SU1 = split("-",$SU[1]);
          $wk1 = $SU1[0] . $SU1[1];
       }else{
          $wk1 = $SU[1];
       }
       if( length($wk1) > $col ){
          #-----------------------
          # 少数部の切捨て処理
          #-----------------------
          $a = substr($wk1,0,$col);
          $b = substr($wk1,$col,1);
          if( $b > 0 ){ $a++; }
          $shosu = $a;
       }else{
          $shosu = $wk1;
       }
       $wk = $sign . $seisu . "." . $shosu;
    }else{
       $wk = $val;
    }
    # ------------------------------------
    # 指数表記の数値変換
    # ------------------------------------
    $ret = sisu_conv($wk);
    return $ret;
}
#------------------------------------------#
# 切捨て（少数桁指定でまるめ）
#------------------------------------------#
sub round_fix2 {
    my $val = shift;    # 入力値
    my $col = shift;    # 小数点以下のどこまで残すか
    my $len = split("\\.",$val);
    my $sign = "";
    my @SU = "";
    my @SU1 = "";
    if( $len > 1 ){
       @SU = split("\\.",$val);
       $seisu = $SU[0];
       @SU1 = split("-",$SU[1]);
       #------------------
       # マイナス判定
       #------------------
       if( split("-",$SU[1]) > 1 ){
          $sign = "-";
          @SU1 = split("-",$SU[1]);
          $wk1 = $SU1[0] . $SU1[1];
       }else{
          $wk1 = $SU[1];
       }
       if( length($wk1) > $col ){
          #-----------------------
          # 少数部の切捨て処理
          #-----------------------
          $a = substr($wk1,0,$col);
          $b = substr($wk1,$col,1);
          $shosu = $a;
       }else{
          $shosu = $wk1;
       }
       $wk = $sign . $seisu . "." . $shosu;
    }else{
       $wk = $val;
    }
    # ------------------------------------
    # 指数表記の数値変換
    # ------------------------------------
    $ret = sisu_conv($wk);
    return $ret;
}
#------------------------------------------#
# 四捨五入（少数桁指定でまるめ）
#------------------------------------------#
sub round2 {
    my $val = shift;    # 四捨五入する数
    my $col = shift;    # 小数点以下のどこまで残すか
    my $len = split("\\.",$val);
    my $sign = "";
    if( $len > 1 ){
       @SU = split("\\.",$val);
       $seisu = $SU[0];
       #------------------
       # マイナス判定
       #------------------
       if( split("-",$SU[1]) > 1 ){
          $sign = "-";
          @SU1 = split("-",$SU[1]);
          $wk1 = $SU1[0] . $SU1[1];
       }else{
          $wk1 = $SU[1];
       }
       if( length($wk1) > $col ){
          #-----------------------
          # 少数部の四捨五入処理
          #-----------------------
          $a = substr($wk1,0,$col);
          $b = substr($wk1,$col,1);
          if( $b > 4 ){ $a++; }
          $shosu = $a;
       }else{
          $shosu = $wk1;
       }
       
       $wk = $sign . $seisu . "." . $shosu;
    }else{
       $wk = $val;
    }
    # ------------------------------------
    # 指数表記の数値変換
    # ------------------------------------
    $ret = sisu_conv($wk);
    return $ret;
}
#------------------------------------------#
# 文字列が数値かどうかの判定
#------------------------------------------#
sub num_chk {
    my $val = shift;    # 入力データ
    # +-符号と小数点を含む数値
    if( $val =~ /^[+-]?\d+\.?\d*$/ ){
        return true;
    }else{
        return false;
    }
}
#------------------------------------------#
# 1_3:4:5文字列を1:2:3:4:5に変換
#------------------------------------------#
sub fld_all_cnv {
    my $val = shift;    # 入力データ
    @ALL_ARRAY = split(":",$val);
    $a_fld = "";
    $a_cnt = 0;
    foreach $a ( @ALL_ARRAY ) {
        $a_cnt++;
    }
    for($a_i=0;$a_i<$a_cnt;$a_i++) {
        $a = $ALL_ARRAY[$a_i];
        if( num_chk($a) eq "false" ) {
           $a = fld_cnv($a);
        }
        if( $a_i == 0 ){
           $a_fld = $a;
        }else{
           $a_fld = $a_fld . ":" . $a;
        }
    }
    return $a_fld;
}
#------------------------------------------#
# 1_3文字列を1:2:3に変換
#------------------------------------------#
sub fld_cnv {
    my $val = shift;    # 入力データ
    @FLD_ARRAY = split("_",$val);
    $s_cnt = 0;
    foreach $s ( @FLD_ARRAY ) {
        if( num_chk($s) eq "false" ) {
           err_exit("区切指定に間違いがあります。（数値を指定すること。）VAL:$s");
        }
        $s_cnt++;
    }
    if( $s_cnt == 2 ){
       $st = $FLD_ARRAY[0];
       $ed = $FLD_ARRAY[1];
       $fld = "";
       for( $n=$st;$n<$ed+1;$n++){
           if( $n == $ed ){
              $fld = $fld . $n;
           }else{
              $fld = $fld . $n . ":";
           }
       }
    }else{
       return $val;
    }
    return $fld;
}
#--------------------------------------------------------#
# エラーMSG表示＆エラー終了
# 引数1:エラーMSG Prefix
#--------------------------------------------------------#
sub err_exit{
   my $msg      = shift;
   &get_dtime;
   printf ("++ $PGM_NM $dtime $msg\n");
   printf ("++ $PGM_NM $dtime ABNORMAL END\n");
   if( $log_file ne "dummy" ){
      printf LOG ("++ $PGM_NM $dtime $msg\n");
      printf LOG ("++ $PGM_NM $dtime ABNORMAL END\n");
   }
   exit(1);
}
#--------------------------------------------------------#
# 通常MSG表示
# 引数1:MSG
#--------------------------------------------------------#
sub msg_out{
   my $msg      = shift;
   &get_dtime;
   printf ("++ $PGM_NM $dtime $msg\n");
   if( $log_file ne "dummy" ){
      printf LOG ("++ $PGM_NM $dtime $msg\n");
   }
   return;
}
#------------------------#
# argument check
#------------------------#
if ( $#ARGV != 11 ){
    print "$HELP_TITLE\n";
    print "lt_lex.pl in 2 out 2 8 2 1:2:4:5 1 7:8 Log 6 24\n";
    print "          |  | |   | | | |       | |   |   | |\n";
    print "          |  | |   | | | |       | |   |   | +--> 横展開OCCURS数（2～999）\n";
    print "          |  | |   | | | |       | |   |   +----> IDX指定       （単独列番号の指定）\n";
    print "          |  | |   | | | |       | |   |          横展開位置を表すIDX値を持つ列を指定\n";
    print "          |  | |   | | | |       | |   +--------> ログファイル名（FullPath）\n";
    print "          |  | |   | | | |       | |              dummyの場合は出力しない。\n";
    print "          |  | |   | | | |       | +------------> OCCURS列指定  （:区切りで指定）\n";
    print "          |  | |   | | | |       +--------------> 合計SW        （0:なし, 1:前合計, 2:後合計）\n";
    print "          |  | |   | | | +----------------------> KEY指定       （:区切りで指定）\n";
    print "          |  | |   | | +------------------------> まるめ        （0:切捨, 1:切上, 2:四捨五入）\n";
    print "          |  | |   | +--------------------------> 有効少数桁数  （0～14）\n";
    print "          |  | |   +----------------------------> OUTPUT区切指定（1:カンマ, 2:スペース, 3:TAB）\n";
    print "          |  | +--------------------------------> 出力ファイル名（FullPath）\n";
    print "          |  +----------------------------------> INPUT区切指定 （1:カンマ, 2:スペース, 3:TAB）\n";
    print "          +-------------------------------------> 入力ファイル名（FullPath）\n";
    print "＜注意＞\n";
    print "・入力データ中に横展開位置を決定するIDX値が無い場合は、IDX指定は、0を指定する。\n";
    print "  この場合の横展開をKEYブレークまでIDX値がカウントアップされて横展開する。\n";
    print "  横展開数があふれた場合は、あふれ分は無視して横展開OCCURS数しか出力されないので注意すること。\n";
    print "・合計SWは、OCCURSの前または後ろに位置し、KEYブレークまで足し込みされます。\n";
    print "  合計SW=0の場合は、合計列は出力しない。\n";
    print "  合計SW=1の場合は、OCCURSの前に合計列を出力する。\n";
    print "  合計SW=2の場合は、OCCURSの後ろに合計列を出力する。\n";
    print "・連続する列は、1_4という書式で記述すれば、1:2:3:4と記述したことと同じとする。\n";
    print "--------------------------------------------------------------------\n";
    print " ＜処理内容イメージ図＞\n";
    print "  ・入力ファイルをKEYブレークするまで横展開して出力する。\n";
    print "    入力ファイルには、横展開位置を決定するIDX値を持つ必要がある。\n";
    print "    入力ファイルにIDX値を持つ列が無い場合は,IDX列番号には0指定をする。\n";
    print "    ただし、この場合は、単純にカウントアップ横展開する。\n";
    print "    同一IDX位置が発生した場合は、たし込まれる。\n";
    print "   １回もIDX位置が出現しない場合は、0出力。\n";
    print "\n";
    print "       KEY=1:2:4:5, 合計SW=1, OCCURS列=7:8, IDX=6, OCCURS数=24 \n";
    print "       +-------------------------------+\n";
    print "       |  KEY              | DATA      |\n";
    print "       +-------------------+-----------+\n";
    print "       | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |\n";
    print "       +-----------+-------------------+\n";
    print "                   |\n";
    print "           +-------+-------+\n";
    print "           |   lt_lex.pl   | 上記の第6列を横展開IDXとして使用する。\n";
    print "           +-------+-------+\n";
    print "                   |\n";
    print "   +---------------+---------------------+\n";
    print "   |               | DATA                | \n";
    print "   |    KEY        +-------+-------------+\n";
    print "   |               |前合計 |  OCCURS 24  |\n";
    print "   +---------------+-------+-------------+\n";
    print "   | 1 | 2 | 4 | 5 | 7 | 8 | 7 | 8 | ... | ---> KEY列 ＋ 合計列 ＋ 2列 × 24 OCCURS\n";
    print "   +---------------+---------------------+\n";
    print "--------------------------------------------------------------------\n";
    exit(1);
}
$in_file    = shift(@ARGV);
$in_fs      = shift(@ARGV);
$out_file   = shift(@ARGV);
$out_fs     = shift(@ARGV);
$keta       = shift(@ARGV);
$marume     = shift(@ARGV);
$keys       = shift(@ARGV);
$total_sw   = shift(@ARGV);
$dats       = shift(@ARGV);
$log_file   = shift(@ARGV);
$f_idx      = shift(@ARGV);
$f_max      = shift(@ARGV);
$key_f_cnt  = 0;
$dat_f_cnt  = 0;
&get_dtime;
if( $log_file ne "dummy" ){
   if ( !open(LOG,">>$log_file") ){ err_exit("Log File Not Open :$log_file");}
}
#*******************************************************
# :区切指定の省略形の整形を行う。1_5 ---> 1:2:3:4:5
#*******************************************************
$p_keys = $keys;
$p_dats = $dats;
$keys   = fld_all_cnv($keys);
$dats   = fld_all_cnv($dats);
#----------------------------
# 引数の整合性チェック
#----------------------------
if( num_chk($in_fs) eq "false" ) { err_exit("INPUT区切指定に間違いがあります。（数値を指定すること。）");}
if( $in_fs < 1 || 3 < $in_fs )   { err_exit("INPUT区切指定に間違いがあります。（1～3を指定すること。）");}
if( $in_fs == 1 ){ $IFS = $COM; }
if( $in_fs == 2 ){ $IFS = $SPC; }
if( $in_fs == 3 ){ $IFS = $TAB; }
if( num_chk($out_fs) eq "false" ){ err_exit("OUTPUT区切指定に間違いがあります。（数値を指定すること。）");}
if( $out_fs == 1 ){ $OFS = $COM; }
if( $out_fs == 2 ){ $OFS = $SPC; }
if( $out_fs == 3 ){ $OFS = $TAB; }
if( $out_fs < 1 || 3 < $out_fs ) { err_exit("OUTPUT区切指定に間違いがあります。（1～3を指定すること。）");}
if( num_chk($keta) eq "false" )  { err_exit("有効少数桁数に間違いがあります。（数値を指定すること。）");}
if( $keta < 1 || 14 < $keta )    { err_exit("有効少数桁数に間違いがあります。（1～14を指定すること。）");}
if( num_chk($marume) eq "false" ){ err_exit("まるめ指定に間違いがあります。（数値を指定すること。）");}
if( $marume < 0 || 2 < $marume ) { err_exit("まるめ指定に間違いがあります。（0～2の範囲で指定すること。）");}
@K = split(":",$keys);
foreach $kstr ( @K ) {
   if( num_chk($kstr) eq "false" ){ err_exit("KEY指定の列番号に間違いがあります。（数値を指定すること。）");}
   $key_f_cnt++;
}
if( num_chk($total_sw) eq "false" ) { err_exit("合計SWに間違いがあります。（数値を指定すること。）");}
if( $total_sw < 0 || 2 < $total_sw ){ err_exit("合計SWに間違いがあります。（0～2を指定すること。）");}

@D = split(":",$dats);
foreach $dstr ( @D ) {
   if( num_chk($dstr) eq "false" ){ err_exit("DAT指定の列番号に間違いがあります。（数値を指定すること。）");}
   $dat_f_cnt++;
}
if( num_chk($f_idx) eq "false" ) { err_exit("IDX指定に間違いがあります。（数値を指定すること。）");}
if( num_chk($f_max) eq "false" ) { err_exit("横展開OCCURS数が数値ではありません。（数値を指定すること。）");}
if( $f_max < 2 || 999 < $f_max ) { err_exit("横展開OCCURS数に間違いがあります。（2～999を指定すること。）");}
#----------------------------
# 横展開の全列数の計算
# 全列数 = 横展開数 * DAT指定の列数
#----------------------------
$all_max = $f_max * $dat_f_cnt;
#----------------------------
# 引数情報のログ表示
#----------------------------
msg_out("--------横レイアウト変換（LawsonTool）引数情報--------");
msg_out(" 1.INPUTファイル名     :$in_file");
msg_out(" 2.INPUT区切指定       :$in_fs");
msg_out(" 3.OUTPUTファイル名    :$out_file");
msg_out(" 4.OUTPUT区切指定      :$out_fs");
msg_out(" 5.有効少数桁数        :$keta");
msg_out(" 6.まるめ              :$marume");
msg_out(" 7.KEY指定             :$p_keys");
msg_out(" 8.合計SW              :$total_sw");
msg_out(" 9.DAT指定             :$p_dats");
msg_out("10.ログファイル名      :$log_file");
msg_out("11.IDX指定             :$f_idx");
msg_out("12.横展開OCCURS数      :$f_max");
msg_out("------------------------------------------------------");
#----------------------------
# ファイル・オープン
#----------------------------
&get_dtime;
if ( !open(INF, $in_file) )   { err_exit("Input File Not Open :$in_file");}
if ( !open(OUT,">$out_file") ){ err_exit("Output File Not Open :$out_file");}
msg_out("START");
$in_cnt = 0;
$out_cnt = 0;
$x = 0;
$old_key = "";
$old_data = "";
&clear_LEX_ARRAY;
$z = 0;
while(<INF>){
   chop;
   $in_cnt++;
   $new_key = "";
   $new_data = "";
   @A = split($IFS,$_);
   #-----------------------------------
   # KEYの抜き出し ---> $new_key
   # DATの抜き出し ---> $new_dat
   # IDXの抜き出し ---> $idx
   #-----------------------------------
   for( $i=0;$i<$key_f_cnt;$i++){
      $kstr = $K[$i];
      $x = $kstr - 1;
      if( $i ==  ($key_f_cnt - 1) ){
         $new_key = $new_key . $A[$x];
      }else{
         $new_key = $new_key . $A[$x] . $OFS;
      }
   }
   for( $i=0;$i<$dat_f_cnt;$i++){
      $dstr = $D[$i];
      $x = $dstr - 1;
      # 数値の最大桁チェック
      $len = length($A[$x]);
      $wk = $A[$x];
      if( $len > 16 ){err_exit("16桁以上（少数点含む）の数値は処理できません。REC:$in_cnt,VALUE:$wk");}
      if( $i ==  ($dat_f_cnt - 1) ){
         $new_data = $new_data . $A[$x];
      }else{
         $new_data = $new_data . $A[$x] . $OFS;
      }
   }
   $x = $f_idx-1;
   $idx = $A[$x];
   if( num_chk($idx) eq "false" )  { err_exit("IDX指定の列データが数値ではありません。REC:$in_cnt");}
   &get_dtime;
   #-----------------------------------
   # 1件目だけ特別処理
   # ブレーク判定せずに無条件サマリー
   #-----------------------------------
   if( $in_cnt == 1 ){
      $old_key = $new_key;
      $old_data = $new_data;
      #-------------------------------------------
      # $xが-1の場合は、カウントアップ横展開する。
      #-------------------------------------------
      if( $x == -1){
         add_LEX_ARRAY($z);
         $z++;
      }else{
         add_LEX_ARRAY($idx);
      }
      next;
   }
   #-----------------------------------
   # 1件目以降はブレーク判定
   #-----------------------------------
   if( $old_key ne $new_key ){
      $old_data = get_LEX_ARRAY();
      #msg_out("***BREAK***  KEY:$old_key,DATA:$old_data");
      #----------------------------------------
      # KEYブレークした時のみ出力
      # $old_keyと$old_dataを出力
      #----------------------------------------
      if( $total_sw == 0 ){
         printf OUT ("$old_key$OFS$old_data\n");
      }else{
         $total_data = get_total($old_data);
         if( $total_sw == 1 ){
            printf OUT ("$old_key$OFS$total_data$OFS$old_data\n");
         }else{
            printf OUT ("$old_key$OFS$old_data$OFS$total_data\n");
         }
      }
      $out_cnt++;
      $old_key = $new_key;
      $old_data = $new_data;
      #----------------------------------------
      # KEYブレーク時は、集計エリアをクリア
      #----------------------------------------
      &clear_LEX_ARRAY;
      #-------------------------------------------
      # $xが-1の場合は、カウントアップ横展開する。
      #-------------------------------------------
      if( $x == -1){
         $z = 0;
         add_LEX_ARRAY($z);
         $z++;
      }else{
         add_LEX_ARRAY($idx);
      }
   }else{
      #-------------------------------------------
      # 以外は、横展開（$idx位置へ展開）
      # OCCURS列の足し込み --> $LEX_ARRAY
      #-------------------------------------------
      # $xが-1の場合は、カウントアップ横展開する。
      #-------------------------------------------
      if( $x == -1){
         add_LEX_ARRAY($z);
         $z++;
      }else{
         add_LEX_ARRAY($idx);
      }
   }
}
if( $in_cnt > 0 ){
   $old_data = get_LEX_ARRAY();
   #msg_out("***BREAK***  KEY:$old_key,DATA:$old_data");
   if( $total_sw == 0 ){
      printf OUT ("$old_key$OFS$old_data\n");
   }else{
      $total_data = get_total($old_data);
      if( $total_sw == 1 ){
         printf OUT ("$old_key$OFS$total_data$OFS$old_data\n");
      }else{
         printf OUT ("$old_key$OFS$old_data$OFS$total_data\n");
      }
   }
   $out_cnt++;
   $old_key = $new_key;
}
close(INF);
close(OUT);
#------------------------------------------------------------------------#
# End Proc
#------------------------------------------------------------------------#
&get_dtime;
$msg_in_cnt   = sprintf("%08d",$in_cnt);
$msg_out_cnt  = sprintf("%08d",$out_cnt);
msg_out("INPUT  COUNT=$msg_in_cnt");
msg_out("OUTPUT COUNT=$msg_out_cnt");
msg_out("NORMAL END");
exit 0;

