LOAD DATA
 APPEND INTO TABLE HNT_入出残セットマスタ
 (
 ＤＤＣコード                             POSITION(1:6) CHAR,
 ベンダーコード                           POSITION(7:12) CHAR,
 商品コード                               POSITION(13:18) CHAR,
 実在庫数                                 POSITION(19:26) ZONED NULLIF 実在庫数=BLANKS
 )
