--ＤＢの不要パッケージ用削除 ORACLE命令
--
--権限のあるユーザー（C##VINXnn)で実行すること
--

--元のPackageシノニム確認
select * from ALL_SYNONYMS  where table_name = 'SMZZOP001';
select * from ALL_SYNONYMS  where table_name = 'SMZZOP102';
select * from ALL_SYNONYMS  where table_name = 'SFZZOP001';
select * from ALL_SYNONYMS  where table_name = 'SMZZOP021';

--「レコードが選択されませんでした。」が出力された場合は存在しない

--元のPackageシノニム削除（旧名（上記で権限内容の出力）があれば削除）
DROP SYNONYM HHN001.SMZZOP001
DROP SYNONYM HHN001.SMZZOP102
DROP SYNONYM OHN001.SFZZOP001
DROP SYNONYM OHN001.SMZZOP021

