#ifndef _COBconv
#define _COBconv

#include <stdbool.h>
#include <iconv.h>

#define MAXNUM_FIELD  512
#define MAXSIZE_RECORD 1024*5

typedef struct conv_field conv_field_t;
typedef struct conv_record conv_record_t;
typedef int (*conv_func_t) (unsigned char *cob, conv_field_t * fields, char *str);

struct conv_field {
    char *name;                 // 項目名称
    char *type;                 // 変換パターン CHAR/ZONE/PACK/FILLER
    bool sign;                  // 符号付きの数字項目か？
    size_t digit;               // 英数字項目バイト数 or 整数部桁数
    size_t scale;               // 小数部桁数 ZONR or PACK の時のみ有効
    bool supress;           // ZONE/PACKをテキスト化する時、先頭のゼロサプレスをするか？ 文字列をテキスト化する時、空白をサプレスするか？
    char *code;                 // 【省略可能】文字コード変換する場合の文字コード

    // 下記は initialize で設定される変換ライブラリの内部情報
    size_t _pos;                //　レイアウト上の相対バイト位置
    size_t _byte;               // ＣＯＢＯＬ表現のバイト数
    conv_func_t _func;          //　変換パターンに応じた変換関数
    const char *_label;         // レイアウトにつけた名前。異常時に識別するコメント
    bool _iconv;                // iconvで変換を行うか？
    iconv_t _cd;                // iconv用にopenしたfiledescriptor
};


struct conv_record {
    size_t num;                 // 項目数
    char *item[MAXNUM_FIELD];   // テキスト化項目
    size_t str_len[MAXNUM_FIELD];       // テキスト化項目の長さ
    char str[MAXSIZE_RECORD];   // テキスト化１レコード
};

///////////////////////////////////////////////////////////////////////////////
// COBOLレイアウト定義の初期化関数
///////////////////////////////////////////////////////////////////////////////
int init_cob2str(const char *label,     // レイアウトにつけた名前。異常時に識別するコメント
                 size_t num,    //　COBOLレイアウト定義の項目数
                 conv_field_t fields[]  //　COBOLレイアウト定義
    );

int init_str2cob(const char *label,     // レイアウトにつけた名前。異常時に識別するコメント
                 size_t num,    //　COBOLレイアウト定義の項目数
                 conv_field_t fields[]  //　COBOLレイアウト定義
    );

///////////////////////////////////////////////////////////////////////////////
// レコード変換関数
///////////////////////////////////////////////////////////////////////////////
int conv_cob2str(const unsigned char *cob,      // COBOLデータ
                 size_t num,    // COBOLレイアウト定義の項目数
                 conv_field_t fields[], // COBOLレイアウト定義
                 conv_record_t * record,        // 項目分解済テキストレコード
                 bool split     // 項目分解した配列で参照するか？
    );
int conv_str2cob(char *str,     // テキストデータ
                 size_t num,    // COBOLレイアウト定義の項目数
                 conv_field_t fields[], // COBOLレイアウト定義
                 unsigned char *cob     // COBOLデータ
    );

///////////////////////////////////////////////////////////////////////////////
// レイアウト定義パラメータファイル読み込み関数
///////////////////////////////////////////////////////////////////////////////
int read_layout(const char *param_file, // レイアウト定義ファイル名
                int *para_num,  // COBOLレイアウト定義の項目数
                conv_field_t para[]     //COBOLレイアウト定義
    );



///////////////////////////////////////////////////////////////////////////////
// １６進ダンプ関数
///////////////////////////////////////////////////////////////////////////////
void hexdump(const unsigned char *area, // ダンプ領域
             size_t byte        // バイト数
    );

///////////////////////////////////////////////////////////////////////////////
// 文字列から１６進変換関数
///////////////////////////////////////////////////////////////////////////////
size_t tohex(const char *s,     // １６進文字列
             unsigned char *area        //　変換後領域
    );

///////////////////////////////////////////////////////////////////////////////
// 英数字項目のテキスト化関数
///////////////////////////////////////////////////////////////////////////////
int x2str(const unsigned char *zone     // COBOLデータ型領域
    );

///////////////////////////////////////////////////////////////////////////////
// ゾーン１０進のテキスト化関数
///////////////////////////////////////////////////////////////////////////////
int zone2str(const unsigned char *zone, // COBOLデータ型領域
             bool sign,         // COBOLデータ型の符号有無
             size_t digit,      // 整数部桁数
             size_t scale,      // 小数部桁数
             char *str,         //テキスト化文字列
             bool supress   // 先頭のゼロサプレスをするか？
    );

///////////////////////////////////////////////////////////////////////////////
// パック１０進のテキスト化関数
///////////////////////////////////////////////////////////////////////////////
int pack2str(const unsigned char *pack, // COBOLデータ型領域
             bool sign,         // COBOLデータ型の符号有無
             size_t digit,      // 整数部桁数
             size_t scale,      // 小数部桁数
             char *str,         //テキスト化文字列
             bool supress   // 先頭のゼロサプレスをするか？
    );

///////////////////////////////////////////////////////////////////////////////
// テキストのパック１０進化関数
///////////////////////////////////////////////////////////////////////////////
int str2pack(const char *str,   //テキスト文字列
             unsigned char *pack,       // COBOLデータ型領域
             bool sign,         // COBOLデータ型の符号有無
             size_t digit,      // 整数部桁数
             size_t scale       // 小数部桁数
    );

///////////////////////////////////////////////////////////////////////////////
// テキストのゾーン１０進化関数
///////////////////////////////////////////////////////////////////////////////
int str2zone(const char *str,   //テキスト文字列
             unsigned char *pack,       // COBOLデータ型領域
             bool sign,         // COBOLデータ型の符号有無
             size_t digit,      // 整数部桁数
             size_t scale       // 小数部桁数
    );


#endif                          //_COBconv
