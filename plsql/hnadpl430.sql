/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：計画終了対象商品名称付加
/*　　　ＰＧＭID　　　　：hnadpl430.sql
/*　　　機能　　　　　　：HNT_計画終了対象商品の各コードより名称を付加して
/*　　　　　　　　　　　：HNW_計画終了対象商品名称付に追加する。
/*　　　　　　　　　　　：
/*　　　インプット　　　：HNT_計画終了対象商品
/*　　　　　　　　　　　：HNW_計画終了対象商品名称付
/*　　　　　　　　　　　：HNT_ＣＬＡＳＳ
/*　　　　　　　　　　　：HNT_処理日付コントロール
/*　　　　　　　　　　　：HNT_商品部エリア
/*　　　　　　　　　　　：HNT_商品部地域
/*　　　　　　　　　　　：HNT_ベンダー
/*　　　　　　　　　　　：HNT_ＤＥＰＴ
/*　　　　　　　　　　　：HNT_社別商品
/*　　　　　　　　　　　：HNW_宛先
/*　　　　　　　　　　　：
/*　　　アウトプット　　：HNW_計画終了対象商品名称付
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：H.Nagata
/*　　　作成日　　　　　：2010/03/10
/*　　　修正履歴　　　　：YYYY/MM/DD xxxxx
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

    w_insert_cnt number(10);    -- INSERT件数

    w_処理実施日 date;          -- 処理実施日

BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNADPL430 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_insert_cnt := 0 ;

---------------------------------------------
-- (HNW021) HNW_計画終了対象商品名称付追加
---------------------------------------------

    -- 処理実施日を取得
    SELECT 処理実施日 into w_処理実施日 FROM HNT_処理日付コントロール ;


    /* HNW_計画終了対象商品名称付追加 */
    INSERT INTO HNW_計画終了対象商品名称付

    SELECT  
            A.商品部地域コード,
            A.利用者コード,
            A.ベンダーコード,
            A.商品コード,
            A.店案内年度週ＮＯ,
            A.商品部エリアコード,
            C.商品部エリア名,
            D.商品部地域名,
            A.くくりベンダーコード      AS くくりＶＤＲコード,
            E.ベンダー名                AS くくりＶＤＲ名,
            F.ベンダー名,
            A.コピー宛先コード１        AS コピー宛先１コード,
            A.宛先コード,
            G.宛先名,
            A.センターコード,
            H.ベンダー名                AS センター名,
            A.ＤＥＰＴコード,
            I.ＤＥＰＴ名,
            A.ＣＬＡＳＳコード,
            J.ＣＬＡＳＳ名,
            K.商品名,
            A.計画終了有効開始日,
            A.推奨取消日,
            A.不可日設定日,
            B.処理実施日                AS 作成日時
    FROM
    HNT_計画終了対象商品 A,
    HNT_処理日付コントロール B,
    (
        SELECT
                商品部エリアコード,
                商品部エリア名
        FROM
                HNT_商品部エリア
        WHERE   処理区分              < '07'
     ) C,
    (
        SELECT
                商品部地域コード,
                商品部地域名
        FROM
                HNT_商品部地域
        WHERE   処理区分              < '07'
     ) D,
    (
        SELECT  --くくりＶＤＲ名抽出用
                ベンダーコード,
                ベンダー名
        FROM    HNT_ベンダー
        WHERE   有効開始日           <= w_処理実施日
          AND   有効終了日           >= w_処理実施日
     ) E,
    (
        SELECT  --ベンダー名抽出用
                ベンダーコード,
                ベンダー名
        FROM    HNT_ベンダー
        WHERE   有効開始日           <= w_処理実施日
          AND   有効終了日           >= w_処理実施日
     ) F,
     HNW_宛先 G,
    (
        SELECT  --センター名抽出用
                ベンダーコード,
                ベンダー名
        FROM    HNT_ベンダー
        WHERE   有効開始日           <= w_処理実施日
          AND   有効終了日           >= w_処理実施日
     ) H,
    (
        SELECT
                ＤＥＰＴコード,
                ＤＥＰＴ名
        FROM
                HNT_ＤＥＰＴ
        WHERE   社コード              = '81'
          AND   有効開始日           <= w_処理実施日
          AND   有効終了日           >= w_処理実施日
     ) I,
    (
        SELECT
                ＣＬＡＳＳコード,
                ＤＥＰＴコード,
                ＣＬＡＳＳ名
        FROM
                HNT_ＣＬＡＳＳ
        WHERE   社コード              = '81'
          AND   有効開始日           <= w_処理実施日
          AND   有効終了日           >= w_処理実施日
     ) J,
    (
        SELECT
                商品コード,
                商品名
        FROM    HNT_社別商品
        WHERE   社コード              = '81'
          AND   有効開始日           <= w_処理実施日
          AND   有効終了日           >= w_処理実施日
     ) K
    WHERE   A.商品部エリアコード    = C.商品部エリアコード
      AND   A.商品部地域コード      = D.商品部地域コード
      AND   A.くくりベンダーコード  = E.ベンダーコード
      AND   A.ベンダーコード        = F.ベンダーコード
      AND   A.ベンダーコード        = G.ベンダーコード
      AND   A.宛先コード            = G.宛先コード
      AND   A.センターコード        = H.ベンダーコード
      AND   A.ＤＥＰＴコード        = I.ＤＥＰＴコード
      AND   A.ＤＥＰＴコード        = J.ＤＥＰＴコード
      AND   A.ＣＬＡＳＳコード      = J.ＣＬＡＳＳコード
      AND   A.商品コード            = K.商品コード
    ;



    -- 処理件数出力
    w_insert_cnt := sql%rowcount;
    dbms_output.put_line('[HNW021][INSERT] : ' || w_insert_cnt ||  '件') ;

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNADPL430  END  : ' || w_time_end);

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
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL430 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL430 RETURN CODE :' || :RtnCd);

    /* 一意制約違反以外のエラー */
    when others then
    -- ロールバック
        rollback;
    -- 戻り値のセット
        :RtnCd  :=  sqlcode;
    -- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
    -- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL430 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL430 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
