create or replace package BODY OHN001.hnadop021 as
/************************************************************************/
/* ＰＧＭ名         ： HNM_メニュー情報.サービス区分の更新              */
/* ＰＧＭ ｉｄ      ： hnadop021                                        */
/* 機能             ： 『HNM_メニュー情報.サービス区分』のUPDATE        */
/*                                                                      */
/* 作成者           ： M.Yamamoto                                       */
/* 作成日           ： 2025/11/28                                       */
/************************************************************************/
function main
   (P_メニュー単位  in varchar2,
    P_サービス区分  in varchar2,
    P_本部取引区分  in varchar2
   )
 return varchar2
 as
-- ＰＧＭ内変数
 W_TODAY   date;
 W_処理結果  char(2);
 W_存在チェック number;
begin
   -- セキュリティシステム日付ＧＥＴ
     W_TODAY := to_char(SYSDATE,'YYYYMMDD');

   -- 入力値チェック
   if P_メニュー単位 is null or P_メニュー単位 = '%' then
      return '09';
   end if;
   
   if P_本部取引区分 = hnadop900.C_本部取引区分_本部 then   
       
      -- HNM_メニュー情報存在チェック
      SELECT COUNT(*)
        INTO W_存在チェック
        FROM HNM_メニュー情報
       WHERE メニューＩＤ like P_メニュー単位
         AND W_TODAY BETWEEN 有効開始日 AND 有効終了日
         AND 会社コード != 'ZZ'
       ;
          
   else
      -- HNM_メニュー情報存在チェック
      SELECT COUNT(*)
        INTO W_存在チェック
        FROM HNM_メニュー情報
       WHERE メニューＩＤ like P_メニュー単位
         AND W_TODAY BETWEEN 有効開始日 AND 有効終了日
         AND 会社コード = 'ZZ'
       ;
   end if;
   
   if W_存在チェック = 0 then
       return '09';
   end if;

   if P_本部取引区分 = hnadop900.C_本部取引区分_本部 then 
    
      -- HNM_メニュー情報更新
      UPDATE HNM_メニュー情報
         SET サービス区分  = P_サービス区分,
             最終更新日時  = SYSDATE,
             最終更新者ＩＤ = 'BATCH'
       WHERE メニューＩＤ like P_メニュー単位
         AND W_TODAY BETWEEN 有効開始日 AND 有効終了日
         AND 会社コード != 'ZZ'
      ;
   else
      -- HNM_メニュー情報更新
      UPDATE HNM_メニュー情報
         SET サービス区分  = P_サービス区分,
             最終更新日時  = SYSDATE,
             最終更新者ＩＤ = 'BATCH'
       WHERE メニューＩＤ like P_メニュー単位
         AND W_TODAY BETWEEN 有効開始日 AND 有効終了日
          AND 会社コード = 'ZZ'
      ;
   end if;
   
   commit;
   return '00';
exception
   when others then
      return '09';
end;
end hnadop021;
/
