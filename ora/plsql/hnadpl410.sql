/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：在庫切れ速報作成
/*　　　ＰＧＭID　　　　：hnadpl410.sql
/*　　　機能　　　　　　：HNT_受発注在庫切れ情報の各コードより名称を付加して
/*　　　　　　　　　　　：HNT_在庫切れ速報に追加する。
/*　　　　　　　　　　　：
/*　　　インプット　　　：HNT_受発注在庫切れ情報
/*　　　　　　　　　　　：HNT_在庫切れ速報
/*　　　　　　　　　　　：HNT_ＣＬＡＳＳ
/*　　　　　　　　　　　：HNT_処理日付コントロール
/*　　　　　　　　　　　：HNT_ベンダー納品エリア
/*　　　　　　　　　　　：HNT_納品エリア
/*　　　　　　　　　　　：HNT_商品部エリア
/*　　　　　　　　　　　：HNT_商品部地域
/*　　　　　　　　　　　：HNT_ベンダー
/*　　　　　　　　　　　：HNT_ＤＥＰＴ
/*　　　　　　　　　　　：HNT_社別商品
/*　　　　　　　　　　　：
/*　　　アウトプット　　：HNT_在庫切れ速報
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：H.Nagata
/*　　　作成日　　　　　：2010/03/15
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
    dbms_output.put_line('**** hnadpl410 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_insert_cnt := 0 ;

---------------------------------------------
-- (HNT034) HNT_在庫切れ速報作成
---------------------------------------------

    -- 処理実施日を取得
    SELECT 処理実施日 into w_処理実施日 FROM HNT_処理日付コントロール ;


    /* HNT_在庫切れ速報作成 */
    INSERT INTO HNT_在庫切れ速報

    SELECT  
            A.在庫切速報配信年月日      AS 発生日,
            C.商品部地域コード,
            A.利用者コード,
            A.在庫場所コード,
            A.配送ベンダコード          AS センターコード,
            A.製造ベンダコード          AS ベンダーコード,
            A.商品コード,
            A.サイクルコード,
            C.商品部エリアコード,
            D.商品部エリア名,
            E.商品部地域名,
            A.製造ベンダ名称            AS ベンダー名,
            A.配送ベンダ名称            AS センター名,
            F.ＤＥＰＴコード,
            G.ＤＥＰＴ名,
            F.ＣＬＡＳＳコード,
            H.ＣＬＡＳＳ名,
            F.商品名,
            A.在庫数量                  AS 当日在庫数,
            A.合計在庫引当数            AS 当日引当発注数,
            A.在庫抽選期間開始年月日    AS 計画終了有効開始日,
            A.在庫抽選期間終了年月日    AS 取消日,
            B.処理実施日                AS 作成日時
    FROM
    HNT_受発注在庫切れ情報 A,
    HNT_処理日付コントロール B,
    (
        SELECT  distinct
                sa.ベンダーコード,
                sb.商品部エリアコード,
                sb.商品部地域コード
        FROM
                HNT_ベンダー納品エリア sa,
                HNT_納品エリア         sb
                
        WHERE   sa.納品エリアコード  = sb.納品エリアコード
          AND   sa.処理区分          < '07'
          AND   sa.有効開始日        <= w_処理実施日
          AND   sa.有効終了日        >= w_処理実施日
          AND   sb.処理区分          < '07'
          AND   sb.有効開始日        <= w_処理実施日
          AND   sb.有効終了日        >= w_処理実施日
     ) C,
    (
        SELECT
                商品部エリアコード,
                商品部エリア名
        FROM
                HNT_商品部エリア
        WHERE   処理区分              < '07'
     ) D,
    (
        SELECT
                商品部地域コード,
                商品部地域名
        FROM
                HNT_商品部地域
        WHERE   処理区分              < '07'
     ) E,
    (
        SELECT
                商品コード,
                商品名,
                ＤＥＰＴコード,
                ＣＬＡＳＳコード
        FROM    HNT_社別商品
        WHERE   社コード              = '81'
          AND   有効開始日           <= w_処理実施日
          AND   有効終了日           >= w_処理実施日
     ) F,
    (
        SELECT
                ＤＥＰＴコード,
                ＤＥＰＴ名
        FROM
                HNT_ＤＥＰＴ
        WHERE   社コード              = '81'
          AND   有効開始日           <= w_処理実施日
          AND   有効終了日           >= w_処理実施日
     ) G,
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
     ) H
    WHERE   A.製造ベンダコード      = C.ベンダーコード
      AND   C.商品部エリアコード    = D.商品部エリアコード
      AND   C.商品部地域コード      = E.商品部地域コード
      AND   A.商品コード            = F.商品コード
      AND   F.ＤＥＰＴコード        = G.ＤＥＰＴコード
      AND   F.ＤＥＰＴコード        = H.ＤＥＰＴコード
      AND   F.ＣＬＡＳＳコード      = H.ＣＬＡＳＳコード
    ;



    -- 処理件数出力
    w_insert_cnt := sql%rowcount;
    dbms_output.put_line('[HNW021][INSERT] : ' || w_insert_cnt ||  '件') ;

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** hnadpl410  END  : ' || w_time_end);

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
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' hnadpl410 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** hnadpl410 RETURN CODE :' || :RtnCd);

    /* 一意制約違反以外のエラー */
    when others then
    -- ロールバック
        rollback;
    -- 戻り値のセット
        :RtnCd  :=  sqlcode;
    -- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
    -- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' hnadpl410 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** hnadpl410 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
