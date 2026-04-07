/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：HNT_処理日付コントロール更新処理
/*　　　ＰＧＭID　　　　：hnadpl020.sql
/*　　　機能　　　　　　：HNT_処理日付コントロールを更新する
/*　　　インプット　　　：HNT_日付コントロール
/*　　　　　　　　　　　：HNT_処理日付コントロール 
/*　　　アウトプット　　：HNT_処理日付コントロール 
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：G.Saito
/*　　　作成日　　　　　：2009/09/28
/*　　　修正履歴　　　　：YYYY/MM/DD xxxxx
/*                      ：2010/01.21 VIXUS)G.Saito  緊急カット入力対象商品のカット期間が
/*                                                  処理週を含むよう前倒しに変更
/*                      ：2010/01.27 VIXUS)G.Saito  計画終了案内前倒しにより、計画終了期間
/*                                                  （勝負期間）が延長。
/*                                                  →カット対象終了、発注終了対象が１週間延長
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

    w_処理実施日        date;
    w_カット対象開始    date;
    w_カット対象終了    date;
    w_発注対象抽出開始  date;
    w_発注対象抽出終了  date;
    w_変位日２          date;
    w_年度週ＮＯ        char(6);

    w_前日処理日付      date;

BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNADPL020 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_cnt := 0;

---------------------------------------------
-- メイン処理
---------------------------------------------

    /* 前日処理日付取得 */
    SELECT
        処理実施日
    INTO
        w_前日処理日付
    FROM
        HNT_処理日付コントロール
    ;

    /* 更新項目取得 */
    SELECT
        * 
    INTO
        w_カット対象開始,
        w_カット対象終了,
        w_変位日２,
        w_年度週ＮＯ
    FROM
        (
            SELECT
                変位日１,
                変位日２ +22,
                変位日２,
                年度週ＮＯ
            FROM
                HNT_日付コントロール
            WHERE
                変位日１ > w_前日処理日付
            AND
                日付区分 = '1'
            ORDER BY
                変位日１
        ) A
    WHERE ROWNUM = 1
    ;

    /* 翌日へ */
    w_処理実施日 := w_前日処理日付 +1;

    dbms_output.put_line('[更新後 処理実施日] :' || w_処理実施日);

    /* 緊急カット入力対象の期間抽出 */
    SELECT
        A.変位日１,
        A.変位日２ +21
    INTO
        w_発注対象抽出開始,
        w_発注対象抽出終了
    FROM
        HNT_日付コントロール A,
        (
            SELECT
                MAX(変位日１) 変位日１
            FROM
                HNT_日付コントロール
            WHERE
                日付区分 = '1'
            AND
                変位日１ < w_処理実施日
        ) B
    WHERE
        A.日付区分 = '1'
    AND
        A.変位日１ = B.変位日１
    ;

    /* ＤＢ更新 */
    UPDATE
        HNT_処理日付コントロール
    SET
        処理実施日 = w_処理実施日,
        カット対象開始 = w_カット対象開始,
        カット対象終了 = w_カット対象終了,
        発注対象抽出開始 = w_発注対象抽出開始,
        発注対象抽出終了 = w_発注対象抽出終了,
        変位日２ = w_変位日２,
        年度週ＮＯ = w_年度週ＮＯ
    ;

    /* 更新件数表示 */
    w_cnt := sql%rowcount;
    dbms_output.put_line('[HNT002][UPDATE] : ' || w_cnt ||  '件');

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNADPL020  END  : ' || w_time_end);

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
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL020 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL020 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
