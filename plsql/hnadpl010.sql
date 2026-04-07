/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：HNT_日付コントロール更新処理
/*　　　ＰＧＭID　　　　：hnadpl010.sql
/*　　　機能　　　　　　：HNT_日付コントロールを更新する
/*　　　　　　　　　　　　　XPM_販計日付コントロール更新処理を流用
/*        1. 日別コントロール・レコードを作成する。
/*             区分     = 0（日別コントロール）
/*             変位     = -800 ～ 800
/*             日付1    = 当日＋変位
/*             日付2    = 翌日＋変位
/*        2. 週別コントロール・レコードを作成する。
/*             区分     = 1（週別コントロール）
/*             変位     = -116 ～ 116
/*             日付1    =（当週＋変位）週の月曜日
/*             日付2    =（当週＋変位）週の日曜日
/*        3. 月別コントロール・レコードを作成する。
/*             区分     = 2（月別コントロール）
/*             変位     = -26 ～ 26
/*             日付1    =（当月＋変位）月の１日
/*             日付2    =（当月＋変位）月の末日
/*        4. 上記 1, 2, 3 共通事項
/*             曜日区分 = 日付1の曜日（月曜日から 1,2, ... ,7）
/*             週No.    = 日付1の週№の該当年＋日付1の週No
/*　　　インプット　　　：HNT_日付コントロール
/*　　　　　　　　　　　：
/*　　　アウトプット　　：HNT_日付コントロール 
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：G.Saito
/*　　　作成日　　　　　：2009/09/29
/*　　　修正履歴　　　　：YYYY/MM/DD xxxxx
/*                      ：2010/03/15 VIXUS)G.Saito  返品２対応 各レコードを２倍持たせる
/********************************************************************************/
whenever oserror  exit sql.sqlcode rollback
whenever sqlerror exit sql.sqlcode rollback
set feed off
set verify off
set heading off
set underline off
set termout on
set serveroutput on size 20000

variable RtnCd     number
variable ErrMsg    varchar2(100)

---------------------------------------------
-- 変数宣言
---------------------------------------------
DECLARE
    w_time_start varchar2(19);  -- プログラム開始日時
    w_time_end   varchar2(19);  -- プログラム終了日時

    w_cnt        number(10);    -- INSERT件数

    wk_sysdate   date;
    wk_num1      char(6);

    日付変位数   number(3);
    変位日１     date;
    変位日２     date;
    曜日区分     char(1);
    年度週ＮＯ   char(6);

    matsu_youbi_kbn  char(1);
    syori_youbi_kbn  char(1);

    w_date       date;
    w_date_year  date;
    w_date2      date;
    w_date3      date;
    w_date99     date;
    w_date98     date;
    w_date_mon   date;
    w_mon_yobi_kbn   char(1);
    w_num        number(3);
    w_num2       number(3);
    w_num3       number(3);
    w_num4       number(3);
    w_year       char(4);
    w_year_c     char(4);
    w_youbi_c    char(1);
    w_youbi_m    number(1);
    w_youbi_m2   number(1);
    w_week_c1    char(1);
    w_week_c2    char(2);
    w_week_m     number(2);
    w_matsu1     date;
    w_matsu2     date;
    w_sysdate    date;
    w_wrkdate    date;
    w_wrkchr     char(4);
    w_today      char(8);
    w_month      char(2);

-- 年始の最初の月曜日を１週目になるロジック
-- ２次の時はこちらを使用
---------------------------------------------
-- function get_week_no
---------------------------------------------
function get_week_no(変位日１ in date) return char is 年度週ＮＯ char(6);

begin

    w_num  := to_number(to_char(変位日１,'DDD'),'999'); /*当日通算日*/

    w_date := 変位日１; /*当日*/

    syori_youbi_kbn := to_char(w_date,'D') - 1; /*当日曜日*/
    IF syori_youbi_kbn = '0' THEN
       syori_youbi_kbn := '7';
    END IF;

    w_year := to_char(w_date,'YYYY'); /*当年*/

    w_matsu1 := to_date( w_year - 1 || '1231','YYYYMMDD'); /*前年末日*/

    matsu_youbi_kbn := to_char(w_matsu1,'D') - 1; /*前年末日曜日*/
    IF matsu_youbi_kbn = '0' THEN
       matsu_youbi_kbn := '7';
    END IF;

    w_date2 := w_matsu1 - matsu_youbi_kbn + 1; /*前年最終月曜日*/

    w_date99 := w_date2 + 7; /*当年最初月曜日*/

    w_num2 := to_number(to_char(w_date99,'DDD'),'999'); /*当年最初月曜日通算日*/

    /*当日通算日と当年最初月曜日を比較*/

    IF w_num > w_num2 THEN  /*当日通算日が大きい場合*/

       IF syori_youbi_kbn = '1' THEN /*次式でceil関数にて週Noを算出するが*/
            w_num := w_num + 1;      /*切り上げのため2週目以降の月曜日が*/
       END IF;                       /*前週になってしまうの防ぐ*/

       w_week_m := ceil( (w_num - w_num2) / 7 ); /*週No算出*/

    ELSIF w_num < w_num2 THEN  /*当日通算日が小さい場合*/

       w_matsu2 := to_date(w_year - 2 || '1231','YYYYMMDD'); /*前々年末日*/

       matsu_youbi_kbn := to_char(w_matsu2,'D') - 1; /*前々年末日曜日*/
       IF matsu_youbi_kbn = '0' THEN
          matsu_youbi_kbn := '7';
       END IF;

       w_date3 := w_matsu2 - matsu_youbi_kbn + 1; /*前々年最終月曜日*/

       w_date98 := w_date3 + 7; /*前年最初月曜日*/

       w_num3 := to_number(to_char(w_date98,'DDD'),'999'); /*前年最初月曜日通算日*/

       w_num4 := to_number(to_char(to_date(w_year - 1 || '1231','YYYYMMDD'),'DDD'),'999');
       /*前年末日の通算日*/

       /*w_num  := w_num4 - w_num3 + w_num;*/
       w_num  := w_num4 - ( w_num3 - 1 ) + w_num; /*前年末通算日-前年最初月曜通算日+当日通算日 2001/08/30 add K.Yamashiro*/

       w_week_m := ceil( (w_num) / 7 );

       w_week_m := w_week_m - 1;
    ELSE /*当年最初月曜日当日の場合*/

       w_week_m := 1;

    END IF;

    w_month := to_char(w_date,'MM'); /*当月*/

    IF w_week_m <= 9  THEN
       w_week_c1 := to_char(w_week_m);
       年度週ＮＯ := w_year || '0' || w_week_c1;
    ELSE
       w_week_c2 := to_char(w_week_m);
       年度週ＮＯ := w_year || w_week_c2;

       IF w_week_m >= 50 and w_month = '01' THEN
          年度週ＮＯ := w_year - 1 || w_week_c2 + 1;
       END IF;

    END IF;

    return 年度週ＮＯ;

end get_week_no;


BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNADPL010 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_cnt := 0;

---------------------------------------------
-- メイン処理
--   販計日付コントロール更新処理（sxpub1001.csh XPUBOM001.sql）をそのまま流用
---------------------------------------------

  日付変位数 := 0;

  -------------------------------
  -- 当日日付（前日の翌日）の取得
  -------------------------------
  SELECT TO_CHAR(変位日１,'YYYYMMDD') INTO w_today
  FROM
      HNT_日付コントロール
  WHERE
      日付区分 = '0'
  AND 日付変位数 = 1
  ;

  dbms_output.put_line('DT_CLS = 0, 日付変位数 = 1 '||w_today);
  wk_sysdate := to_date(substr(w_today,3,6),'RRMMDD');  /* 2000年対応 */

  -------------------------------
  -- DELETE
  -------------------------------
  DELETE FROM HNT_日付コントロール;
  --***************************************************************************
  -- 日別コントロール
  --***************************************************************************
  for 日付変位数 in -800..800 loop
    ----------------------
    -- 日付1（当日）の取得
    ----------------------
    変位日１ :=  wk_sysdate + 日付変位数;

    ----------------------
    -- 日付2（翌日）の取得
    ----------------------
    変位日２ := wk_sysdate + (日付変位数 + 1);

    -----------------
    -- 曜日区分の取得
    -----------------
    曜日区分 := to_char(wk_sysdate + 日付変位数, 'D') - 1;

    IF 曜日区分 = '0' THEN
       曜日区分 := '7';
    END IF;

    --------------
    -- 週No.の取得
    --------------

    年度週ＮＯ := get_week_no(変位日１);
    ------------------------------
    -- 販計日付コントロールDB への挿入
    ------------------------------
    insert into HNT_日付コントロール
         values('0',
                日付変位数,
                変位日１,
                変位日２,
                年度週ＮＯ,
                曜日区分);
  end loop;

  --***************************************************************************
  -- 週別コントロール
  --***************************************************************************

  for 日付変位数 in -116..116 loop
    --------------------------
    -- 日付1（当週月曜）の取得
    --------------------------
    w_mon_yobi_kbn := to_char(wk_sysdate,'D') - 1; /*当日曜日*/
    IF w_mon_yobi_kbn = '0' THEN
       w_mon_yobi_kbn := '7';
    END IF;

    w_date_mon := wk_sysdate - ( w_mon_yobi_kbn - 1 );
    変位日１ := w_date_mon + (日付変位数 * 7);
    /*変位日１ := next_day(trunc(w_date_mon + (日付変位数 * 7 - 1), 'DY'), '月');*/

    --------------------------
    -- 日付2（当週日曜）の取得
    --------------------------
    変位日２ := 変位日１ + 6;
    /*変位日２ := next_day(trunc(wk_sysdate + (日付変位数 * 7 - 1), 'DY'), '日');*/

    -----------------
    -- 曜日区分の取得
    -----------------
    曜日区分 := '1';

    --------------
    -- 週No.の取得
    --------------
    年度週ＮＯ := get_week_no(変位日１);

    ------------------------------
    -- 販計日付コントロールDB への挿入
    ------------------------------
    insert into HNT_日付コントロール
         values('1',
                日付変位数,
                変位日１,
                変位日２,
                年度週ＮＯ,
                曜日区分);
  end loop;

  --***************************************************************************
  -- 月別コントロール
  --***************************************************************************
  for 日付変位数 in -26..26 loop
    ---------------------------
    -- 日付1（当月 1 日）の取得
    ---------------------------
    変位日１ := add_months(trunc(wk_sysdate, 'MM'), 日付変位数);

    --------------------------
    -- 日付2（当月末日）の取得
    --------------------------
    変位日２ := last_day(add_months(trunc(wk_sysdate, 'MM'), 日付変位数));

    -----------------
    -- 曜日区分の取得
    -----------------
    曜日区分 := to_char(add_months(trunc(wk_sysdate, 'MM'), 日付変位数), 'D') - 1;

    IF 曜日区分 = '0' THEN
       曜日区分 := '7';
    END IF;

    --------------
    -- 週No.の取得
    --------------
    年度週ＮＯ := get_week_no(変位日１);

    ------------------------------
    -- 販計日付コントロールDB への挿入
    ------------------------------
    insert into HNT_日付コントロール
         values('2',
                日付変位数,
                変位日１,
                変位日２,
                年度週ＮＯ,
                曜日区分);
 end loop;

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNADPL010  END  : ' || w_time_end);

    -- コミット
    commit;

---------------------------------------------
-- 例外処理
---------------------------------------------
EXCEPTION
    when no_data_found then
-- ロールバック
        rollback;
        :RtnCd  :=  sqlcode;
        :ErrMsg := substr(sqlerrm,1,100);
-- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL010 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL010 RETURN CODE :' || :RtnCd);
    when others then
-- ロールバック
        rollback;
        :RtnCd  :=  sqlcode;
        :ErrMsg := substr(sqlerrm,1,100);
-- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL010 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL010 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
