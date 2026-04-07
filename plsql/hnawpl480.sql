/********************************************************************************/
/*      システム名      ：ＤＤＣ返品コスト削減
/*      プログラム名    ：計画終了対象商品週次作成
/*      プログラムID    ：hnawpl480.sql
/*      機能            ：HNW_計画終了対象商品名称付から
/*                      ：HNT_計画終了対象商品_週次を作成する。
/*      インプット      ：(HNT001) HNT_日付コントロール
/*                      ：(HNT002) HNT_処理日付コントロール
/*                      ：(HNW021) HNW_計画終了対象商品名称付
/*      アウトプット    ：(HNT033) HNT_計画終了対象商品＿週次
/*                      ：
/*      作成者          ：G.Sasaki
/*      作成日          ：2010/03/25
/*      修正履歴        ：2022/03/08 計画終了システム改善対応
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

    w_cnt               number(10);    -- INSERT件数

    w_年度週ＮＯ        char(06);      -- 年度週ＮＯ
    w_処理実施日        date;          -- 処理実施日
    w_カット対象開始    date;          -- カット対象開始


BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNAWPL480 START : ' || w_time_start);
    :RtnCd  := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_cnt   := 0 ;

---------------------------------------------
-- (HNT033) HNT_計画終了対象商品＿週次 作成
---------------------------------------------

    /* 処理日付コントロール取得 */
    SELECT
            処理実施日,
            カット対象開始
    INTO
            w_処理実施日,
            w_カット対象開始
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

    /* HNT_計画終了対象商品＿週次 作成 */
    INSERT INTO 
            HNT_計画終了対象商品＿週次
    SELECT
            KEI.商品部地域コード,
            KEI.利用者コード,
            KEI.ベンダーコード,
            KEI.商品コード,
            KEI.店案内年度週ＮＯ,
            KEI.商品部エリアコード,
            KEI.商品部エリア名,
            KEI.商品部地域名,
            KEI.くくりＶＤＲコード,
            KEI.くくりＶＤＲ名,
            KEI.ベンダー名,
            KEI.コピー宛先１コード,
            KEI.宛先コード,
            KEI.宛先名,
            KEI.センターコード,
            KEI.センター名,
            KEI.ＤＥＰＴコード,
            KEI.ＤＥＰＴ名,
            KEI.ＣＬＡＳＳコード,
            KEI.ＣＬＡＳＳ名,
            KEI.商品名,
            KEI.計画終了有効開始日,
            KEI.推奨取消日,
            KEI.不可日設定日,
            w_処理実施日 AS 作成日時
    FROM
            HNW_計画終了対象商品名称付 KEI
    WHERE
    -- MODIFY 2022/03/08 START
    -- 翌々週の案内週以前でかつ、HNT_計画終了対象商品＿週次に無い
    -- 計画終了商品をインサートする
    --        KEI.店案内年度週ＮＯ = w_年度週ＮＯ
    --AND
    --        KEI.推奨取消日 > w_カット対象開始
            KEI.店案内年度週ＮＯ <= w_年度週ＮＯ
    AND NOT EXISTS (
            SELECT 1 FROM HNT_計画終了対象商品＿週次 SYU
            -- キー項目で結合
            WHERE KEI.商品部地域コード = SYU.商品部地域コード
            AND KEI.利用者コード       = SYU.利用者コード
            AND KEI.ベンダーコード     = SYU.ベンダーコード
            AND KEI.商品コード         = SYU.商品コード
            AND KEI.店案内年度週ＮＯ   = SYU.店案内年度週ＮＯ
            )
    -- MODIFY 2022/03/08 END
    ;

    /* 挿入件数表示 */
    w_cnt := sql%rowcount;
    dbms_output.put_line('[HNT033][INSERT] : ' || w_cnt ||  '件');

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNAWPL480  END  : ' || w_time_end);

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
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNAWPL480 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNAWPL480 RETURN CODE :' || :RtnCd);

    /* 一意制約違反以外のエラー */
    when others then
    -- ロールバック
        rollback;
    -- 戻り値のセット
        :RtnCd  :=  sqlcode;
    -- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
    -- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNAWPL480 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNAWPL480 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
