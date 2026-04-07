/*=========================================
System name     : DIS Utility
Header filename : comm_msg.h
Infomation      : 共通定義モジュール
==========================================*/
/*<Include Begin>**************************/
/*<Include End>****************************/

/*<Define Begin>***************************/
/*<Define End>*****************************/

/*<Grobal Begin>***************************/
typedef struct{
	char msg[50];
} err_info;

err_info err_msg[] = {
	{""},									/*	正常終了				*/
	{""},									/*	警告終了				*/
	{"アーギュメントに不正があります"},		/*	アーギュメント不正		*/
	{"ファイルオープンエラー"},				/*	ファイルオープンエラー	*/
	{"ファイルリードエラー"},				/*	ファイルリードエラー	*/
	{"メモリー確保エラー"},					/*	メモリー確保エラー		*/
	{"Too many lines"},
	{"Too many record"},
	{"Fild split error"},
	{"return code change error"}
};
/*<Grobal End>*****************************/

/*<PrivatePrototype Begin>*****************/
/*<PrivatePrototype End>*******************/

