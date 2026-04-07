--UPDATED BY : vinx 2019/12/16 ED1912_35412（CH-2019-00578_STEP1_在庫切れ速報12時締めメール関連の改修）日付項目定義修正
set heading off
set feedback off
set trimspool on
set termout off
set echo off
set pages 0
set line 9999
spool ###OUT1###
select
TRANSLATE(NVL(TO_CHAR(チェーンストアコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(宛先コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(ベンダーコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(ベンダー営業所コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(テストフラグ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 START
--TRANSLATE(NVL(TO_CHAR(有効終了日),'YYYYMMDD'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(有効終了日,'YYYYMMDD'),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 END
-- vinx MOD 2019/12/16 ED1912_35412 START
--TRANSLATE(NVL(TO_CHAR(有効開始日),'YYYYMMDD'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(有効開始日,'YYYYMMDD'),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 END
TRANSLATE(NVL(TO_CHAR(配信種別４０コピーフラグ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(配信種別５０コピーフラグ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(コピー１チェーンストアコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(コピー１宛先コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(コピー１センターコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(配信種別４０コピー１フラグ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(配信種別５０コピー１フラグ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(コピー２チェーンストアコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(コピー２宛先コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(コピー２センターコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(配信種別４０コピー２フラグ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(配信種別５０コピー２フラグ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(くくりＶＤＲコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(ＤＤＣ宛先コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(ＤＤＣベンダーコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(実績配信有無フラグ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(処理区分),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 START
--TRANSLATE(NVL(TO_CHAR(物理削除日),'YYYYMMDD'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(物理削除日,'YYYYMMDD'),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 END
-- vinx MOD 2019/12/16 ED1912_35412 START
--TRANSLATE(NVL(TO_CHAR(コピー処理日),'YYYYMMDD'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(コピー処理日,'YYYYMMDD'),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 END
TRANSLATE(NVL(TO_CHAR(コピー処理状況区分),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(物理削除区分),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(作成者氏名コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(作成者氏名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(作成者部署コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(作成者部署名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(作成プログラムＩＤ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 START
--TRANSLATE(NVL(TO_CHAR(作成日時),'YYYYMMDD'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(作成日時,'YYYYMMDD'),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 END
TRANSLATE(NVL(TO_CHAR(オンライン更新者氏名コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(オンライン更新者氏名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(オンライン更新者部署コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(オンライン更新者部署名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(オンライン更新プログラムＩＤ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 START
--TRANSLATE(NVL(TO_CHAR(オンライン更新日時),'YYYYMMDD'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(オンライン更新日時,'YYYYMMDD'),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 END
TRANSLATE(NVL(TO_CHAR(バッチ更新プログラムＩＤ),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 START
--TRANSLATE(NVL(TO_CHAR(バッチ更新日時),'YYYYMMDD'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(バッチ更新日時,'YYYYMMDD'),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
-- vinx MOD 2019/12/16 ED1912_35412 END
TRANSLATE(NVL(TO_CHAR(ロック連番),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(削除区分),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(更新端末ＩＤ),'_'),' '||chr(9)||chr(10)||chr(13),'___')
from
HNT_オンライン宛先ベンダー
;
spool off
;
quit
