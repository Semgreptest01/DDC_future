/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：オンライン宛先パラメータ抽出
/*　　　ＰＧＭID　　　　：hnadpl040.sql
/*　　　機能　　　　　　：HNW_オンライン宛先パラメータを更新する
/*　　　インプット　　　：HNT_処理日付コントロール
/*　　　　　　　　　　　：HNT_オンライン宛先パラメータ
/*　　　アウトプット　　：HNW_オンライン宛先パラメータ
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：G.Saito
/*　　　作成日　　　　　：2009/09/30
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

    w_cnt        number(10);    -- INSERT件数

    w_処理実施日   date;

BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNADPL040 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

    w_cnt := 0;

---------------------------------------------
-- メイン処理
---------------------------------------------

    /* 処理日付取得 */
    SELECT 処理実施日 INTO w_処理実施日 FROM HNT_処理日付コントロール;

    /* HNW_オンライン宛先パラメータ更新 */
    INSERT INTO HNW_オンライン宛先パラメータ
    SELECT  DISTINCT
        SUBSTR(宛先パラメータキーコード,3,1) || ベンダーコード 利用者コード
       ,宛先コード
       ,SUBSTR(宛先パラメータキーコード,1,1) 提供先区分
       ,SUBSTR(宛先パラメータキーコード,2,1) 宛先抽出区分
    FROM HNT_オンライン宛先パラメータ
    WHERE
        宛先パラメータデータ区分 = '13'
    AND 社コード = '81'
    AND 有効開始日 <= w_処理実施日
    AND 有効終了日 >= w_処理実施日
    AND 処理区分 < '07'
    ;

    /* 更新件数表示 */
    w_cnt := sql%rowcount;
    dbms_output.put_line('[HNW004][INSERT] : ' || w_cnt ||  '件');

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNADPL040  END  : ' || w_time_end);

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
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' HNADPL040 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** HNADPL040 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
