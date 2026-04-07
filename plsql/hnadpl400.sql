/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：オンライン画面サービス開始、停止
/*　　　ＰＧＭID　　　　：hnadpl400.sql
/*　　　機能　　　　　　：共通メニューセキュリティー提供のＰＬ／ＳＱＬパッケージ
/*　　　　　　　　　　　：hnadop021（HNM_メニュ情報.サービス区分の更新）
/*　　　　　　　　　　　：をＣＡＬＬし、HNM_メニュ情報のサービス区分を
/*　　　　　　　　　　　：更新する。
/*　　　　　　　　　　　：
/*　　　インプット　　　：PARM1:メニュー単位
/*　　　　　　　　　　　：PARM2:サービス区分
/*　　　　　　　　　　　：                    '00':サービス中
/*　　　　　　　　　　　：                    '02':バッチ処理中のためサービス停止
/*　　　　　　　　　　　：                    '21':月次バッチ処理中のためサービス停止
/*　　　　　　　　　　　：PARM3:本部取引先区分
/*　　　　　　　　　　　：                    '0':本部
/*　　　　　　　　　　　：                    '1':取引先
/*　　　　　　　　　　　：
/*　　　アウトプット　　：HNM_メニュ情報
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：H.Nagata
/*　　　作成日　　　　　：2010/03/03
/*　　　修正履歴　　　　：2025/12/01 M.Yamamoto (Replace) LMDC移行対応
/*　　　　　　　　　　　：                                 PARAM3追加
/*　　　　　　　　　　　：                                 アウトプットをHNM_メニュ情報に変更
/*　　　　　　　　　　　：                                 PL/SQLパッケージをhnadop021に変更
/*　　　　　　　　　　　：                                 上記項目変更に伴うロジック変更
/********************************************************************************/

set feed off
set verify off
set heading off
set underline off
set termout on
set serveroutput on
whenever oserror  exit sql.oscode rollback
whenever sqlerror exit sql.sqlcode rollback

variable RtnCd     number
variable ErrMsg    varchar2(100)

---------------------------------------------
-- 変数宣言
---------------------------------------------
DECLARE
  w_menu       varchar2(40);  -- メニュー単位
  w_sv_kbn     varchar2(02);  -- サービス区分
  pkg_rtn      varchar2(02);  -- FUNCTION リターン値
  w_time_start varchar2(19);  -- プログラム開始日時
  w_time_end   varchar2(19);  -- プログラム終了日時
-- 2025/12/01 M.Yamamoto LMDC移行対応 start
  w_honbu_kbn  varchar2(01) ;  -- 本部取引区分（0:本部、1:取引先）
-- 2025/12/01 M.Yamamoto LMDC移行対応 end

---------------------------------------------
-- メイン処理
---------------------------------------------
BEGIN
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** HNADPL400 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);

---------------------------------------------
-- FUNCTIONの引数設定
---------------------------------------------
    w_menu   := '&&1';  -- メニュー単位
    w_sv_kbn := '&&2';  -- サービス区分
-- 2025/12/01 M.Yamamoto LMDC移行対応 start
--    dbms_output.put_line('【メニュー単位 : ' || w_menu || '／サービス区分:' || w_sv_kbn || '】');
    w_honbu_kbn := '&&3';  -- 本部取引先区分
    dbms_output.put_line('【メニュー単位 : ' || w_menu || '／サービス区分:' || w_sv_kbn 
                      || '／本部取引先区分 : ' || w_honbu_kbn || '】');
-- 2025/12/01 M.Yamamoto LMDC移行対応 end

---------------------------------------------
-- パッケージsmzzop021のＣＡＬＬ
---------------------------------------------
-- 2025/12/01 M.Yamamoto LMDC移行対応 start
--    pkg_rtn  := smzzop021.main(w_menu,w_sv_kbn);
    pkg_rtn  := hnadop021.main(w_menu, w_sv_kbn, w_honbu_kbn);
-- 2025/12/01 M.Yamamoto LMDC移行対応 end

---------------------------------------------
-- FUNCTIONの返却値をSQLのリターン値に設定
---------------------------------------------
    :RtnCd   := pkg_rtn;

    if  pkg_rtn  !=  '00'  then
        :ErrMsg := '++ hnadpl400.sql FUNCTION smzzop021.main ' ||
                   'CALL ERR STATUS = "' || pkg_rtn || '" ++';
    end if;

    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** HNADPL400  END  : ' || w_time_end);

---------------------------------------------
-- 例外処理
---------------------------------------------
EXCEPTION
  when others then
    :RtnCd := sqlcode;
    :ErrMsg := substr(sqlerrm,1,100);
    rollback;
END;
/

print RtnCd
print ErrMsg

EXIT :RtnCd;
/
