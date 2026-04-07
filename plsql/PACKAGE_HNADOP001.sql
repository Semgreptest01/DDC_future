create or replace package OHN001.hnadop001 as
/************************************************************************/
/*    ＰＧＭ名    ：  サービス時間判定                                  */
/*    ＰＧＭ ｉｄ ：    hnadop001                                       */
/*    機能        ：    in_parameterの処理連番をＫＥＹに『処理』        */
/*                      を参照しサービス時間の判定を行う                */
/*    作成者      ：    VINX                                            */
/*    作成日      ：    2025/11/28                                      */
/************************************************************************/
procedure main
    (P_メニュー単位連番 in     varchar2,
    P_本部取引区分      in     varchar2,
    P_判定結果          out    varchar2,
    P_処理日            out    varchar2
    );
end hnadop001;
/

--権限がないと実行できない
--GRANT EXECUTE ON OHN001.HNADOP001 TO HHN001 WITH GRANT OPTION;

