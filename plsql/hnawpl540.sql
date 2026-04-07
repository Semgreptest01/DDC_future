/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：計画終了対象商品週次帳票作成
/*　　　ＰＧＭID　　　　：hnawpl540.sql
/*　　　機能　　　　　　：HNT計画終了対象商品＿週次から
/*　　　　　　　　　　　：利用者コード、センターコード、ＤＥＰＴコード、
/*　　　　　　　　　　　：ＣＬＡＳＳコード、商品コード、店案内年度週ＮＯ、
/*　　　　　　　　　　　：計画終了有効開始日、推奨取消日
/*　　　　　　　　　　　：の重複を削除して
/*　　　　　　　　　　　：HNT_計画終了対象商品＿週次帳票へ登録する。
/*　　　　　　　　　　　：
/*　　　インプット　　　：HNT_計画終了対象商品＿週次
/*　　　　　　　　　　　：HNT_処理日付コントロール
/*　　　　　　　　　　　：HNT_日付コントロール
/*　　　　　　　　　　　：
/*　　　アウトプット　　：HNT_計画終了対象商品＿週次帳票
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：Hirata
/*　　　作成日　　　　　：2009/03/11
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

    -- プログラム内変数
    w_ins_cnt    number(10);    -- INSERT件数
    w_shori_dt   date;          -- 処理実施日

    -- 結果格納変数
    w_利用者コード       CHAR(7);
    w_センターコード     CHAR(6);
    w_ＤＥＰＴコード     CHAR(2);
    w_ＣＬＡＳＳコード   CHAR(3);
    w_商品コード         CHAR(6);
    w_店案内年度週ＮＯ   CHAR(6);
    w_計画終了有効開始日 DATE;
    w_推奨取消日         DATE;
    w_商品部エリアコード CHAR(2);
    w_商品部地域コード   CHAR(2);

    --カーソルの定義
    CURSOR HNT033_CUR IS
        SELECT
            A.商品部地域コード,
            A.利用者コード,
            A.ベンダーコード,
            A.商品コード,
            A.店案内年度週ＮＯ,
            A.商品部エリアコード,
            A.商品部エリア名,
            A.商品部地域名,
            A.くくりＶＤＲコード,
            A.くくりＶＤＲ名,
            A.ベンダー名,
            A.コピー宛先１コード,
            A.宛先コード,
            A.宛先名,
            A.センターコード,
            A.センター名,
            A.ＤＥＰＴコード,
            A.ＤＥＰＴ名,
            A.ＣＬＡＳＳコード,
            A.ＣＬＡＳＳ名,
            A.商品名,
            A.計画終了有効開始日,
            A.推奨取消日,
            B.変位日１ - 13 AS 店案内日
        FROM
            HNT_計画終了対象商品＿週次 A,
            HNT_日付コントロール B
        WHERE
            B.日付区分 = '1'
            AND B.年度週ＮＯ = A.店案内年度週ＮＯ
        ORDER BY
            A.利用者コード,
            A.センターコード,
            A.ＤＥＰＴコード,
            A.ＣＬＡＳＳコード,
            A.商品コード,
            A.店案内年度週ＮＯ,
            A.計画終了有効開始日,
            A.推奨取消日,
            A.商品部エリアコード,
            A.商品部地域コード
    ;

BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNAWPL540 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_ins_cnt := 0;

    w_利用者コード       := NULL;
    w_センターコード     := NULL;
    w_ＤＥＰＴコード     := NULL;
    w_ＣＬＡＳＳコード   := NULL;
    w_商品コード         := NULL;
    w_店案内年度週ＮＯ   := NULL;
    w_計画終了有効開始日 := NULL;
    w_推奨取消日         := NULL;
    w_商品部エリアコード := NULL;
    w_商品部地域コード   := NULL;

---------------------------------------------
-- HNT_計画終了対象商品＿週次帳票作成
---------------------------------------------

    -- 処理実施日を取得
    SELECT
        処理実施日
        into
        w_shori_dt
    FROM
        HNT_処理日付コントロール
    ;

    FOR HNT033_REC IN HNT033_CUR LOOP
        IF
            w_利用者コード                IS NULL OR
            HNT033_REC.利用者コード       <> w_利用者コード OR
            HNT033_REC.センターコード     <> w_センターコード OR
            HNT033_REC.ＤＥＰＴコード     <> w_ＤＥＰＴコード OR
            HNT033_REC.ＣＬＡＳＳコード   <> w_ＣＬＡＳＳコード OR
            HNT033_REC.商品コード         <> w_商品コード OR
            HNT033_REC.店案内年度週ＮＯ   <> w_店案内年度週ＮＯ OR
            HNT033_REC.計画終了有効開始日 <> w_計画終了有効開始日 OR
            HNT033_REC.推奨取消日         <> w_推奨取消日
        THEN
            w_利用者コード       := HNT033_REC.利用者コード;
            w_センターコード     := HNT033_REC.センターコード;
            w_ＤＥＰＴコード     := HNT033_REC.ＤＥＰＴコード;
            w_ＣＬＡＳＳコード   := HNT033_REC.ＣＬＡＳＳコード;
            w_商品コード         := HNT033_REC.商品コード;
            w_店案内年度週ＮＯ   := HNT033_REC.店案内年度週ＮＯ;
            w_計画終了有効開始日 := HNT033_REC.計画終了有効開始日;
            w_推奨取消日         := HNT033_REC.推奨取消日;
            
            INSERT INTO HNT_計画終了対象商品＿週次帳票 (
                商品部地域コード,
                利用者コード,
                ベンダーコード,
                商品コード,
                店案内年度週ＮＯ,
                商品部エリアコード,
                商品部エリア名,
                商品部地域名,
                くくりＶＤＲコード,
                くくりＶＤＲ名,
                ベンダー名,
                コピー宛先１コード,
                宛先コード,
                宛先名,
                センターコード,
                センター名,
                ＤＥＰＴコード,
                ＤＥＰＴ名,
                ＣＬＡＳＳコード,
                ＣＬＡＳＳ名,
                商品名,
                計画終了有効開始日,
                推奨取消日,
                店案内日,
                作成日時
            ) VALUES (
                HNT033_REC.商品部地域コード,
                HNT033_REC.利用者コード,
                HNT033_REC.ベンダーコード,
                HNT033_REC.商品コード,
                HNT033_REC.店案内年度週ＮＯ,
                HNT033_REC.商品部エリアコード,
                HNT033_REC.商品部エリア名,
                HNT033_REC.商品部地域名,
                HNT033_REC.くくりＶＤＲコード,
                HNT033_REC.くくりＶＤＲ名,
                HNT033_REC.ベンダー名,
                HNT033_REC.コピー宛先１コード,
                HNT033_REC.宛先コード,
                HNT033_REC.宛先名,
                HNT033_REC.センターコード,
                HNT033_REC.センター名,
                HNT033_REC.ＤＥＰＴコード,
                HNT033_REC.ＤＥＰＴ名,
                HNT033_REC.ＣＬＡＳＳコード,
                HNT033_REC.ＣＬＡＳＳ名,
                HNT033_REC.商品名,
                HNT033_REC.計画終了有効開始日,
                HNT033_REC.推奨取消日,
                HNT033_REC.店案内日,
                w_shori_dt
            );
            -- INSERT件数カウント
            w_ins_cnt := w_ins_cnt + sql%rowcount;
        END IF;
    END  LOOP;

    dbms_output.put_line('[HNT037][INSERT] : ' || w_ins_cnt ||  '件');

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNAWPL540  END  : ' || w_time_end);

    -- コミット
    commit;

---------------------------------------------
-- 例外処理
---------------------------------------------
EXCEPTION
-- それ以外のエラー
    when others then
-- ロールバック
        rollback;
-- 戻り値のセット
        :RtnCd  := sqlcode;
-- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
-- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNAWPL540 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNAWPL540 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
