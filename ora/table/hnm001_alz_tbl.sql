/* 修正履歴                                                    */
/* Ｎｏ   修正日付   修正者     修正内容                       */
exec dbms_stats.gather_table_stats(ownname=>user,tabname=>'HNM_メニュー情報');
