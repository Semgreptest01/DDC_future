/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：在庫切れ速報削除
/*　　　ＰＧＭID　　　　：hnampl500.sql (HNAMPL500)
/*　　　機能　　　　　　：処理実施日の月から14ヶ月以前のデータ全件を削除する
/*　　　　　　　　　　　　（保存期間は13ヶ月)
/*　　　インプット　　　：HNT_在庫切れ速報
/*　　　　　　　　　　　：HNT_処理日付コントロール 
/*　　　　　　　　　　　：
/*　　　アウトプット　　：HNT_在庫切れ速報
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：VIXUS)G.Saito
/*　　　作成日　　　　　：2010/03/16
/*　　　修正履歴　　　　：
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

    w_cnt        number(10);    -- 処理件数
    w_kijun_day  date;
BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNAMPL500 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_cnt := 0;

---------------------------------------------
-- 在庫切れ速報削除
---------------------------------------------

    -- 削除基準月の末日を取得
    SELECT LAST_DAY(ADD_MONTHS(処理実施日, -14)) into w_kijun_day
      FROM HNT_処理日付コントロール;

    dbms_output.put_line('[削除基準月の末日] : ' || w_kijun_day);

    -- 削除基準月の末日以前のデータを削除

    delete from HNT_在庫切れ速報
      where 作成日時 <= w_kijun_day;

    w_cnt := sql%rowcount;
    dbms_output.put_line('[HNT034][DELETE] : ' || w_cnt ||  '件');


---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNAMPL500  END  : ' || w_time_end);

    -- コミット
    commit;

---------------------------------------------
-- 例外処理
---------------------------------------------
EXCEPTION
    when others then
-- ロールバック
        rollback;
-- 戻り値のセット
        :RtnCd  :=  sqlcode;
-- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
-- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNAMPL500 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNAMPL500 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
