LOAD DATA
 APPEND INTO TABLE HNT_商品部エリア
 (
 商品部エリアコード                       POSITION(1:2) CHAR,
 商品部エリア名                           POSITION(3:52) CHAR
  "DECODE(RTRIM(RTRIM(:商品部エリア名),'　'),NULL,'　',RTRIM(RTRIM(:商品部エリア名),'　'))",
 商品部エリア名カナ                       POSITION(53:77) CHAR
  "DECODE(RTRIM(RTRIM(:商品部エリア名カナ),'　'),NULL,'　',RTRIM(RTRIM(:商品部エリア名カナ),'　'))",
 ダミー商品部エリアフラグ                 POSITION(78:79) CHAR,
 社コード                                 POSITION(80:85) CHAR,
 処理区分                                 POSITION(86:87) CHAR,
 コピー処理日                             POSITION(88:95) DATE "YYYYMMDD",
 コピー処理状況区分                       POSITION(96:97) CHAR,
 作成者氏名コード                         POSITION(98:105) CHAR,
 作成者氏名                               POSITION(106:135) CHAR,
 作成者部署コード                         POSITION(136:141) CHAR,
 作成者部署名                             POSITION(142:181) CHAR,
 作成プログラムＩＤ                       POSITION(182:221) CHAR,
 作成日時                                 POSITION(222:235) DATE "YYYYMMDDHH24MISS",
 オンライン更新者氏名コード               POSITION(236:243) CHAR,
 オンライン更新者氏名                     POSITION(244:273) CHAR,
 オンライン更新者部署コード               POSITION(274:279) CHAR,
 オンライン更新者部署名                   POSITION(280:319) CHAR,
 オンライン更新プログラムＩＤ             POSITION(320:359) CHAR,
 オンライン更新日時                       POSITION(360:373) DATE "YYYYMMDDHH24MISS",
 バッチ更新プログラムＩＤ                 POSITION(374:413) CHAR,
 バッチ更新日時                           POSITION(414:427) DATE "YYYYMMDDHH24MISS",
 ロック連番                               POSITION(428:431) ZONED NULLIF ロック連番=BLANKS,
 削除区分                                 POSITION(432:433) CHAR,
 更新端末ＩＤ                             POSITION(434:463) CHAR
 )
