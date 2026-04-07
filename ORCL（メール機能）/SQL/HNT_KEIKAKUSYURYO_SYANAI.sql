set heading off
set feedback off
set trimspool on
set termout off
set echo off
set pages 0
set line 9999
spool ###OUT1###
select
TRANSLATE(NVL(TO_CHAR(T.センターコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.センター名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.ベンダーコード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.ベンダー名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.ＤＥＰＴ名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.ＣＬＡＳＳ名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.商品コード),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.商品名),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.計画終了有効開始日),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.計画終了有効終了日),'_'),' '||chr(9)||chr(10)||chr(13),'___')||' '||
TRANSLATE(NVL(TO_CHAR(T.実在庫数),'_'),' '||chr(9)||chr(10)||chr(13),'___')
from (
       SELECT
          A.センターコード センターコード
         ,A.センター名     センター名
         ,A.ベンダーコード ベンダーコード
         ,A.ベンダー名     ベンダー名
         ,A.ＤＥＰＴ名     ＤＥＰＴ名
         ,A.ＣＬＡＳＳ名   ＣＬＡＳＳ名
         ,A.商品コード     商品コード
         ,A.商品名         商品名
         ,TO_CHAR(A.計画終了有効開始日,'YYYY/MM/DD') 計画終了有効開始日
         ,TO_CHAR(B.計画終了有効終了日,'YYYY/MM/DD') 計画終了有効終了日
         ,A.実在庫数       実在庫数
       FROM
       --テーブルA  計画終了対象商品_日次と在庫データを取得する
           (
            SELECT DISTINCT 
                  KEI.センターコード センターコード
                 ,KEI.センター名     センター名
                 ,KEI.ベンダーコード ベンダーコード
                 ,KEI.ベンダー名     ベンダー名
                 ,KEI.ＤＥＰＴコード ＤＥＰＴコード
                 ,KEI.ＤＥＰＴ名     ＤＥＰＴ名
                 ,KEI.ＣＬＡＳＳコード ＣＬＡＳＳコード
                 ,KEI.ＣＬＡＳＳ名   ＣＬＡＳＳ名
                 ,KEI.商品コード     商品コード
                 ,KEI.商品名         商品名
                 ,KEI.計画終了有効開始日 計画終了有効開始日
               --在庫数
               --取得できなければ「99999999」をセットする
                ,CASE
                   WHEN ZAI.実在庫数 IS NULL
                    THEN 99999999
                   ELSE ZAI.実在庫数
                 END 実在庫数
            FROM  (SELECT 変位日１ FROM HNT_日付コントロール WHERE 日付区分 = 0 AND 日付変位数 = 0) ADT0
                 ,HNT_計画終了対象商品＿日次 KEI
                  LEFT JOIN HNT_入出残セットマスタ ZAI
                  ON KEI.センターコード = ZAI.ＤＤＣコード
                  AND KEI.くくりＶＤＲコード = ZAI.ベンダーコード
                  AND KEI.商品コード = ZAI.商品コード
            WHERE KEI.履歴区分 = 0
              AND KEI.不可日設定日 IS NULL
              AND KEI.計画終了有効開始日 <= ADT0.変位日１
              AND KEI.推奨取消日 = '9999-12-31'
           ) A
       INNER JOIN 
       -- テーブルB  ベンダー別計画終了対象商品抽出 ※計画終了終了日－２１日前（３週前）から配信開始する
           (
            SELECT  PPL.商品コード
                   ,PPL.ベンダーコード
                   ,PPL.配送センターベンダーコード
                   ,TO_DATE(PPL.計画終了有効開始日,'YYYY-MM-DD') 計画終了有効開始日
                   ,TO_DATE(PPL.計画終了有効終了日,'YYYY-MM-DD') 計画終了有効終了日
            FROM (SELECT TO_CHAR(変位日１,'YYYYMMDD') 変位日１ FROM HNT_日付コントロール WHERE 日付区分 = 0 AND 日付変位数 = 0) BDT0
                ,(SELECT TO_CHAR(変位日１,'YYYYMMDD') 変位日１ FROM HNT_日付コントロール WHERE 日付区分 = 0 AND 日付変位数 = +21 ) BDT21
                ,HNT_ベンダー別計画終了対象商品 PPL
            WHERE
                  PPL.不可日設定日 IS NULL
            AND   PPL.計画終了有効開始日 <= BDT0.変位日１       -- 計画終了開始日＜＝処理日
            AND   PPL.計画終了有効終了日 >= BDT0.変位日１       -- 計画終了終了日＞＝処理日
            AND   PPL.計画終了有効終了日 <= BDT21.変位日１      -- 計画終了終了日＜＝処理日＋２１日
            AND   PPL.推奨取消日 = '99991231'
           ) B
       -- テーブルAとテーブルBを結合
       ON  A.センターコード = B.配送センターベンダーコード
       AND A.ベンダーコード = B.ベンダーコード
       AND A.商品コード     = B.商品コード
       ORDER BY    A.センターコード
                  ,A.ベンダーコード
                  ,A.ＤＥＰＴコード
                  ,A.ＣＬＡＳＳコード
                  ,A.商品コード
     )T
;
spool off
;
quit
