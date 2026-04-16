create or replace package OHN001.hnadop102 as
/************************************************************************/
/*    ＰＧＭ名    ：  取引相手先コード取得                              */
/*    ＰＧＭ ｉｄ ：    hnadop102                                       */
/*    機能        ：    in_parameterの氏名コード（利用者コード）        */
/*                      に対する取引相手先名＿略称を取得する            */
/*    作成者      ：    VINX                                            */
/*    作成日      ：    2025/11/28                                      */
/************************************************************************/
procedure main
    (P_氏名コード       in     varchar2,
    P_有効日付          in     varchar2,
    P_処理結果          out    varchar2,
    P_取引相手先名      out    varchar2
    );
end hnadop102;
/

--権限がないと実行できない
--GRANT EXECUTE ON OHN001.HNADOP102 TO HHN001 WITH GRANT OPTION;

