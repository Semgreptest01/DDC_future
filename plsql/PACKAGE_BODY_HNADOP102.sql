create or replace package body OHN001.hnadop102 as
/************************************************************************/
/*    ＰＧＭ名    ：  取引相手先名称取得                                */
/*    ＰＧＭ ｉｄ ：    hnadop102                                       */
/*    機能        ：    in_parameterの氏名コード（利用者コード）に      */
/*                      対する取引相手先名＿略称を取得する              */
/*    作成者      ：    VINX                                            */
/*    作成日      ：    2025/11/28                                      */
/************************************************************************/
procedure main
   (P_氏名コード        in     varchar2,
    P_有効日付          in     varchar2,
    P_処理結果          out    varchar2,
    P_取引相手先名      out    varchar2)
as
begin
    --  ID管理の取引相手先名称取得（HNM_セキュリティ取引相手先）

    -- 取引相手先名＿略称取得
    select 取引相手先名＿略称
        into P_取引相手先名
        from HNM_セキュリティ取引相手先
        where 取引相手先ＳＥＱコード = substrb(P_氏名コード,1,5)
          and to_date(P_有効日付,'YYYYMMDD') between 有効開始日 and 有効終了日;

    P_処理結果 := hnadop900.C_CHK_00;

    exception
        --no_data_found
        when no_data_found then
            P_処理結果    :=    hnadop900.C_CHK_08;
        --other
        when others then
            P_処理結果    :=    hnadop900.C_CHK_09;

end;
end hnadop102;
/

--権限がないと実行できない
--GRANT EXECUTE ON OHN001.HNADOP102 TO HHN001 WITH GRANT OPTION;

