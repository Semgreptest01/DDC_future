LOAD DATA
 APPEND INTO TABLE HNT_商品部地域
 (
 商品部地域コード                         POSITION(1:3) CHAR,
 商品部地域名                             POSITION(4:53) CHAR
  "DECODE(RTRIM(RTRIM(:商品部地域名),'　'),NULL,'　',RTRIM(RTRIM(:商品部地域名),'　'))",
 商品部地域名カナ                         POSITION(54:78) CHAR
  "DECODE(RTRIM(RTRIM(:商品部地域名カナ),'　'),NULL,'　',RTRIM(RTRIM(:商品部地域名カナ),'　'))",
 代表商品部地域フラグ                     POSITION(79:80) CHAR,
 ダミー商品部地域フラグ                   POSITION(81:82) CHAR,
 商品部エリアコード                       POSITION(83:84) CHAR,
 社コード                                 POSITION(85:90) CHAR,
 処理区分                                 POSITION(91:92) CHAR,
 コピー処理日                             POSITION(93:100) DATE "YYYYMMDD",
 コピー処理状況区分                       POSITION(101:102) CHAR,
 作成者氏名コード                         POSITION(103:110) CHAR,
 作成者氏名                               POSITION(111:140) CHAR,
 作成者部署コード                         POSITION(141:146) CHAR,
 作成者部署名                             POSITION(147:186) CHAR,
 作成プログラムＩＤ                       POSITION(187:226) CHAR,
 作成日時                                 POSITION(227:240) DATE "YYYYMMDDHH24MISS",
 オンライン更新者氏名コード               POSITION(241:248) CHAR,
 オンライン更新者氏名                     POSITION(249:278) CHAR,
 オンライン更新者部署コード               POSITION(279:284) CHAR,
 オンライン更新者部署名                   POSITION(285:324) CHAR,
 オンライン更新プログラムＩＤ             POSITION(325:364) CHAR,
 オンライン更新日時                       POSITION(365:378) DATE "YYYYMMDDHH24MISS",
 バッチ更新プログラムＩＤ                 POSITION(379:418) CHAR,
 バッチ更新日時                           POSITION(419:432) DATE "YYYYMMDDHH24MISS",
 ロック連番                               POSITION(433:436) ZONED NULLIF ロック連番=BLANKS,
 削除区分                                 POSITION(437:438) CHAR,
 更新端末ＩＤ                             POSITION(439:468) CHAR
 )
