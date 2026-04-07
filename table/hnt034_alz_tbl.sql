/* 修正履歴                                                    */
/* Ｎｏ   修正日付   修正者     修正内容                       */
/*   1    2025/11/18 r.someya  LMDC移行対応：ANALYZE非互換対応 */
--ANALYZE TABLE HNT_在庫切れ速報 ESTIMATE STATISTICS;
exec dbms_stats.gather_table_stats(ownname=>user,tabname=>'HNT_在庫切れ速報');
