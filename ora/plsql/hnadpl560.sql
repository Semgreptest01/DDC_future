-------------------------------------------------------------------------------
-- システム名    : DDC返品コスト削減＆他システムIF
-- サブシステム名: DDC返品コスト削減
-- プログラム名  : 統計情報取得
-- プログラムID  : hnadpl560
-- 作成者        : S.Oyama
-- 作成日        : 2013/08/02 (Create)
-- 修正履歴      :
-------------------------------------------------------------------------------
-- 機能概要:
--        指定されたテーブルの統計情報を取得する
-------------------------------------------------------------------------------
--
-- Argument:
--        &1 スキーマ
--        &2 テーブル名
--        &3 サンプルレート
--
-------------------------------------------------------------------------------

SET ECHO ON
SET FEEDBACK ON
SET TERMOUT ON
SET TIMING ON
SET SERVEROUTPUT ON SIZE 1000000
SET DEFINE '&'

VARIABLE StatusCode NUMBER

-- パラメータ変数
DEFINE pi_own_name = &1
DEFINE pi_tab_name = &2
DEFINE pi_estimate_percent = &3


BEGIN
  DBMS_STATS.GATHER_TABLE_STATS(
   OWNNAME          => '&&pi_own_name'
  ,TABNAME          => '&&pi_tab_name'
  ,ESTIMATE_PERCENT => '&&pi_estimate_percent'
  );

 :StatusCode := SQLCODE;

  --処理終了のコード値、メッセージを出力
  DBMS_OUTPUT.PUT_LINE( SQLERRM(SQLCODE) );

EXCEPTION
  WHEN OTHERS        THEN
    :StatusCode := SQLCODE;
    DBMS_OUTPUT.PUT_LINE( SQLERRM(SQLCODE) );

END;
/
EXIT  :StatusCode
