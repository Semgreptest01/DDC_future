/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：在庫切れ速報更新通知１２時締め
/*　　　ＰＧＭID　　　　：hnadpl460.sql
/*　　　機能　　　　　　：HNT_更新通知のデータ区分「07」レコードを更新する
/*　　　　　　　　　　　：
/*　　　インプット　　　：HNT_処理日付コントロール
/*　　　　　　　　　　　：
/*　　　アウトプット　　：HNT_HNT_更新通知
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：H.Nagata
/*　　　作成日　　　　　：2010/03/03
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
    w_update_cnt number(10);    -- UPDATE件数
    w_shr_js_ymd date;          -- 処理実施日

BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNADPL460 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_insert_cnt := 0 ;
    w_update_cnt := 0 ;

---------------------------------------------
-- HNT_更新処理処理
---------------------------------------------

    -- 処理実施日を取得
    SELECT 処理実施日 into w_shr_js_ymd FROM HNT_処理日付コントロール ;

    -- HNT_更新通知のデータ区分「07」のレコードを更新
    UPDATE HNT_更新通知
    SET    最終更新日 = w_shr_js_ymd
    WHERE  データ区分 = '07' ;

    w_update_cnt := SQL%ROWCOUNT ;

    -- UPDATE 出来なかった場合は INSERTする
    if w_update_cnt = 0 then
        INSERT INTO HNT_更新通知 ( データ区分 , 最終更新日 )
        VALUES ( '07' , w_shr_js_ymd ) ;

        w_insert_cnt := SQL%ROWCOUNT ;

    end if;

    -- 処理件数出力
    dbms_output.put_line('[HNT003][INSERT] : ' || w_insert_cnt ||  '件') ;
    dbms_output.put_line('[HNT003][UPDATE] : ' || w_update_cnt ||  '件') ;

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNADPL460  END  : ' || w_time_end);

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
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL460 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL460 RETURN CODE :' || :RtnCd);

    /* 一意制約違反以外のエラー */
    when others then
    -- ロールバック
        rollback;
    -- 戻り値のセット
        :RtnCd  :=  sqlcode;
    -- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
    -- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL460 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL460 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
