------------------------------------------------------------------------
-- Shell-id      : shnad1121.csh
-- Shell-名      : hnm002.ctl
-- 機能          : HNM_セキュリティ取引相手先 データローディング
-- 作成者        : M.Yamamoto
-- 作成日        : 2025/12/01
-- 修正履歴      :
-- Ｎｏ   修正日付   修正者     修正内容
--    1  xxxx/xx/xx  XXXXXXXXX  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
------------------------------------------------------------------------
------------------------------------------------------------------------
--      初期処理部
------------------------------------------------------------------------
LOAD DATA
INFILE * "FIX 247"
REPLACE
PRESERVE BLANKS
INTO TABLE HNM_セキュリティ取引相手先
TRAILING NULLCOLS
(
    取引相手先ＳＥＱコード      POSITION(1:5),
    有効開始日                  POSITION(6:13) "TO_DATE(:有効開始日,'YYYYMMDD')",
    業務区分                    POSITION(14:21),
    取引相手先コード            POSITION(22:27),
    取引相手先名                POSITION(28:127) "TRIM(:取引相手先名)", 
    取引相手先名＿カナ          POSITION(128:177) "TRIM(:取引相手先名＿カナ)",
    取引相手先名＿略称          POSITION(178:217) "TRIM(:取引相手先名＿略称)",
    有効終了日                  POSITION(218:225) "TO_DATE(:有効終了日,'YYYYMMDD')",
    最終更新日時                POSITION(226:239) "TO_DATE(:最終更新日時,'YYYYMMDDHH24MISS')",
    最終更新氏名コード          POSITION(240:246)
)

