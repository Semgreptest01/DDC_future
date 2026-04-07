/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：宛先抽出
/*　　　ＰＧＭID　　　　：hnadpl030.sql
/*　　　機能　　　　　　：HNW_宛先を更新する
/*　　　インプット　　　：HNT_処理日付コントロール
/*　　　　　　　　　　　：HNT_ベンダー
/*　　　　　　　　　　　：HNT_新オンライン宛先ベンダー
/*　　　　　　　　　　　：HNT_新オンライン宛先基本情報
/*　　　　　　　　　　　：HNP_返品削減宛先パラメータ
/*　　　アウトプット　　：HNW_宛先
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：G.Saito
/*　　　作成日　　　　　：2009/09/29
/*　　　修正履歴　　　　：2025/11/19 LMDC移行対応 Oracle非互換（非推奨）対応
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

    w_処理実施日   date;
    w_年度週ＮＯ   char(6);

BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNADPL030 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_cnt := 0;

---------------------------------------------
-- メイン処理
---------------------------------------------

    /* 処理日付取得 */
    SELECT 処理実施日, 年度週ＮＯ INTO w_処理実施日, w_年度週ＮＯ FROM HNT_処理日付コントロール;

    /* HNW_宛先更新 */
    INSERT  INTO
        HNW_宛先
    SELECT
        NV.宛先コード,
        NA.宛先名,
        VD.ベンダーコード,
        VD.ベンダー名
-- 2025/11/19 VINX 【LMDC移行対応 Oracle非互換（非推奨）対応 start
    FROM
            (
            SELECT * FROM  HNT_ベンダー
            WHERE
                w_処理実施日  BETWEEN  有効開始日  AND  有効終了日
            AND 処理区分 < '07' 
            ) VD   
        LEFT JOIN
        (
            SELECT OVNA.宛先コード, OVNA.ベンダーコード
            FROM
            (
                SELECT OV.宛先コード, OV.ベンダーコード
                FROM
                (
                SELECT * FROM  HNT_新オンライン宛先ベンダー
                WHERE
                w_処理実施日 BETWEEN 有効開始日 AND 有効終了日
                AND 処理区分 < '07'
                ) OV
                LEFT JOIN
                HNT_新オンライン宛先基本情報  NA
                ON  OV.コピー宛先コード = NA.宛先コード
                AND  w_処理実施日 BETWEEN NA.有効開始日 AND NA.有効終了日
                AND NA.処理区分 < '07'
            )OVNA
            WHERE
            OVNA.宛先コード IN 
            (
                SELECT 宛先コード FROM HNP_返品削減宛先パラメータ
                WHERE  w_年度週ＮＯ BETWEEN 開始年度週ＮＯ AND 終了年度週ＮＯ
            )
            GROUP BY
            OVNA.宛先コード,
            OVNA.ベンダーコード 
        ) NV
    ON VD.ベンダーコード = NV.ベンダーコード
    LEFT JOIN
         (
            SELECT * FROM HNT_新オンライン宛先基本情報
            WHERE
                w_処理実施日 BETWEEN 有効開始日 AND 有効終了日
            AND 処理区分 < '07'
        ) NA    
    ON  NV.宛先コード = NA.宛先コード
    ;
--    FROM
--    (
--        SELECT * FROM  HNT_ベンダー
--        WHERE
--            w_処理実施日  BETWEEN  有効開始日  AND  有効終了日
--        AND 処理区分 < '07' 
--    ) VD,
--    (
--        SELECT 宛先コード, ベンダーコード
--        FROM
--            (
--                SELECT OV.宛先コード, OV.ベンダーコード
--                FROM
--                (
--                    SELECT * FROM  HNT_新オンライン宛先ベンダー
--                    WHERE
--                        w_処理実施日 BETWEEN 有効開始日 AND 有効終了日
--                    AND 処理区分 < '07'
--                ) OV,
--                (
--                    SELECT * FROM HNT_新オンライン宛先基本情報
--                    WHERE  w_処理実施日 BETWEEN 有効開始日 AND 有効終了日
--                    AND 処理区分 < '07'
--                ) NA 
--                WHERE OV.コピー宛先コード = NA.宛先コード(+)
--            )
--        WHERE
--            宛先コード IN 
--            (
--                SELECT 宛先コード FROM HNP_返品削減宛先パラメータ
--                WHERE  w_年度週ＮＯ BETWEEN 開始年度週ＮＯ AND 終了年度週ＮＯ
--            )
--        GROUP BY
--            宛先コード,
--            ベンダーコード 
--    ) NV,
--    (
--        SELECT * FROM HNT_新オンライン宛先基本情報
--        WHERE
--            w_処理実施日 BETWEEN 有効開始日 AND 有効終了日
--        AND 処理区分 < '07'
--    ) NA
--    WHERE
--        VD.ベンダーコード = NV.ベンダーコード (+) 
--    AND
--        NV.宛先コード = NA.宛先コード (+)
--    ;
-- 2025/08/22 VINX 【LMDC移行対応 Oracle非互換（非推奨）対応 end
    /* 更新件数表示 */
    w_cnt := sql%rowcount;
    dbms_output.put_line('[HNW006][INSERT] : ' || w_cnt ||  '件');

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNADPL030  END  : ' || w_time_end);

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
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL030 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL030 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
