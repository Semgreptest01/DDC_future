LOAD DATA
 APPEND INTO TABLE HNT_計画終了対象商品
 (
 商品部地域コード                         POSITION(1:2) CHAR,
 利用者コード                             POSITION(3:9) CHAR,
 ベンダーコード                           POSITION(10:15) CHAR,
 商品コード                               POSITION(16:21) CHAR,
 店案内年度週ＮＯ                         POSITION(22:27) CHAR,
 商品部エリアコード                       POSITION(28:29) CHAR,
 くくりベンダーコード                     POSITION(30:35) CHAR,
 コピー宛先コード１                       POSITION(36:43) CHAR,
 宛先コード                               POSITION(44:51) CHAR,
 センターコード                           POSITION(52:57) CHAR,
 ＤＥＰＴコード                           POSITION(58:59) CHAR,
 ＣＬＡＳＳコード                         POSITION(60:62) CHAR,
 計画終了有効開始日                       POSITION(63:70) DATE "YYYYMMDD"
  "DECODE(RTRIM(:計画終了有効開始日),NULL,NULL,RTRIM(:計画終了有効開始日))",
 不可日設定日                             POSITION(71:78) DATE "YYYYMMDD"
  "DECODE(RTRIM(:不可日設定日),NULL,NULL,RTRIM(:不可日設定日))",
 推奨取消日                               POSITION(79:86) DATE "YYYYMMDD"
  "DECODE(RTRIM(:推奨取消日),NULL,NULL,RTRIM(:推奨取消日))"
 )
