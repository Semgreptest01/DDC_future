create or replace package OHN001.hnadop021
/************************************************************************/
/* ＰＧＭ名         ： HNM_メニュー情報.サービス区分の更新              */
/* ＰＧＭ ｉｄ      ： hnadop021                                        */
/* 機能             ： 『HNM_メニュー情報.サービス区分』のUPDATE        */
/*                                                                      */
/* 作成者           ： M.Yamamoto                                       */
/* 作成日           ： 2025/11/28                                       */
/************************************************************************/
as
   function main
      (P_メニュー単位  in varchar2,
       P_サービス区分  in varchar2,
       P_本部取引区分  in varchar2
   )
   return varchar2;

end     hnadop021;
/
