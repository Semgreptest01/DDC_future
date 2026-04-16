/********************************************************************************/
/*　　　システム名　　　：ＤＤＣ返品コスト削減
/*　　　ＰＧＭ名　　　　：計画終了対象商品日次作成
/*　　　ＰＧＭID　　　　：hnadpl440.sql
/*　　　機能　　　　　　：HNW_計画終了対象商品名称付とHNT_計画終了対象商品＿日次
/*　　　　　　　　　　　：のレコードのKEY値を比較し
/*　　　　　　　　　　　：HNT_計画終了対象商品＿日次を更新する
/*　　　インプット　　　：HNW_計画終了対象商品名称付　<トラン>
/*　　　　　　　　　　　：HNT_計画終了対象商品＿日次 　<マスタ>
/*　　　　　　　　　　　：HNT_処理日付コントロール
/*　　　　　　　　　　　：
/*　　　アウトプット　　：HNT_計画終了対象商品_日次
/*　　　　　　　　　　　：
/*　　　作成者　　　　　：H.Nagata
/*　　　作成日　　　　　：2010/03/05
/*　　　修正履歴　　　　：YYYY/MM/DD xxxxx
/********************************************************************************/
whenever oserror  exit sql.sqlcode rollback
whenever sqlerror exit sql.sqlcode rollback
set feed off
set verify off
set heading off
set underline off
set termout on
set serveroutput on size 20000

variable RtnCd     number
variable ErrMsg    varchar2(100)

---------------------------------------------
-- 変数宣言
---------------------------------------------
DECLARE
    w_time_start varchar2(19);  -- プログラム開始日時
    w_time_end   varchar2(19);  -- プログラム終了日時

    w_input_cnt number(10);     -- HNW_計画終了対象商品名称付の入力件数
    w_insert_cnt number(10);    -- INSERT件数
    w_update_cnt number(10);    -- UPDATE件数
    w_sakujyo_cnt number(10);   -- 履歴区分を'1'に変更した件数

    w_処理実施日 date;          -- 処理実施日
    w_HNW021_key char(21);      -- key値格納用１
    w_HNT032_key char(21);      -- key値格納用２
    w_idx_key char(21);         -- インデックスkey
    w_inst_FLG char(1);         -- insertフラグ
    w_inst_CK  number(10);      -- insertチェック

---------------------------------------------
-- カーソル宣言
---------------------------------------------
--  HNW_計画終了対象商品名称付取得用

    /* ベンダー付替え時には同一地域、同一利用者、同一ベンダー、同一商品で
       ２つの案内週データが送信される為、最新の案内週データを使用する。
    */
    CURSOR HNW021_CUR
        IS
        select
            a.商品部地域コード,
            a.利用者コード,
            a.ベンダーコード,
            a.商品コード,
            a.店案内年度週ＮＯ,
            a.商品部エリアコード,
            a.商品部エリア名,
            a.商品部地域名,
            a.くくりＶＤＲコード,
            a.くくりＶＤＲ名,
            a.ベンダー名,
            a.コピー宛先１コード,
            a.宛先コード,
            a.宛先名,
            a.センターコード,
            a.センター名,
            a.ＤＥＰＴコード,
            a.ＤＥＰＴ名,
            a.ＣＬＡＳＳコード,
            a.ＣＬＡＳＳ名,
            a.商品名,
            a.計画終了有効開始日,
            a.推奨取消日,
            a.不可日設定日,
            a.作成日時
        from
            HNW_計画終了対象商品名称付  a,
            (select
                商品部地域コード,
                利用者コード,
                ベンダーコード,
                商品コード,
                max(店案内年度週ＮＯ) 店案内年度週ＮＯ
            from
                HNW_計画終了対象商品名称付
            group by
                商品部地域コード,
                利用者コード,
                ベンダーコード,
                商品コード
            ) b
        where
            a.商品部地域コード = b.商品部地域コード
        and
            a.利用者コード = b.利用者コード
        and
            a.ベンダーコード = b.ベンダーコード
        and
            a.商品コード = b.商品コード
        and
            a.店案内年度週ＮＯ = b.店案内年度週ＮＯ
        order by
            a.商品部地域コード,
            a.利用者コード,
            a.ベンダーコード,
            a.商品コード
    ;
    w_HNW021 HNW021_CUR%rowtype;

--  HNT_計画終了対象商品＿日次取得用
    CURSOR HNT032_CUR
        IS
        select   商品部地域コード
                ,利用者コード
                ,ベンダーコード
                ,商品コード
                ,店案内年度週ＮＯ
                ,商品部エリアコード
                ,商品部エリア名
                ,商品部地域名
                ,くくりＶＤＲコード
                ,くくりＶＤＲ名
                ,ベンダー名
                ,コピー宛先１コード
                ,宛先コード
                ,宛先名
                ,センターコード
                ,センター名
                ,ＤＥＰＴコード
                ,ＤＥＰＴ名
                ,ＣＬＡＳＳコード
                ,ＣＬＡＳＳ名
                ,商品名
                ,計画終了有効開始日
                ,推奨取消日
                ,履歴区分
                ,不可日設定日
                ,作成日時
          from  HNT_計画終了対象商品＿日次
          where 履歴区分 = '0'
        order by    商品部地域コード
                   ,利用者コード
                   ,ベンダーコード
                   ,商品コード
    ;
    w_HNT032 HNT032_CUR%rowtype;

BEGIN
---------------------------------------------
-- 初期処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_start from dual;
    dbms_output.put_line('**** hnadpl440 START : ' || w_time_start);
    :RtnCd := 0;
    :ErrMsg := substr(sqlerrm,1,100);
    
    w_input_cnt := 0 ;
    w_insert_cnt := 0 ;
    w_update_cnt := 0 ;
    w_sakujyo_cnt := 0 ;
    w_HNW021_key := null ;
    w_HNT032_key := null ;
    w_idx_key := null ;
    w_inst_FLG := '0' ;
    w_inst_CK := 0 ;

---------------------------------------------
-- メイン処理
---------------------------------------------

    /* 処理日付取得 */
    select 処理実施日 into w_処理実施日 from HNT_処理日付コントロール;

    OPEN HNW021_CUR;
    OPEN HNT032_CUR;

    LOOP
        ---------------------------------------------
        -- 初回及び前回処理済のレコード(indexキーで判定)から読込み
        ---------------------------------------------

        /* HNW_計画終了対象商品名称付取得用読込 */
        IF (w_idx_key is null  or  (w_HNW021_key = w_idx_key  and  HNW021_CUR%found)) THEN
            fetch HNW021_CUR into w_HNW021;
            IF HNW021_CUR%found THEN
                w_HNW021_key := w_HNW021.商品部地域コード || w_HNW021.利用者コード || w_HNW021.ベンダーコード || w_HNW021.商品コード;
                w_input_cnt := w_input_cnt + 1 ;
            ELSE
                w_HNW021_key := null ;
            END IF;
        END IF;

        /* HNT_計画終了対象商品＿日次取得用読込 */
        IF (w_idx_key is null  or  (w_HNT032_key = w_idx_key  and  HNT032_CUR%found)) THEN
            fetch HNT032_CUR into w_HNT032;
            IF HNT032_CUR%found THEN
                w_HNT032_key := w_HNT032.商品部地域コード || w_HNT032.利用者コード || w_HNT032.ベンダーコード || w_HNT032.商品コード;
            ELSE
                w_HNT032_key := null ;
            END IF;
        END IF;


        ---------------------------------------------
        -- マッチング処理
        ---------------------------------------------
        /* マッチング終了 */
        IF HNW021_CUR%notfound and HNT032_CUR%notfound THEN
            EXIT;

        /* トランKEY < マスタKEY (トランのみ） */
        ELSIF HNT032_CUR%notfound  or  (w_HNT032_key is not null  and  (w_HNW021_key < w_HNT032_key))  THEN
            /* トランのKEYをインデックスキーに設定 */
            w_idx_key := w_HNW021_key ;
            /* トランのレコードをinsert */
            w_inst_FLG := '1' ;

        /* トランKEY > マスタKEY (マスタのみ） */
        ELSIF HNW021_CUR%notfound  or  (w_HNW021_key is not null  and  (w_HNW021_key > w_HNT032_key))  THEN
            /* マスタのKEYをインデックスキーに設定 */
            w_idx_key := w_HNT032_key ;
            /* マスタの履歴区分と作成日時をupdate */
            update HNT_計画終了対象商品＿日次
                    set 履歴区分 = '1',
                        作成日時 = w_処理実施日
                    where   商品部地域コード = w_HNT032.商品部地域コード
                        and 利用者コード = w_HNT032.利用者コード
                        and ベンダーコード = w_HNT032.ベンダーコード
                        and 商品コード = w_HNT032.商品コード
                        and 店案内年度週ＮＯ = w_HNT032.店案内年度週ＮＯ
                        and 履歴区分 = '0'
                    ;
            w_sakujyo_cnt := w_sakujyo_cnt + 1 ;

        /* トランKEY = マスタKEY (Key一致） */
        ELSIF w_HNW021_key = w_HNT032_key THEN
            /* 一致したKEY(トランのKEY=マスタのKEY)をインデックスキーに設定 */
            w_idx_key := w_HNW021_key ;
            /* カット日変更有り */
            IF w_HNW021.店案内年度週ＮＯ != w_HNT032.店案内年度週ＮＯ THEN
                /* マスタの履歴区分と作成日時をupdate及びトランのレコードをinsert */
                update HNT_計画終了対象商品＿日次
                        set 履歴区分 = '1',
                            作成日時 = w_処理実施日
                        where   商品部地域コード = w_HNT032.商品部地域コード
                            and 利用者コード = w_HNT032.利用者コード
                            and ベンダーコード = w_HNT032.ベンダーコード
                            and 商品コード = w_HNT032.商品コード
                            and 店案内年度週ＮＯ = w_HNT032.店案内年度週ＮＯ
                            and 履歴区分 = '0'
                        ;
                w_sakujyo_cnt := w_sakujyo_cnt + 1 ;
                w_inst_FLG := '1' ;
                
            /* カット日変更無し */
            ELSE
                /* トランの値でマスタのプライマリキー（KEY値+店案内年度週ＮＯ）以外の各項目と作成日時をupdate */
                update HNT_計画終了対象商品＿日次
                        set 商品部エリアコード = w_HNW021.商品部エリアコード,
                            商品部エリア名     = w_HNW021.商品部エリア名,
                            商品部地域名       = w_HNW021.商品部地域名,
                            くくりＶＤＲコード = w_HNW021.くくりＶＤＲコード,
                            くくりＶＤＲ名     = w_HNW021.くくりＶＤＲ名,
                            ベンダー名         = w_HNW021.ベンダー名,
                            コピー宛先１コード = w_HNW021.コピー宛先１コード,
                            宛先コード         = w_HNW021.宛先コード,
                            宛先名             = w_HNW021.宛先名,
                            センターコード     = w_HNW021.センターコード,
                            センター名         = w_HNW021.センター名,
                            ＤＥＰＴコード     = w_HNW021.ＤＥＰＴコード,
                            ＤＥＰＴ名         = w_HNW021.ＤＥＰＴ名,
                            ＣＬＡＳＳコード   = w_HNW021.ＣＬＡＳＳコード,
                            ＣＬＡＳＳ名       = w_HNW021.ＣＬＡＳＳ名,
                            商品名             = w_HNW021.商品名,
                            計画終了有効開始日 = w_HNW021.計画終了有効開始日,
                            推奨取消日         = w_HNW021.推奨取消日,
                            履歴区分           = '0',
                            不可日設定日       = w_HNW021.不可日設定日,
                            作成日時           = w_処理実施日
                        where   商品部地域コード = w_HNT032.商品部地域コード
                            and 利用者コード = w_HNT032.利用者コード
                            and ベンダーコード = w_HNT032.ベンダーコード
                            and 商品コード = w_HNT032.商品コード
                            and 店案内年度週ＮＯ = w_HNT032.店案内年度週ＮＯ
                            and 履歴区分 = '0'
                        ;
                w_update_cnt := w_update_cnt + 1;
            END IF;
        END IF;

        /* トランレコードinsert(新規の計画終了対象商品) */
        IF w_inst_FLG = '1' THEN
            /*  同一KEY存在チェック */
            w_inst_CK := 0;
            select
                count(*)
            into
               w_inst_CK
            from
                HNT_計画終了対象商品＿日次
            where
                商品部地域コード = w_HNW021.商品部地域コード
            and
                利用者コード = w_HNW021.利用者コード
            and
                ベンダーコード = w_HNW021.ベンダーコード
            and
                商品コード = w_HNW021.商品コード
            and
                店案内年度週ＮＯ = w_HNW021.店案内年度週ＮＯ
            ;
            IF w_inst_CK  >  0 THEN
                /*  同一週内で計画終了の取消、再登録された場合は履歴区分「0」で更新する  */
                update HNT_計画終了対象商品＿日次
                        set 商品部エリアコード = w_HNW021.商品部エリアコード,
                            商品部エリア名     = w_HNW021.商品部エリア名,
                            商品部地域名       = w_HNW021.商品部地域名,
                            くくりＶＤＲコード = w_HNW021.くくりＶＤＲコード,
                            くくりＶＤＲ名     = w_HNW021.くくりＶＤＲ名,
                            ベンダー名         = w_HNW021.ベンダー名,
                            コピー宛先１コード = w_HNW021.コピー宛先１コード,
                            宛先コード         = w_HNW021.宛先コード,
                            宛先名             = w_HNW021.宛先名,
                            センターコード     = w_HNW021.センターコード,
                            センター名         = w_HNW021.センター名,
                            ＤＥＰＴコード     = w_HNW021.ＤＥＰＴコード,
                            ＤＥＰＴ名         = w_HNW021.ＤＥＰＴ名,
                            ＣＬＡＳＳコード   = w_HNW021.ＣＬＡＳＳコード,
                            ＣＬＡＳＳ名       = w_HNW021.ＣＬＡＳＳ名,
                            商品名             = w_HNW021.商品名,
                            計画終了有効開始日 = w_HNW021.計画終了有効開始日,
                            推奨取消日         = w_HNW021.推奨取消日,
                            履歴区分           = '0',
                            不可日設定日       = w_HNW021.不可日設定日,
                            作成日時           = w_処理実施日
                        where   商品部地域コード = w_HNW021.商品部地域コード
                            and 利用者コード = w_HNW021.利用者コード
                            and ベンダーコード = w_HNW021.ベンダーコード
                            and 商品コード = w_HNW021.商品コード
                            and 店案内年度週ＮＯ = w_HNW021.店案内年度週ＮＯ
                        ;
                w_update_cnt := w_update_cnt + 1;
            ELSE
                /* 新規 計画終了商品 */
                insert into HNT_計画終了対象商品＿日次
                       values  (
                                  w_HNW021.商品部地域コード
                                  ,w_HNW021.利用者コード
                                  ,w_HNW021.ベンダーコード
                                  ,w_HNW021.商品コード
                                  ,w_HNW021.店案内年度週ＮＯ
                                  ,w_HNW021.商品部エリアコード
                                  ,w_HNW021.商品部エリア名
                                  ,w_HNW021.商品部地域名
                                  ,w_HNW021.くくりＶＤＲコード
                                  ,w_HNW021.くくりＶＤＲ名
                                  ,w_HNW021.ベンダー名
                                  ,w_HNW021.コピー宛先１コード
                                  ,w_HNW021.宛先コード
                                  ,w_HNW021.宛先名
                                  ,w_HNW021.センターコード
                                  ,w_HNW021.センター名
                                  ,w_HNW021.ＤＥＰＴコード
                                  ,w_HNW021.ＤＥＰＴ名
                                  ,w_HNW021.ＣＬＡＳＳコード
                                  ,w_HNW021.ＣＬＡＳＳ名
                                  ,w_HNW021.商品名
                                  ,w_HNW021.計画終了有効開始日
                                  ,w_HNW021.推奨取消日
                                  ,'0'
                                  ,w_HNW021.不可日設定日
                                  ,w_処理実施日
                          );
                w_insert_cnt := w_insert_cnt + 1;
                w_inst_FLG := '0';
            END IF;
        END IF;
    END LOOP;

    CLOSE HNW021_CUR;
    CLOSE HNT032_CUR;

    -- 処理件数出力
    dbms_output.put_line('[HNW021][INPUT]                : ' || w_input_cnt ||  '件') ;
    dbms_output.put_line('[HNT032][INSERT]               : ' || w_insert_cnt ||  '件') ;
    dbms_output.put_line('[HNT032][UPDATE]               : ' || w_update_cnt ||  '件') ;
    dbms_output.put_line('[HNT032][削除（履歴区分変更）] : ' || w_sakujyo_cnt ||  '件') ;

---------------------------------------------
-- 終了処理
---------------------------------------------
    select to_char(sysdate,'yyyy/mm/dd hh24:mi:ss') into w_time_end from dual;
    dbms_output.put_line('**** hnadpl440  END  : ' || w_time_end);

    -- コミット
    commit;

---------------------------------------------
-- 例外処理
---------------------------------------------
EXCEPTION
    /* 一意制約違反 */
    when DUP_VAL_ON_INDEX then
    -- ロールバック
        rollback;
    -- 戻り値のセット
        :RtnCd  :=  sqlcode;
    -- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
    -- カーソルOPENであればCLOSE
        IF HNW021_CUR%isopen THEN
            CLOSE HNW021_CUR;
        END IF;
        IF HNT032_CUR%isopen THEN
            CLOSE HNT032_CUR;
        END IF;
    -- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' hnadpl440 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('w_idx_key（KEY値）   :' || w_idx_key);
        dbms_output.put_line('**** hnadpl440 RETURN CODE :' || :RtnCd);

    /* 一意制約違反以外のエラー */
    when others then
    -- ロールバック
        rollback;
    -- 戻り値のセット
        :RtnCd  :=  sqlcode;
    -- ErrMsg SET
        :ErrMsg := substr(sqlerrm,1,100);
    -- カーソルOPENであればCLOSE
        IF HNW021_CUR%isopen THEN
            CLOSE HNW021_CUR;
        END IF;
        IF HNT032_CUR%isopen THEN
            CLOSE HNT032_CUR;
        END IF;
    -- ログ出力
        dbms_output.put_line(to_char(sysdate,'YYYY-MM-DD HH24:MI:SS') || ' hnadpl440 ABNORMAL ENDED' || ')');
        dbms_output.put_line('**** ORACLE ERROR MESSAGE  :' || :ErrMsg);
        dbms_output.put_line('**** hnadpl440 RETURN CODE :' || :RtnCd);
END;
/

EXIT  :RtnCd;
