#!/usr/bin/perl
#------------------------------------------------------------------------#
# Script-id     : lt_join.pl
# 機能          : 結合
# 作成者        : Y.Takabayashi
# 作成日        : 2012/05/10
# 修正履歴      :
# Ｎｏ   修正日付   修正者     修正内容
#    1  yyyy/mm/dd  XXXXXXXXX  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#------------------------------------------------------------------------#
$PGM_NM     = "lt_join.pl";                # Program Name
$HELP_TITLE = "＜lt_join.pl＞ 結合（LawsonTool）";
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
#-----------------------------------------#
# エラーMSG表示＆エラー終了
# 引数1:エラーMSG Prefix
#-----------------------------------------#
sub err_exit{
   my $msg      = shift;
   &get_dtime;
   printf ("++ $PGM_NM $dtime $msg\n");
   printf ("++ $PGM_NM $dtime ABNORMAL END\n");
   if( $log_file ne "dummy" ){
      printf LOG ("++ $PGM_NM $dtime $msg\n");
      printf LOG ("++ $PGM_NM $dtime ABNORMAL END\n");
   }
   exit(9);
}
#-----------------------------------------#
# 通常MSG表示
# 引数1:MSG
#-----------------------------------------#
sub msg_out{
   my $msg      = shift;
   &get_dtime;
   printf ("++ $PGM_NM $dtime $msg\n");
   if( $log_file ne "dummy" ){
      printf LOG ("++ $PGM_NM $dtime $msg\n");
   }
   return;
}
#------------------------------------------#
# イニシャル・チェック・サブルーチン
#------------------------------------------#
sub initial_chk_rtn {
   if ( $mst_sp eq '1' ){
      $mst_sp = ',';
   }elsif( $mst_sp eq '2' ){
      $mst_sp = ' ';
   }elsif( $mst_sp eq '3' ){
      $mst_sp = "\x09";
   }else{
      err_exit("***ERR*** マスタ区切形式は、1～3を指定する。");
   }
   if ( $trn_sp eq '1' ){
      $trn_sp = ',';
   }elsif( $trn_sp eq '2' ){
      $trn_sp = ' ';
   }elsif( $trn_sp eq '3' ){
      $trn_sp = "\x09";
   }else{
      err_exit("***ERR*** トラン区切形式は、1～3を指定する。");
   }
   if ( $c_sp eq '1' ){
      $c_sp = ',';
   }elsif( $c_sp eq '2' ){
      $c_sp = ' ';
   }elsif( $c_sp eq '3' ){
      $c_sp = "\x09";
   }else{
      err_exit("***ERR*** 設定区切形式は、1～3を指定する。");
   }
   if ( $o_sp eq '1' ){
      $o_sp = ',';
   }elsif( $o_sp eq '2' ){
      $o_sp = ' ';
   }elsif( $o_sp eq '3' ){
      $o_sp = "\x09";
   }else{
      err_exit("***ERR*** 出力区切形式は、1～3を指定する。");
   }
   my $join_type_no = 0 + $join_type;
   if( $join_type !~ /^\d+$/ || $join_type_no < 1 || $join_type_no > 7 ){
      err_exit("***ERR*** 結合方式は、1～7を指定する。");
   }
   my $sort_ordr_no = 0 + $sort_ordr;
   if( $sort_ordr !~ /^\d+$/ || $sort_ordr_no < 1 || $sort_ordr_no > 3 ){
      err_exit("***ERR*** ソート順は、1～3を指定する。");
   }
}
#------------------------------------------#
# 設定ファイル処理・サブルーチン
#------------------------------------------#
sub cnf_file_rtn {
   if ( !open(CNF_FILE, $cnf) ){err_exit("設定ファイルがＯＰＥＮできません。$cnf");}
   $c_cnt = 0;
   $i = 0;
   $j = 0;
   while(<CNF_FILE>){
      chop;
      if( substr($_,0,1) eq '#' || length($_) == 0){
         next;
      }
      if( length($_) < 11 ){
         msg_out("***WARNING*** 無効な設定ファイルレコードをSKIPしました。DATA:$_");
         next;
      }
      ( $sw, $mst_no_zone, $trn_no_zone, $out_no_zone, $df_vl, $nvl_vl, $fld_name ) = split($c_sp, $_);
      $mst_no = 0 + $mst_no_zone;
      $trn_no = 0 + $trn_no_zone;
      $out_no = 0 + $out_no_zone;
      #--------------------------------------
      # 設定ファイルの項目番号の範囲チェック
      #--------------------------------------
      if( $mst_no_zone !~ /^\d+$/ || $mst_no < 0 || $mst_no > 9999 ){
         msg_out("***ERR*** 設定ファイルのマスタの項目番号は、0000～9999の範囲で指定すること。");
         err_exit(" ***ERR*** mst_no:$mst_no_zone");
      }
      if( $trn_no_zone !~ /^\d+$/ || $trn_no < 0 || $trn_no > 9999 ){
         msg_out("***ERR*** 設定ファイルのトランの項目番号は、0000～9999の範囲で指定すること。");
         err_exit("***ERR*** trn_no:$trn_no_zone");
      }
      if( $out_no_zone !~ /^\d+$/ || $out_no < 0 || $out_no > 9999 ){
         msg_out("***ERR*** 設定ファイルの出力番号は、0000～9999の範囲で指定すること。");
         err_exit("***ERR*** out_no:$out_no_zone");
      }
      if( $sw eq "1" ){
         @mst_key_array[$i]     = $mst_no;
         @trn_key_array[$i]     = $trn_no;
         @out_key_array[$i]     = $out_no;
         @key_df_vl_array[$i]   = $df_vl;
         @key_nvl_vl_array[$i]  = $nvl_vl;
         @key_name_array[$i]    = $fld_name;
         $i++;
      }elsif( $sw eq "2" ){
         @mst_data_array[$j]    = $mst_no;
         @trn_data_array[$j]    = $trn_no;
         @out_data_array[$j]    = $out_no;
         @data_df_vl_array[$j]  = $df_vl;
         @data_nvl_vl_array[$j] = $nvl_vl;
         @data_name_array[$j]   = $fld_name;
         $j++;
      }else{
         next;
      }
      $c_cnt++;
   }
   close(KEY_FILE);
   if( $i == 0 ){err_exit("***ERR*** １項目以上のKEY設定をして下さい。"); }
   $key_len = $i;
   $data_len = $j;
   $all_len = $key_len + $data_len;
   
   @wk_out_array = ();
   push(@wk_out_array, @out_key_array);
   push(@wk_out_array, @out_data_array);
   @wk_out_array = sort { $a <=> $b } @wk_out_array;
   $y = 0;
   for($x=0;$x<$all_len;$x++){
      if( @wk_out_array[$x] != 0 ){
         if( @wk_out_array[$x] != ++$y ){
            msg_out("***ERR*** 設定ファイルの出力番号は0001からの連番で指定すること。");
            err_exit("***ERR*** out_no:$y");
         }
      }
   }
   $out_len = $y;
   %out_tbl = ();
}
#---------------------------------------------#
# 入力ファイル読み込みサブルーチン
#---------------------------------------------#
sub read_hash_rtn {
   my ( $fl_idx, $fl, $i_sp, $fl_nm, $key_array, $data_array ) = @_;
   if ( !open(FL, $fl) ){err_exit("指定の$fl_nmがOPENできません。$fl"); }
   my $ln_cnt = 0;
   while(<FL>){
      chop;
      if( length($_) == 0){
         next;
      }
      $ln_cnt++;
      my $len = split($i_sp , $_);
      my @sp_array = split($i_sp , $_);
      my $key = "";
      my $data = "";
      my $idx = 0;
      my $out_idx = 0;
      my $wk = "";
      
      #--------------------------#
      # KEYの抜き出し＆連結
      #--------------------------#
      my %wk_key_tbl = ();
      my @wk_key_0_array = ();
      my @wk_key_array = ();
      for($x=0;$x<$key_len;$x++){
         $idx = @$key_array[$x] - 1;
         $out_idx = @out_key_array[$x] - 1;
         if( $idx >= 0 ){
            #--------------------------------------------------------#
            # 設定ファイルで指定した列番号が入力列数よりも大きいか？
            #--------------------------------------------------------#
            if( $len <= $idx ){err_exit("設定ファイルのキーの列番号が$fl_nmファイルにありません。idx:" . ++$idx); }
            $wk = @sp_array[$idx];
            if( $out_idx >= 0 ){
               $wk_key_tbl{$out_idx} = $wk;
            }else{
               push( @wk_key_0_array, $wk );
            }
         }
      }
      my @wk_out_idx_array = keys( %wk_key_tbl );
      @wk_out_idx_array = sort { $a <=> $b } @wk_out_idx_array;
      foreach ( @wk_out_idx_array ) {
         push( @wk_key_array, $wk_key_tbl{$_} );
      }
      push( @wk_key_array, @wk_key_0_array );
      $key = join( $o_sp, @wk_key_array );
      
      #--------------------------#
      # 連想配列から出力項目の取得
      #--------------------------#
      my $wk_array = [];
      my $wk_flg_array = [];
      my $wk_out_array = [];
      if( exists( $out_tbl{ $key } ) ){
         $wk_array = $out_tbl{$key};
         $wk_flg_array = @$wk_array[0];
         $wk_out_array = @$wk_array[1];
      }else{
         @$wk_flg_array[0] = '0';
         @$wk_flg_array[1] = '0';
         for($x=0;$x<$out_len;$x++){
            @$wk_out_array[$x] = "";
         }
         for($x=0;$x<$data_len;$x++){
            $out_idx = @out_data_array[$x] - 1;
            if( $out_idx >= 0 ){
               @$wk_out_array[$out_idx] = @data_df_vl_array[$x];
            }
         }
      }
      
      #--------------------------#
      # KEYを出力項目として格納
      #--------------------------#
      for($x=0;$x<$key_len;$x++){
         $idx = @$key_array[$x] - 1;
         $out_idx = @out_key_array[$x] - 1;
         if( $idx >= 0 && $out_idx >= 0 ){
            #--------------------------------------------------------#
            # 設定ファイルで指定した列番号が入力列数よりも大きいか？
            #--------------------------------------------------------#
            if( $len <= $idx ){err_exit("設定ファイルのキーの列番号が$fl_nmファイルにありません。idx:" . ++$idx); }
            $wk = @sp_array[$idx];
            if( $wk ne "" ){
               @$wk_out_array[$out_idx] = $wk;
            }else{
               @$wk_out_array[$out_idx] = @key_nvl_vl_array[$x];
            }
         }
      }
      #--------------------------#
      # DATAを出力項目として格納
      #--------------------------#
      for($x=0;$x<$data_len;$x++){
         $idx = @$data_array[$x] - 1;
         $out_idx = @out_data_array[$x] - 1;
         if( $idx >= 0 && $out_idx >= 0 ){
            #--------------------------------------------------------#
            # 設定ファイルで指定した列番号が入力列数よりも大きいか？
            #--------------------------------------------------------#
            if( $len <= $idx ){err_exit("設定ファイルのデータの列番号が$fl_nmファイルにありません。idx:" . ++$idx); }
            $wk = @sp_array[$idx];
            if( $wk ne "" ){
               @$wk_out_array[$out_idx] = $wk;
            }else{
               @$wk_out_array[$out_idx] = @data_nvl_vl_array[$x];
            }
         }
      }
      @$wk_flg_array[$fl_idx] = '1';
      @$wk_array[0] = $wk_flg_array;
      @$wk_array[1] = $wk_out_array;
      $out_tbl{$key} = $wk_array;   # OTPT連想配列へ格納
   }
   close(FL);
   return $ln_cnt;
}
#==============================================================================#
#-----------------------------#
# 引数チェック
#-----------------------------#
if ( $#ARGV != 10 ){
    print "$HELP_TITLE\n";
    printf("\n");
    printf("・マスターファイルとトランファイルを引数の結合方式に応じて結合して出力する。\n");
    printf("  内部結合の場合、マスタとトランの両方に存在するキーのレコードを出力。\n");
    printf("  左外部結合の場合、マスタに存在するキーのレコードを出力。\n");
    printf("  右外部結合の場合、トランに存在するキーのレコードを出力。\n");
    printf("  どちらかに存在の場合、マスタとトランに存在する全てのキーのレコードを出力。\n");
    printf("  どちらかにのみ存在の場合、マスタとトランの片方のみに存在するキーのレコードを出力。\n");
    printf("  マスタのみに存在の場合、マスタに存在しトランに存在しないキーのレコードを出力。\n");
    printf("  トランのみに存在の場合、トランに存在しマスタに存在しないキーのレコードを出力。\n");
    printf("lt_join.pl mst trn cnf 2 1 3 1 1 1 o LogFile\n");
    printf("           |   |   |   | | | | | | | |\n");
    printf("           |   |   |   | | | | | | | +-> ログファイル\n");
    printf("           |   |   |   | | | | | | |     dummyの場合は出力しない。\n");
    printf("           |   |   |   | | | | | | +---> 出力ファイル\n");
    printf("           |   |   |   | | | | | |       dummyの場合は出力しない。\n");
    printf("           |   |   |   | | | | | +-----> ソート順（1:キー全体の昇順,2:キー全体の降順,3:ソートしない）\n");
    printf("           |   |   |   | | | | +-------> 結合方式（1:内部結合,2:左外部結合,3:右外部結合,4:どちらかに存在,5:どちらかにのみ存在,6:マスタのみに存在,7:トランのみに存在）\n");
    printf("           |   |   |   | | | +---------> 出力区切形式（1:カンマ,2:スペース,3:TAB）\n");
    printf("           |   |   |   | | +-----------> 設定区切形式（1:カンマ,2:スペース,3:TAB）\n");
    printf("           |   |   |   | +-------------> トラン区切形式（1:カンマ,2:スペース,3:TAB）\n");
    printf("           |   |   |   +---------------> マスタ区切形式（1:カンマ,2:スペース,3:TAB）\n");
    printf("           |   |   +-------------------> 設定ファイル\n");
    printf("           |   +-----------------------> トランファイル\n");
    printf("           +---------------------------> マスタファイル\n");
    printf("【注意】\n");
    printf("・このプログラムは、ファイルを全件連想配列としてメモリー展開します。\n");
    printf("  マスタファイル、トランファイルともソートする必要はありません。\n");
    printf("  ※ファイルにキー重複があった場合は、後で読み込んだ内容で上書きされます。\n");
    printf("・oには、通常ファイル名を記述しますがdummyと記述すれば、何も出力しません。\n");
    printf("  ※ただし、処理終了時の件数メッセージには本来の件数が表示されます。\n");
    printf("・マスタファイルとトランファイルは、同じフォーマットである必要はありませんが\n");
    printf("  キー指定とDATA指定を双方とも列番号位置を編集しておく必要があります。\n");
    printf("・実行ログである開始/終了メッセージやエラーメッセージは、標準出力に返されます。\n");
    printf("・マスタファイル、トランファイル、設定ファイルともに空行データはSKIPします。\n");
    printf("・設定ファイルのフォーマットは以下のとうり。\n");
    printf("  先頭が#は、コメント行として扱う。（先頭が1,2以外は、読み飛ばします。）\n");
    printf("  先頭が1の場合は、KEY設定。   （マッチングキーの設定）\n");
    printf("  先頭が2の場合は、DATA設定。  （データ照合の位置設定）\n");
    printf("  マスタおよびトランの列指定は、１からの列番号で指定します。\n");
    printf("  DATA設定で0000を指定したファイルからは読み込まれません。\n");
    printf("  DATA設定でマスタおよびトランの両方とも0000以外を指定した場合、トランの値が優先されます。\n");
    printf("  4列目は、出力列の設定。  （1からの連番で且つ重複が無いものでなければなりません。ただし0000を指定する事で出力しない事も可能です。）\n");
    printf("  5列目は、デフォルト値の設定。  （どちらかのファイルにしか存在しないキーの出力項目に出力します。）\n");
    printf("  6列目は、NVL値の設定。  （ファイルから読み込んだ出力項目が空の場合に出力します。）\n");
    printf("  入力ファイルは、CSVのように項目間に区切り文字のある形式でなければなりません。\n");
    printf("# MAST TRAN OTPT DFLT NVLV NAME\n");
    printf("1 0001 0001 0001   商品コード\n");
    printf("1 0002 0002 0000   日付\n");
    printf("2 0003 0000 0004   商品名漢字\n");
    printf("2 0000 0004 0003 0 0 売価_T\n");
    printf("2 0004 0000 0002 0 0 売価_M\n");
    printf("2 0000 0005 0005   商品名カナ\n");
    exit(9);
}
#-----------------------------#
# 初期処理部
#-----------------------------#
$|=1;
$mst        = shift(@ARGV);
$trn        = shift(@ARGV);
$cnf        = shift(@ARGV);
$mst_sp     = shift(@ARGV);
$trn_sp     = shift(@ARGV);
$c_sp       = shift(@ARGV);
$o_sp       = shift(@ARGV);
$join_type  = shift(@ARGV);
$sort_ordr  = shift(@ARGV);
$out_file   = shift(@ARGV);
$log_file   = shift(@ARGV);
&get_dtime;
if( $out_file ne "dummy" ){
   if( !open(OUT_FILE,">$out_file") ){ err_exit("出力ファイルがOPENできません。$out_file");}
}
if( $log_file ne "dummy" ){
   if ( !open(LOG,">>$log_file") ){ err_exit("Log File Not Open :$log_file");}
}
#-----------------------------#
# 引数情報のログ表示
#-----------------------------#
msg_out("------結合（LawsonTool）引数情報-------");
msg_out(" 1.MAST FILE           :$mst");
msg_out(" 2.TRAN FILE           :$trn");
msg_out(" 3.CONF FILE           :$cnf");
msg_out(" 4.マスタ区切形式      :$mst_sp");
msg_out(" 5.トラン区切形式      :$trn_sp");
msg_out(" 6.設定区切形式        :$c_sp");
msg_out(" 7.出力区切形式        :$o_sp");
msg_out(" 8.結合方式            :$join_type");
msg_out(" 9.ソート順            :$sort_ordr");
msg_out("10.OUTPUT FILE         :$out_file");
msg_out("11.ログファイル名      :$log_file");
msg_out("---------------------------------------");
msg_out("START");
#-----------------------------#
# イニシャル・チェック
#-----------------------------#
&initial_chk_rtn;
#-----------------------------#
# 設定ファイル処理
#-----------------------------#
&cnf_file_rtn;
#-----------------------------#
# マスタ・ファイル読み込み
#-----------------------------#
$mst_cnt = &read_hash_rtn( 0, $mst, $mst_sp, "マスタ", \@mst_key_array, \@mst_data_array );
#-----------------------------#
# トラン・ファイル読み込み
#-----------------------------#
$trn_cnt = &read_hash_rtn( 1, $trn, $trn_sp, "トラン", \@trn_key_array, \@trn_data_array );

@tbl_key_array = keys( %out_tbl );
if( $sort_ordr == '1' ){
   @tbl_key_array = sort { $a cmp $b } @tbl_key_array;
}elsif( $sort_ordr == '2' ){
   @tbl_key_array = sort { $b cmp $a } @tbl_key_array;
}

if( $out_file ne "dummy" ){
   $out_cnt = 0;
   foreach ( @tbl_key_array ) {
      my $wk_array = $out_tbl{$_};
      my $wk_flg_array = @$wk_array[0];
      my $wk_out_array = @$wk_array[1];
      my $mst_flg = @$wk_flg_array[0];
      my $trn_flg = @$wk_flg_array[1];
      if(
         ( $join_type == '1' && $mst_flg == '1' && $trn_flg == '1' )
      || ( $join_type == '2' && $mst_flg == '1' )
      || ( $join_type == '3' && $trn_flg == '1' )
      || ( $join_type == '4' && ( $mst_flg == '1' || $trn_flg == '1' ) )
      || ( $join_type == '5' && ( ( $mst_flg == '1' && $trn_flg == '0' ) || ( $mst_flg == '0' && $trn_flg == '1' ) ) )
      || ( $join_type == '6' && $mst_flg == '1' && $trn_flg == '0' )
      || ( $join_type == '7' && $mst_flg == '0' && $trn_flg == '1' )
      ){
         printf( OUT_FILE "%s\n", join( $o_sp, @$wk_out_array ) );
         $out_cnt++;
      }
   }
   close(OUT_FILE);
}

#------------------------------------------------------------------------#
#      終了処理部
#------------------------------------------------------------------------#
&get_dtime;
$msg_mst_cnt   = sprintf("%09d",$mst_cnt);
$msg_trn_cnt   = sprintf("%09d",$trn_cnt);
$msg_c_cnt     = sprintf("%09d",$c_cnt);
$msg_out_cnt   = sprintf("%09d",$out_cnt);
msg_out("MAST FILE    COUNT=$msg_mst_cnt");
msg_out("TRAN FILE    COUNT=$msg_trn_cnt");
msg_out("CONF FILE    COUNT=$msg_c_cnt");
msg_out("OTPT FILE    COUNT=$msg_out_cnt");
msg_out("NORMAL END");
if( $out_cnt < 1 ){
   exit 1;
}else{
   exit 0;
}
