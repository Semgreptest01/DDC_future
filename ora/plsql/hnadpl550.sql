/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：アドレスマスタ不要データ削除
/*　　　ＰＧＭID　　　　：hnadpl550.sql (HNADPL550)
/*　　　機能　　　　　　：HNT_アドレスマスタの不要データを削除する
/*　　　インプット　　　：HNT_アドレスマスタ
/*　　　　　　　　　　　：HNT_処理日付コントロール 
/*　　　　　　　　　　　：
/*　　　アウトプット　　：HNT_アドレスマスタ
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：S.Oyama
/*　　　作成日　　　　　：2013/07/30
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
    dbms_output.put_line('**** HNADPL550 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_cnt := 0;

---------------------------------------------
-- HNT_アドレスマスタの不要データ削除
---------------------------------------------

    -- HNT_アドレスマスタの有効終了日が
    -- 処理実施日（当日）よりも過去のレコードを削除する。

    DELETE FROM HNT_アドレスマスタ A
    WHERE exists (
          SELECT 'X'
          FROM HNT_処理日付コントロール B
          WHERE A.有効終了日 < B.処理実施日
          );

    w_cnt := sql%rowcount;
    dbms_output.put_line('[HNT038][DELETE] : ' || w_cnt ||  '件');


---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNADPL550  END  : ' || w_time_end);

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
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL550 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL550 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
