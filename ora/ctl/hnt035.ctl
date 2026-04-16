LOAD DATA
 APPEND INTO TABLE HNT_受発注在庫切れ情報
 (
 利用者コード                             POSITION(1:7) CHAR,
 在庫場所コード                           POSITION(8:15) CHAR,
 配送ベンダコード                         POSITION(16:21) CHAR,
 配送ベンダ名称                           POSITION(22:71) CHAR,
 製造ベンダコード                         POSITION(72:77) CHAR,
 製造ベンダ名称                           POSITION(78:127) CHAR,
 商品コード                               POSITION(128:133) CHAR,
 サイクルコード                           POSITION(134:135) CHAR,
 合計在庫引当数                           POSITION(136:144) DECIMAL EXTERNAL,
 在庫数量                                 POSITION(145:153) DECIMAL EXTERNAL,
 在庫抽選期間開始年月日                   POSITION(154:161) DATE "YYYYMMDD"
  "DECODE(RTRIM(:在庫抽選期間開始年月日),NULL,NULL,RTRIM(:在庫抽選期間開始年月日))",
 在庫抽選期間終了年月日                   POSITION(162:169) DATE "YYYYMMDD"
  "DECODE(RTRIM(:在庫抽選期間終了年月日),NULL,NULL,RTRIM(:在庫抽選期間終了年月日))",
 在庫切速報配信年月日                     POSITION(170:177) DATE "YYYYMMDD"
  "DECODE(RTRIM(:在庫切速報配信年月日),NULL,NULL,RTRIM(:在庫切速報配信年月日))"
 )
