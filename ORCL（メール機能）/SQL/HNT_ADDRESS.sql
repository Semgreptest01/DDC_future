set heading off
set feedback off
set trimspool on
set termout off
set echo off
set pages 0
set line 9999
spool ###OUT1###
select
 lpad( rownum, 7, '0' )||' '||
 TRANSLATE(NVL(TO_CHAR(有効開始日),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(有効終了日),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(利用者コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(利用者名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(メールアドレス),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(緊急カット商品配信Ｆ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(在庫切れ速報１２時締め配信Ｆ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(在庫切れ速報２２時締め配信Ｆ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(計画終了対象商品＿日次),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(配信フラグ予備１),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(配信フラグ予備２),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(配信フラグ予備３),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(登録日),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(登録者),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(更新日),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
 TRANSLATE(NVL(TO_CHAR(更新者),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '
from
 (
select
   DECODE(TO_CHAR(有効開始日,'YYYYMMDD'),'19000101','00000000',TO_CHAR(有効開始日,'YYYYMMDD')) as 有効開始日
  ,DECODE(TO_CHAR(有効終了日,'YYYYMMDD'),'99991231','99999999',TO_CHAR(有効終了日,'YYYYMMDD')) as 有効終了日
  ,利用者コード
  ,利用者名
  ,メールアドレス
  ,緊急カット商品配信Ｆ
  ,在庫切れ速報１２時締め配信Ｆ
  ,在庫切れ速報２２時締め配信Ｆ
  ,計画終了対象商品＿日次
  ,配信フラグ予備１
  ,配信フラグ予備２
  ,配信フラグ予備３
  ,TO_CHAR(登録日,'YYYYMMDD') as 登録日
  ,登録者
  ,TO_CHAR(更新日,'YYYYMMDD') as 更新日
  ,更新者
  from HNT_アドレスマスタ
  order by 利用者コード
)
;
spool off
;
quit
