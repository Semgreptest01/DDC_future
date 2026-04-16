/********************************************************************************/
/*      システム名      ：ＤＤＣ返品コスト削減
/*      プログラム名    ：計画終了対象商品週次リラン用削除
/*      プログラムID    ：hnawpl470.sql
/*      機能            ：リラン用にHNT_計画終了対象商品_週次の
/*                        店案内年度週ＮＯが翌々週のデータを削除する。
/*      インプット      ：(HNT001) HNT_日付コントロール
/*                      ：(HNT002) HNT_処理日付コントロール
/*                      ：(HNT033) HNT_計画終了対象商品＿週次
/*      アウトプット    ：(HNT033) HNT_計画終了対象商品＿週次
/*                      ：
/*      作成者          ：G.SASAKI
/*      作成日          ：2010/03/15
/*      修正履歴        ：YYYY/MM/DD xxxxx
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
    w_time_start        varchar2(19);  -- プログラム開始日時
    w_time_end          varchar2(19);  -- プログラム終了日時

    w_cnt               number(10);    -- DELETE件数

    w_処理実施日        date;
    w_年度週ＮＯ        char(06);

BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNAWPL470 START : ' || w_time_start);
    :RtnCd  := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_cnt   := 0 ;

---------------------------------------------
-- (HNT033) HNT_計画終了対象商品＿週次(リラン用)
---------------------------------------------

    /* 処理日付コントロール取得 */
    SELECT
            処理実施日
    INTO
            w_処理実施日
    FROM
            HNT_処理日付コントロール
    ;
    /* 処理実施日（火曜）から見て翌々週の案内週を取得する */
    SELECT
        A.年度週ＮＯ
    INTO
        w_年度週ＮＯ
    FROM
        (
            SELECT
                ROWNUM  週番号,
                年度週ＮＯ
            FROM
                HNT_日付コントロール
            WHERE
                変位日１ > (w_処理実施日 -1)
            AND
                日付区分 = '1'
            ORDER BY
                変位日１
        ) A
    WHERE A.週番号 = 2
    ;

    /* HNT_計画終了対象商品＿週次 データ削除(リラン用) */

    DELETE
    FROM
            HNT_計画終了対象商品＿週次
    WHERE   店案内年度週ＮＯ  = w_年度週ＮＯ
    ;

    /* 削除件数表示 */
    w_cnt := sql%rowcount;
    dbms_output.put_line('[削除対象 年度週ＮＯ] : ' || w_年度週ＮＯ);
    dbms_output.put_line('[HNT033][DELETE] : ' || w_cnt ||  '件');

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNAWPL470  END  : ' || w_time_end);

    -- コミット
    commit;

---------------------------------------------
-- 例外処理
---------------------------------------------
EXCEPTION
    /* 一意制約違反 */
    when DUP_VAL_ON_INDEX then
    -- ロールバック
        rollback;
    -- 戻り値のセット
        :RtnCd  :=  sqlcode;
    -- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
    -- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNAWPL470 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNAWPL470 RETURN CODE :' || :RtnCd);

    /* 一意制約違反以外のエラー */
    when others then
    -- ロールバック
        rollback;
    -- 戻り値のセット
        :RtnCd  :=  sqlcode;
    -- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
    -- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNAWPL470 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNAWPL470 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
