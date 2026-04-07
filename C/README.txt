コンパイル方法について

***
*** conv系以外のprogram
***

   1)通常コンパイル
     make 【ターゲット名(ソースから.cを除いたもの)】

   2)DEBUGマクロ有効コンパイル
     make -f Makefile_pt 【ターゲット名(ソースから.cを除いたもの)】



***
*** conv系 program
***

   1)ライブラリ及びmainプログラム全て(v_convで始まるもの)のmakeを行う
     make -f Makefile_conv all

   2)ライブラリだけmakeを行う
     make -f Makefile_conv

   3)正規ＰＧ以外(v_convで始まらない)
     make -f Makefile_conv 【ターゲット名(ソースから.cを除いたもの)】



***
*** v_commonを使う program
***  対象program
***   v_ldate
***
     make -f Makefile_v_common 【ターゲット名(ソースから.cを除いたもの)】
