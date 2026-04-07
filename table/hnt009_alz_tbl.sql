/* C³—š—ð                                                    */
/* ‚m‚   C³“ú•t   C³ŽÒ     C³“à—e                       */
/*   1  2025/11/18  k.takashima LMDCˆÚs‘Î‰žFANALYZE”ñŒÝŠ·‘Î‰ž */
--ANALYZE TABLE HNT_‚c‚d‚o‚s ESTIMATE STATISTICS;
exec dbms_stats.gather_table_stats(ownname=>user,tabname=>'HNT_‚c‚d‚o‚s');