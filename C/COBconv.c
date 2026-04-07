////////////////////////////////////////////////////////////////////////////////
//
// システム名：横断集計
//
// プログラム名：libCOBconv.a
//
// 機能概要：
//   COBOLデータ型とテキスト形式の相互変換を行う。
//   レイアウト定義とレコード領域から、複数項目を１度に変換する機能と、
//   目的別関数を使用し、１項目だけを変換する機能がある。
//
//   
// 作成日：2014/12/03
//
// 作成者：K.Shimada
//
// Ｃ言語規格：C99
//
// ソース記述形式：indent -kr -nut -l90
//
// 修正履歴：
//   yyyy/mm/dd K.Shimadaxxxxxx
//     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
//     ＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮＮ
//
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <iconv.h>


#include "COBconv.h"

#define msginf(fmt, ...) fprintf(stderr,"%s:%d:%s[INFO]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#define msgerr(fmt, ...) fprintf(stderr,"%s:%d:%s[ERROR]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#ifdef DEBUG
#define msgdbg(fmt, ...) fprintf(stderr,"%s:%d:%s[DEBUG]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#else
#define msgdbg(fmt, ...) (void)0
#endif



///////////////////////////////////////////////////////////////////////////////
// レイアウト定義に従い関数マッピングで、目的別変換関数を呼び出す
// (COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
int conv_CHAR2STR(unsigned char *cob, conv_field_t * field, char *str)
{
    int i;

    if (field->_iconv) {

        char *inbuf = (char *) cob;
        size_t inbyte = field->_byte;

        char *otbuf = str;
        size_t otbyte = (field->_byte) * 4;

        int rtn = (int) iconv(field->_cd, &inbuf, &inbyte, &otbuf, &otbyte);
        if (rtn == -1) {
            perror("iconv");
            msgerr("[%s]iconv error. name=%s pos=(%zd:%zd) success byte=%zd\n",
                   field->_label, field->name, field->_pos, field->_byte, inbyte);
            hexdump(cob, field->_byte);
            return -1;
        }
        if (inbyte != 0) {
            msgerr("[%s]iconv error remain data. name=%s pos=(%zd:%zd) remain byte=%zd\n",
                   field->_label, field->name, field->_pos, field->_byte, inbyte);
            hexdump(cob, field->_byte);
            return -1;
        }
        // iconv の引数にあるバイト数は、残バイト数なので、呼び出し前のバイト数を引く => 変換後バイト数
        i = (field->_byte) * 4 - otbyte;
    } else {
        for (i = 0; i < field->digit; i++) {
            str[i] = cob[i];
        }
    }

if (field->supress){
    // 末尾の空白を捨てる
    while (0 < i && str[i - 1] == ' ') {
        i--;
    }

    // 途中にある空白を "_" に置き換える
    for (int k = 0; k < i; k++) {
        if (str[k] == ' ') {
            str[k] = '_';
        }
    }
    // 空文字列の場合は "_"をセットする。
    if (i == 0) {
        strcpy(str, "_");
        i = 1;
    } else {
        // 文字列終端を追加する。
        str[i] = '\0';
    }
}else{
        // サプレスしない時は、文字列終端の追加だけ行う。
        str[i] = '\0';
}

    return i;
}

///////////////////////////////////////////////////////////////////////////////
// レイアウト定義に従い関数マッピングで、目的別変換関数を呼び出す
// (COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
int conv_ZONE2STR(unsigned char *cob, conv_field_t * field, char *str)
{
    return zone2str(cob, field->sign, field->digit, field->scale, str,
                    field->supress);
}

///////////////////////////////////////////////////////////////////////////////
// レイアウト定義に従い関数マッピングで、目的別変換関数を呼び出す
// (COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
int conv_PACK2STR(unsigned char *cob, conv_field_t * field, char *str)
{
    return pack2str(cob, field->sign, field->digit, field->scale, str,
                    field->supress);
}

///////////////////////////////////////////////////////////////////////////////
// レイアウト定義に従い関数マッピングで、目的別変換関数を呼び出す
// (COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
int conv_FILLER2STR(unsigned char *cob, conv_field_t * field, char *str)
{
    str[0] = '\0';
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// レイアウト定義に従い関数マッピングで、目的別変換関数を呼び出す
// (COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
int conv_STR2CHAR(unsigned char *cob, conv_field_t * field, char *str)
{
    int i = 0;
    if (field->_iconv) {

        char *inbuf = str;
        size_t inbyte = strlen(str);

        char *otbuf = (char *) cob;
        size_t otbyte = field->_byte;

        int rtn = (int) iconv(field->_cd, &inbuf, &inbyte, &otbuf, &otbyte);
        if (rtn == -1) {
            perror("iconv");
            int okbyte = strlen(str) - inbyte;
            msgerr("[%s]iconv error. name=%s pos=(%zd:%zd) success byte=%d\n",
                   field->_label, field->name, field->_pos, field->_byte, okbyte);
            hexdump(cob, field->_byte);
            return -1;
        }
        if (inbyte != 0) {
            msgerr("[%s]iconv error remain data. name=%s pos=(%zd:%zd) remain byte=%zd\n",
                   field->_label, field->name, field->_pos, field->_byte, inbyte);
            hexdump(cob, field->_byte);
            return -1;
        }
        // iconv の引数にあるバイト数は、残バイト数なので、呼び出し前のバイト数を引く => 変換後バイト数
        i = field->_byte - otbyte;
    } else {
        while (i < field->digit && str[i] != '\0') {
            cob[i] = str[i];
            i++;
        }
    }
    while (i < field->digit) {
        cob[i] = ' ';
        i++;
    }
    return field->digit;
}

///////////////////////////////////////////////////////////////////////////////
// レイアウト定義に従い関数マッピングで、目的別変換関数を呼び出す
// (COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
int conv_STR2PACK(unsigned char *cob, conv_field_t * field, char *str)
{
    return str2pack(str, cob, field->sign, field->digit, field->scale);
}

///////////////////////////////////////////////////////////////////////////////
// レイアウト定義に従い関数マッピングで、目的別変換関数を呼び出す
// (COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
int conv_STR2ZONE(unsigned char *cob, conv_field_t * field, char *str)
{
    return str2zone(str, cob, field->sign, field->digit, field->scale);
}

///////////////////////////////////////////////////////////////////////////////
// レイアウト定義に従い関数マッピングで、目的別変換関数を呼び出す
// (COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
int conv_STR2FILLER(unsigned char *cob, conv_field_t * field, char *str)
{
    for (int i = 0; i < field->digit; i++) {
        cob[i] = ' ';
    }
    return field->digit;
}


///////////////////////////////////////////////////////////////////////////////
// 【まとめて変換関数】レイアウト定義の初期化関数
///////////////////////////////////////////////////////////////////////////////
int initialize(const char *label, size_t num,   //COBOLレイアウト定義の項目数
               conv_field_t fields[],   //COBOLレイアウト定義
               const char *direction    //変換方向
    )
{

    bool flg_2str = false;
    if (strcmp(direction, "cob2str") == 0) {
        flg_2str = true;
    } else if (strcmp(direction, "str2cob") == 0) {
        flg_2str = false;
    } else {
        msgerr("[%s]unknown direction. (%s)\n", label, direction);
        return -1;
    }

    for (int i = 0; i < num; i++) {

        if (strncmp(fields[i].type, "CHAR", 4) == 0) {
            fields[i]._byte = fields[i].digit;
            fields[i]._func = (flg_2str) ? &conv_CHAR2STR : &conv_STR2CHAR;

            fields[i]._iconv = (fields[i].code == NULL) ? false : true;

            if (fields[i]._iconv) {
                char fromcode[100], tocode[100];

                if (flg_2str) {
                    strcpy(fromcode, fields[i].code);
                    strcpy(tocode, "UTF-8");
                } else {
                    strcpy(fromcode, "UTF-8");
                    strcpy(tocode, fields[i].code);
                }

                fields[i]._cd = iconv_open(tocode, fromcode);
                if (fields[i]._cd == (iconv_t) - 1) {
                    perror("iconv open");
                    msgerr("[%s]iconv open error. fromcode=%s tocode=%s\n", label,
                           fromcode, tocode);
                    return -1;
                }
            }

        } else if (strncmp(fields[i].type, "ZONE", 4) == 0) {
            fields[i]._byte = fields[i].digit + fields[i].scale;
            fields[i]._func = (flg_2str) ? &conv_ZONE2STR : &conv_STR2ZONE;
            fields[i]._iconv = false;

        } else if (strncmp(fields[i].type, "PACK", 4) == 0) {
            fields[i]._byte = (fields[i].digit + fields[i].scale) / 2 + 1;
            fields[i]._func = (flg_2str) ? &conv_PACK2STR : &conv_STR2PACK;
            fields[i]._iconv = false;

        } else if (strncmp(fields[i].type, "FILLER", 6) == 0) {
            fields[i]._byte = fields[i].digit;
            fields[i]._func = (flg_2str) ? &conv_FILLER2STR : &conv_STR2FILLER;
            fields[i]._iconv = false;

        } else {
            msgerr("[%s]unknown type. type=%s\n", label, fields[i].type);
            return -1;
        }

        if (i != 0) {
            fields[i]._pos = fields[i - 1]._pos + fields[i - 1]._byte;
        } else {
            fields[i]._pos = 0;
        }
        fields[i]._label = label;
    }

    msginf("=== [%s] %s ===\n", label, direction);
    for (int i = 0; i < num; i++) {
        msginf("%3d) %-20s\t%-6s %d (%3zdV%02zd) %d {%s} _pos=(%zd:%zd) _iconv=%d\n",
               i,
               fields[i].name, fields[i].type,
               fields[i].sign, fields[i].digit, fields[i].scale,
               fields[i].supress,
               fields[i].code, fields[i]._pos, fields[i]._byte, fields[i]._iconv);
    }
    return 0;
}
int init_cob2str(const char *label, size_t num, //COBOLレイアウト定義の項目数
                 conv_field_t fields[]  //COBOLレイアウト定義
    )
{
    return initialize(label, num, fields, "cob2str");
}

int init_str2cob(const char *label, size_t num, //COBOLレイアウト定義の項目数
                 conv_field_t fields[]  //COBOLレイアウト定義
    )
{
    return initialize(label, num, fields, "str2cob");
}

///////////////////////////////////////////////////////////////////////////////
// 【まとめて変換関数】変換実行 COBOL=>テキスト
///////////////////////////////////////////////////////////////////////////////
int conv_cob2str(const unsigned char *area,     //　COBOLデータ
                 size_t num,    //　COBOLレイアウト定義の項目数
                 conv_field_t fields[], //　COBOLレイアウト定義
                 conv_record_t * record,        //　項目分解済テキストレコード
                 bool split     // 項目分解した配列で参照するか？
    )
{

    record->num = num;
    char *out = record->str;
    int rtn;
    for (int i = 0; i < num; i++) {
        rtn = fields[i]._func((unsigned char *) area + fields[i]._pos, &fields[i]
                              , out);
        if (rtn < 0) {
            msgerr("[%s]CONV_* frunc err return.(%d)\n", fields[i]._label, rtn);
            return -1;
        }
        record->str_len[i] = (size_t) rtn;
        if (split) {
            // 項目分解した配列を参照するように指示された場合
            record->item[i] = out;
            out += (rtn + 1);
        } else {
            // 指示されなかった場合は、空白で連結した結果にする
            if (rtn != 0) {
                record->item[i] = NULL;
                if (i != (num - 1)) {
                    // 最終以外なら'\0'を空白に置き換える
                    out[rtn] = ' ';
                }
                out += (rtn + 1);
            }
        }
    }

/*
    if (split) {
        for (int i = 0; i < num; i++) {
            msginf("%s=%s\n", fields[i].name, record->item[i]);
        }
    } else {
        size_t total=0;
        for (int i = 0; i < num; i++) {
            msginf("%s\n", fields[i].name);
            total+=fields[i]._byte;
        }
        hexdump(area,       total );
        hexdump((unsigned char *)record->str, 100);
    }
*/
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// 【まとめて変換関数】変換実行 テキスト=>COBOL
///////////////////////////////////////////////////////////////////////////////
int conv_str2cob(char *str,     // テキストデータ
                 size_t num,    // COBOLレイアウト定義の項目数
                 conv_field_t fields[], // COBOLレイアウト定義
                 unsigned char *cob     // COBOLデータ
    )
{
    int total = 0;

    // 空白区切りの項目数分ループ
    int i = 0;
    for (char *f = strtok(str, " \n"); f; f = strtok(NULL, " \n")) {

        if (num == i) {
            msgerr("[%s]field num over. def=%zd real=%d\n", fields[i]._label, num, i + 1);
            return -1;
        }

        int rtn = fields[i]._func((unsigned char *) cob + fields[i]._pos, &fields[i], f);
        if (rtn < 0) {
            msgerr("[%s]CONV_* frunc err return.", fields[i]._label);
            return -1;
        }

        total += rtn;

        i++;
    }
    if (i != num) {
        msgerr("[%s]field num unmatch. def=%zd real=%d\n", fields[0]._label, num, i);
        return -1;
    }

    return total;
}

///////////////////////////////////////////////////////////////////////////////
// 【ユーティリティ関数】１６進ダンプ関数
///////////////////////////////////////////////////////////////////////////////
void hexdump(const unsigned char *area, size_t leng)
{
    static const char digit16[] =       //ダンプ用
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F'
    };

    if (5000 < leng) {
        msginf("too large leng=(%zd)\n", leng);
        return;
    }

    char *a;
    char *b;
    a = (char *) malloc((leng + 1) * 2);
    if (a == NULL) {
        msgerr("%s\n", "malloc error");
    }
    b = a + (leng + 1);

    int i = 0;
    for (i = 0; i < leng; i++) {
        a[i] = digit16[(0x0F & area[i])];
        b[i] = digit16[(0x0F & area[i] >> 4)];
    }
    a[i] = '\0';
    b[i] = '\0';
    msginf("%s\n", b);
    msginf("%s\n", a);
    free(a);
}

///////////////////////////////////////////////////////////////////////////////
// 【ユーティリティ関数】文字列から１６進変換関数
///////////////////////////////////////////////////////////////////////////////
size_t tohex(const char *s, unsigned char *area)
{
    int j = 0;
    bool flg = true;

    for (int i = 0; s[i] != '\0'; i++) {
        unsigned char c;
        if ('0' <= s[i] && s[i] <= '9') {
            c = '0';
        } else if ('A' <= s[i] && s[i] <= 'F') {
            c = 'A' - 10;
        } else if ('a' <= s[i] && s[i] <= 'f') {
            c = 'a' - 10;
        } else {
            c = s[i];
        }

        if (flg) {
            area[j] = ((s[i] - c) << 4);
        } else {
            area[j] = area[j] + (s[i] - c);
            j++;
        }

        flg = !flg;
    }
    return j;
}

///////////////////////////////////////////////////////////////////////////////
// 【目的別変換関数】ゾーン１０進のテキスト化関数
///////////////////////////////////////////////////////////////////////////////
int zone2str(const unsigned char *zone, bool sign, size_t digit,
             size_t scale, char *str, bool supress)
{

    size_t p;
    p = scale + digit;
    if (p < 1) {
        msgerr("precision is zero. digit=%zd,scale=%zd\n", digit, scale);
        return -1;
    }

    int j = 0;
    if (sign) {
        unsigned char last = zone[p - 1];
        //最終バイトの上位４ビットが 'D' の時のみ負値。
        // 正値が 'F' / 'C' どっちでもＯＫにする
        if ((last & 0xD0) == 0xD0) {
            // 負の値の時だけ符号付加
            str[j++] = '-';
        }
    }

    int i = 0;                  //桁カウンタ
    bool skip = true;
    while (i < p) {

        //小数点編集
        if (digit == i) {
            skip = false;
            str[j++] = '.';
        }
        //１０進数の取り出し
        //上位４ビットは無視し数値を取り出す(ASCII/EBCDIC両方OK)
        unsigned char n = 0x0F & zone[i];

        //結果編集
        if (supress && n == 0) {
            if (!skip) {
                str[j++] = '0' + n;
            }
        } else {
            skip = false;
            str[j++] = '0' + n;
        }

        i++;
    }

    //ゼロサプレス指定で、値がゼロの場合は、何も編集されていない。
    if (supress) {
        if ((sign && (j == 0 || strcmp(str, "-") == 0))
            || (!sign && j == 0)
            ) {
            str[j++] = '0';
        }
    }


    str[j] = '\0';
    return j;
}

///////////////////////////////////////////////////////////////////////////////
// 【目的別変換関数】パック１０進のテキスト化関数
///////////////////////////////////////////////////////////////////////////////
int pack2str(const unsigned char *pack, bool sign, size_t digit,
             size_t scale, char *str, bool supress)
{
    size_t p;
    p = scale + digit;
    if (p < 1) {
        msgerr("precision is zero. digit=%zd,scale=%zd\n", digit, scale);
        return -1;
    }

    div_t d = div(p, 2);
    int j = 0;
    if (sign) {
        unsigned char last = pack[((size_t) d.quot + 1) - 1];
        //最終バイトの下位４ビットが 'D' の時のみ負値。
        // 正値が 'F' / 'C' どっちでもＯＫにする
        if ((last & 0x0D) == 0x0D) {
            // 負の値の時だけ符号付加
            str[j++] = '-';
        }
    }

    int i = 0;                  //桁カウンタ
    int b = 0;                  //バイト位置カウンタ
    bool flg = ((d.rem == 0) ? false : true);   //true:上位４ビット false:下位４ビット.偶数は最初の１桁を無視
    bool skip = true;
    while (i < p) {

        //小数点編集
        if (digit == i) {
            skip = false;
            str[j++] = '.';
        }
        //１０進数の取り出し
        unsigned char n;
        if (flg) {
            n = (0xF0 & pack[b]) >> 4;
        } else {
            n = (0x0F & pack[b]);
            b++;                //次の上位４ビット向けにバイト位置カウンタを進める
        }

        //結果編集
        if (supress && n == 0) {
            if (!skip) {
                str[j++] = '0' + n;
            }
        } else {
            skip = false;
            str[j++] = '0' + n;
        }

        flg = !flg;
        i++;
    }

    //ゼロサプレス指定で、値がゼロの場合は、何も編集されていない。
    if (supress) {
        if ((sign && (j == 0 || strcmp(str, "-") == 0))
            || (!sign && j == 0)
            ) {
            str[j++] = '0';
        }
    }

    str[j] = '\0';
    return j;
}

///////////////////////////////////////////////////////////////////////////////
// テキストの数値文字列を解析結果(COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
typedef struct split_reslut {
    int sign;                   // 解析結果の符号
    int point_idx;              // 解析結果の小数点位置

    unsigned char d[18];        // 解析した整数部 COBOLの規格の１８桁まで対応
    unsigned char s[18];        // 解析した小数部 COBOLの規格の１８桁まで対応
    int d_cnt;                  // 解析した整数部桁数
    int s_cnt;                  // 解析した小数部桁数
} split_reslut_t;

///////////////////////////////////////////////////////////////////////////////
// テキストの数値文字列を解析する関数(COBconv内部関数であり外部に公開しない)
///////////////////////////////////////////////////////////////////////////////
int split_digitANDscale(const char *str,        //テキスト文字列
                        split_reslut_t * rslt)
{
    // 解析結果の初期化
    rslt->sign = 0;
    rslt->point_idx = 0;

    rslt->d_cnt = 0;
    rslt->s_cnt = 0;

    memset(rslt->d, 0, sizeof(rslt->d));
    memset(rslt->s, 0, sizeof(rslt->s));

    // 符号の検出と整数部/小数部の分離
    int i = 0;
    while (str[i] != '\0') {
        if ('0' <= str[i] && str[i] <= '9') {
            if (rslt->point_idx == 0) {
                // 整数部の場合
                if (rslt->d_cnt == 0 && str[i] == '0') {
                    // 整数部の前ゼロは捨てる
                } else {
                    rslt->d[rslt->d_cnt++] = (0x0F & str[i]);
                }
            } else {
                // 小数部の場合
                rslt->s[rslt->s_cnt++] = (0x0F & str[i]);
            }

        } else if (str[i] == '-') {
            if (rslt->sign != 0) {
                msgerr("sign approve tow times. pos=%d\n", i);
                return -1;
            }
            rslt->sign = -1;

        } else if (str[i] == '+') {
            if (rslt->sign != 0) {
                msgerr("sign approve tow times. pos=%d\n", i);
                return -1;
            }
            rslt->sign = 1;

        } else if (str[i] == '.') {
            if (rslt->point_idx != 0) {
                msgerr("point approve tow times. pos=%d\n", i);
                return -1;
            }
            rslt->point_idx = i;
        } else {
            msgerr("illgal char. pos=%d,char=%x\n", i, str[i]);
            return -1;
        }
        i++;
    }
    if (rslt->sign == 0) {
        rslt->sign = 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// 【目的別変換関数】テキストのパック１０進化関数
///////////////////////////////////////////////////////////////////////////////
int str2pack(const char *str,   //テキスト文字列
             unsigned char *pack,       // COBOLデータ型領域
             bool sign,         // COBOLデータ型の符号有無
             size_t digit,      // 整数部桁数
             size_t scale       // 小数部桁数
    )
{

    if (digit > 18 || scale > 18) {
        msgerr("digit or scale is over 18. digit=%zd,scale=%zd\n", digit, scale);
        return -1;
    }
    if (digit + scale < 1) {
        msgerr("precision is zero. digit=%zd,scale=%zd\n", digit, scale);
        return -1;
    }
    // 文字列の解析
    split_reslut_t rslt;
    if (split_digitANDscale(str, &rslt) < 0) {
        msgerr("string check error.%s\n", "");
        return -1;
    }

    if (digit < rslt.d_cnt) {
        msgerr("over flow digit. max=%zd real=%d\n", digit, rslt.d_cnt);
        return -1;
    }
    // 結果編集
    int size = ((digit + scale) / 2) + 1;
    memset(pack, 0, size);
    int j = size - 1;
    bool flg = true;            // true:上位４ビット false:下位４ビット

    for (int i = scale - 1; -1 < i; i--) {
        if (flg) {
            pack[j] += (rslt.s[i] << 4);
            j--;
        } else {
            pack[j] += rslt.s[i];
        }
        flg = !flg;
    }

    for (int i = rslt.d_cnt - 1; -1 < i; i--) {
        if (flg) {
            pack[j] += (rslt.d[i] << 4);
            j--;
        } else {
            pack[j] += rslt.d[i];
        }
        flg = !flg;
    }

    // 符号の編集
    pack[size - 1] |= (sign && rslt.sign == -1) ? 0x0D : 0x0c;

    return size;
}

///////////////////////////////////////////////////////////////////////////////
// 【目的別変換関数】テキストのゾーン１０進化関数
///////////////////////////////////////////////////////////////////////////////
int str2zone(const char *str,   //テキスト文字列
             unsigned char *zone,       // COBOLデータ型領域
             bool sign,         // COBOLデータ型の符号有無
             size_t digit,      // 整数部桁数
             size_t scale       // 小数部桁数
    )
{

    if (digit > 18 || scale > 18) {
        msgerr("digit or scale is over 18. digit=%zd,scale=%zd\n", digit, scale);
        return -1;
    }
    if (digit + scale < 1) {
        msgerr("precision is zero. digit=%zd,scale=%zd\n", digit, scale);
        return -1;
    }
    // 文字列の解析
    split_reslut_t rslt;
    if (split_digitANDscale(str, &rslt) < 0) {
        msgerr("string check error.%s\n", "");
        return -1;
    }

    if (digit < rslt.d_cnt) {
        msgerr("over flow digit. max=%zd real=%d\n", digit, rslt.d_cnt);
        return -1;
    }
    // 結果編集
    int j = 0, i = 0;
    for (i = 0; i < digit - rslt.d_cnt; i++) {
        zone[j++] = '0';
    }
    for (i = 0; i < rslt.d_cnt; i++) {
        zone[j++] = '0' + rslt.d[i];
    }
    for (i = 0; i < scale; i++) {
        zone[j++] = '0' + rslt.s[i];
    }

    static const unsigned char ZONE10MASK = 0x30;       // 符号有の正値は、符号無と同じにする。
    //static const unsigned char ZONE10MASK = 0xC0; //0xC0にしたい時もあるはず…

    if (sign) {
        zone[j - 1] &= 0x0F;    //上位４ビットクリア
        zone[j - 1] |= (rslt.sign == -1) ? 0xD0 : ZONE10MASK;
    }

    return j;
}


////////////////////////////////////////////////////////////////////////////////
// レイアウト定義ファイルの取り込み
////////////////////////////////////////////////////////////////////////////////
char pa_line[2048];
// strdupは、C99範囲外の関数なので、コンパイル時警告が出る。
//   -stdをGNUに変更してもよいが、ここで直接関数型を宣言する。
char *strdup(const char *s);

int read_layout(const char *param_file, // レイアウト定義ファイル
int *para_num,  // 呼び出す時は、用意したレイアウト定義の項目数をセット。ファイルに定義のあった項目数に書き換えて復帰する.
                conv_field_t para[] //レイアウト定義
    )
{

    FILE *pa_fp;
    if ((pa_fp = fopen(param_file, "r")) == NULL) {
        msgerr("読み取り不可(%s)\n", param_file);
        return -1;
    }

    int i = 0;

    while (fgets(pa_line, sizeof(pa_line), pa_fp) != NULL) {
        if ( pa_line[0]=='#' ){
            continue;
        }

        if (*para_num == i) {
            msgerr
                ("パラメータファイルの項目数に達した。定義=%d 実際=%d\n",
                 *para_num, i + 1);
            return -1;
        }
        char *p;
        char split[] = " ,\t\n";
        char *endptr;

        // ----------------------------------------------------------
        //    char *name;                 // 項目名称
        if ((p = strtok(pa_line, split)) == NULL) {
            msgerr("[項目名称]がありません LINE=%d\n", i + 1);
            return -1;
        }
        para[i].name = strdup(p);

        // ----------------------------------------------------------
        // char *type;                 // 変換パターン CHAR/ZONE/PACK/FILLER
        if ((p = strtok(NULL, split)) == NULL) {
            msgerr("[変換パターン]がありません LINE=%d\n", i + 1);
            return -1;
        }
        if (strcmp(p, "CHAR") == 0 || strcmp(p, "ZONE") == 0 || strcmp(p, "PACK")
            || strcmp(p, "FILLER")) {
        } else {
            msgerr
                ("[変換パターン]は CHAR ZONE PACK FILLER のいづれかを指定してください LINE=%d\n",
                 i + 1);
            return -1;
        }
        para[i].type = strdup(p);

        // ----------------------------------------------------------
        // bool sign;                  // 符号付きの数字項目か？
        if ((p = strtok(NULL, split)) == NULL) {
            msgerr("[符号付きの数字項目か？]がありません LINE=%d\n",
                   i + 1);
            return -1;
        }
        if (strcmp(p, "true") == 0 || strcmp(p, "false") == 0) {
        } else {
            msgerr
                ("[符号付きの数字項目か？]は true false のいづれかを指定してください LINE=%d\n",
                 i + 1);
            return -1;
        }
        para[i].sign = (strcmp(p, "true") == 0) ? true : false;

        // ----------------------------------------------------------
        // size_t digit;               // 英数字項目バイト数 or 整数部桁数
        if ((p = strtok(NULL, split)) == NULL) {
            msgerr("[桁数]がありません LINE=%d\n", i + 1);
            return -1;
        }

        para[i].digit = strtol(p, &endptr, 10);
        if (*endptr != '\0') {
            msgerr("[桁数]は 数値を指定してください LINE=%d\n", i + 1);
            return -1;
        }
        if (para[i].digit < 1) {
            msgerr("[桁数]は １以上の整数を指定してください LINE=%d\n",
                   i + 1);
            return -1;
        }
        // ----------------------------------------------------------
        // size_t scale;               // 小数部桁数 ZONR or PACK の時のみ有効
        if ((p = strtok(NULL, split)) == NULL) {
            msgerr("[小数桁数]がありません LINE=%d\n", i + 1);
            return -1;
        }
        para[i].scale = strtol(p, &endptr, 10);
        if (*endptr != '\0') {
            msgerr("[桁数]は 数値を指定してください LINE=%d\n", i + 1);
            return -1;
        }
        if (para[i].scale < 0) {
            msgerr("[桁数]は ０以上の整数を指定してください LINE=%d\n",
                   i + 1);
            return -1;
        }
        // ----------------------------------------------------------
        // bool supress;           // ZONE/PACKをテキスト化する時、先頭のゼロサプレスをするか？
        if ((p = strtok(NULL, split)) == NULL) {
            msgerr
                ("[PACK/ZONEの時ゼロサプレスをするか？]がありません LINE=%d\n",
                 i + 1);
            return -1;
        }
        if (strcmp(p, "true") == 0 || strcmp(p, "false") == 0) {
        } else {
            msgerr
                ("[PACK/ZONEの時ゼロサプレスをするか？]は true false のいづれかを指定してください LINE=%d\n",
                 i + 1);
            return -1;
        }
        para[i].supress = (strcmp(p, "true") == 0) ? true : false;


        // ----------------------------------------------------------
        // char *code;                 // 【省略可能】文字コード変換する場合の文字コード
        if ((p = strtok(NULL, split)) != NULL) {
            para[i].code = strdup(p);
        } else {
            para[i].code = NULL;
        }

        i++;
    }

    fclose(pa_fp);

   *para_num = i;

    return 0;

}
