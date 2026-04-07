////////////////////////////////////////////////////////////////////////////////
//
// システム名：横断集計
//
// プログラム名：v_fixlength
//
// 機能概要：
//   引数に指定したファイルのフィールドを、各フィールドの最大レングスに合わせた桁数の固定長フィールドに変換（半角スペースで補完）して
//   出力する。
//   入力はファイルまたは標準入力で渡す。
//
// 作成日：2014/12/15
//
// 作成者：HUNG.LV
//
// Ｃ言語規格：C99
//
// ソース記述形式：indent -kr -nut -l90
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <wchar.h>
#include <unistd.h>
#include <stdbool.h>

#define MAXSIZE_LINE	4096*16
#define MAXNUM_FIELDS	512
#define MAXSIZE_FIELD   1024
#define MAXSIZE_OPTION  1024

#define msginf(fmt, ...) fprintf(stderr,"%s:%d:%s[INFO]"  fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#define msgerr(fmt, ...) fprintf(stderr,"%s:%d:%s[ERROR]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#ifdef DEBUG
#define msgdbg(fmt, ...) fprintf(stderr,"%s:%d:%s[DEBUG]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#else
#define msgdbg(fmt, ...) (void)0
#endif

int max_length_fields[MAXNUM_FIELDS];   // Length of the longest fields
char input_file[MAXSIZE_OPTION];        // Input file name
char empty_fields[MAXSIZE_FIELD];       // A empty fields which use to add more space " "
FILE *temp_length_fields;       // A temporary file which store length of fields
int format_output = 0;          // Format output, 0:left;1:right

/* Japanese 1 byte */
char kana_one[63][4] = { {"ﾊ"}, {"ﾋ"}, {"ﾌ"}, {"ﾍ"}, {"ﾎ"}, {"ｱ"},
{"ｲ"}, {"ｳ"}, {"ｴ"}, {"ｵ"}, {"ｶ"}, {"ｷ"}, {"ｸ"}, {"ｹ"},
{"ｺ"}, {"ｻ"}, {"ｼ"}, {"ｽ"}, {"ｾ"}, {"ｿ"}, {"ﾀ"}, {"ﾁ"},
{"ﾂ"}, {"ﾃ"}, {"ﾄ"}, {"ﾅ"}, {"ﾆ"}, {"ﾇ"}, {"ﾈ"}, {"ﾉ"},
{"ﾏ"}, {"ﾐ"}, {"ﾑ"}, {"ﾒ"}, {"ﾓ"}, {"ﾔ"}, {"ﾕ"}, {"ﾖ"},
{"ﾗ"}, {"ﾘ"}, {"ﾙ"}, {"ﾚ"}, {"ﾛ"}, {"ﾜ"}, {"ｦ"}, {"ﾝ"},
{"ｧ"}, {"ｨ"}, {"ｩ"}, {"ｪ"}, {"ｫ"}, {"ｬ"}, {"ｭ"}, {"ｮ"},
{"ｯ"}, {"｡"}, {"｢"}, {"｣"}, {"､"}, {"･"}, {"ｰ"}, {"ﾞ"},
{"ﾟ"}
};

////////////////////////////////////////////////////////////////////////////////
// Check if character is ASCII 1 byte
////////////////////////////////////////////////////////////////////////////////
bool is_asc_one_byte(char c)
{

    int i_char = c;
    if (i_char > 31 && i_char < 127) {
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// Check if character is Japanese 1 byte
////////////////////////////////////////////////////////////////////////////////
bool is_japanese_one_byte(char *c)
{
    for (int i = 0; i < 63; i++) {
        if (strcmp(c, kana_one[i]) == 0) {
            return true;
        }
    }

    return false;

}

////////////////////////////////////////////////////////////////////////////////
// Count the length of line
////////////////////////////////////////////////////////////////////////////////
int count_length(char *line)
{
    int len = 0;
    int line_byte_len = strlen(line);
    for (int i = 0; i < line_byte_len; i++) {

        // Check if there is ASCII character 1 byte
        if (is_asc_one_byte(line[i])) {
            len++;
            continue;
        }
        // Check if there is Japanese character 1 byte
        char c[4];
        c[3] = '\0';
        // Get 3 consecutive characters
        strncpy(c, &line[i], 3);

        // Ignore enter character
        if (!strcmp(c, "\n")) {
            continue;
        }

        if (is_japanese_one_byte(c)) {
            len++;
            i += 2;
            continue;
        } else {
            // There are characters 2 byte
            len += 2;
            i += 2;
            continue;
        }
    }

    return len;
}

////////////////////////////////////////////////////////////////////////////////
// 引数解析
////////////////////////////////////////////////////////////////////////////////
int analyze_option(int argc, char *argv[])
{
    if (argc == 1) {
        msgerr("引数が指定されていません%s\n", "");
        return 1;
    }

    if (argc > 3) {
        msgerr("引数は2個以下を指定してください%s\n", "");
        return 1;
    }

    input_file[0] = '\0';

    // Get format output
    char c;
    while ((c = getopt(argc, argv, "l")) != -1) {
        msgdbg("c=%c optind=%d optopt=%c\n", c, optind, optopt);
        switch (c) {
        case 'l':
            format_output = 1;
            break;
        default:
            msgerr("許されない引数が指定されました %s\n", "");
            return 1;
        }
    }

    // Get input file name
    if (optind < argc) {
        strncpy(input_file, argv[optind], sizeof(input_file) - 1);
        input_file[sizeof(input_file) - 1] = '\0';
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Calculate v_fixlength for a line from input file
////////////////////////////////////////////////////////////////////////////////
void calv_fixlength(char *input_line, char *output_line)
{

    output_line[0] = '\0';

    // Get length of fields from temporary file
    int temp_column = 0;
    int length_fields[MAXNUM_FIELDS];
    char in_line_temp[MAXSIZE_LINE];
    if (fgets(in_line_temp, sizeof(in_line_temp), temp_length_fields) != NULL) {
        for (char *f = strtok(in_line_temp, " \n"); f; f = strtok(NULL, " \n")) {

            length_fields[temp_column] = atol(f);

            temp_column++;
        }
    }
    // Get fields from input line
    int input_column = 0;
    for (char *f = strtok(input_line, " \n"); f; f = strtok(NULL, " \n")) {

        // If this is not first field
        if (input_column > 0) {
            strcat(output_line, " ");
        }
        // Check output format and add fields to output line.
        if (format_output) {
            strcat(output_line, f);
            strncat(output_line, empty_fields,
                    max_length_fields[input_column] - length_fields[input_column]);
        } else {
            strncat(output_line, empty_fields,
                    max_length_fields[input_column] - length_fields[input_column]);
            strcat(output_line, f);
        }

        input_column++;
    }

    strcat(output_line, "\n");
}

////////////////////////////////////////////////////////////////////////////////
// Create temporary files
////////////////////////////////////////////////////////////////////////////////
FILE *create_tempfile()
{

    FILE *tmp_fp;
    char temp_file[MAXSIZE_OPTION];
    char tpl_line[MAXSIZE_LINE];

    sprintf(temp_file, "/tmp/%d-v_fixlength", getpid());
    tmp_fp = fopen(temp_file, "w+");

    // Call unlink so that whenever the file is closed or the program exited,
    // the temporary file will be deleted
    unlink(temp_file);
    while (fgets(tpl_line, sizeof(tpl_line), stdin) != NULL) {

        // Store input line to temporary file
        fputs(tpl_line, tmp_fp);

        // Get fields from input line
        int input_column = 0;

        for (char *f = strtok(tpl_line, " \n"); f; f = strtok(NULL, " \n")) {

            // Calculate to get length of the longest fields.
            int len_field = count_length(f);

            if (max_length_fields[input_column] < len_field) {
                max_length_fields[input_column] = len_field;
            }
            // Store length of fields to temporary file
            if (input_column > 0) {
                fprintf(temp_length_fields, "%s", " ");
            }

            fprintf(temp_length_fields, "%d", len_field);

            input_column++;
        }

        fprintf(temp_length_fields, "%s", "\n");

    }

    return tmp_fp;
}

FILE *create_length_tempfile()
{
    FILE *tmp_fp;
    char temp_file[MAXSIZE_OPTION];

    sprintf(temp_file, "/tmp/%d-v_fixlength-length", getpid());
    tmp_fp = fopen(temp_file, "w+");
    // Call unlink so that whenever the file is closed or the program exited,
    // the temporary file will be deleted
    unlink(temp_file);
    return tmp_fp;
}

////////////////////////////////////////////////////////////////////////////////
// メイン
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

    msginf("%s\n", "開始");

    // 引数解析
    if (analyze_option(argc, argv) != 0) {
        msgerr("異常終了 引数の解析でエラーが発生%s\n", "");
        return (1);
    }
    // ファイル準備
    FILE *in_fp, *ot_fp;
    char output_line[MAXSIZE_LINE];

    if (strcmp(input_file, "") == 0 || strcmp(input_file, "-") == 0) {

        temp_length_fields = create_length_tempfile();
        in_fp = create_tempfile();

    } else {

        if ((in_fp = fopen(input_file, "r")) == NULL) {
            msgerr("異常終了 入力ファイルが読み取り不可(%s)\n",
                   input_file);
            return (1);
        }
        // Calculate to get length of the longest fields.
        char in_line[MAXSIZE_LINE];
        temp_length_fields = create_length_tempfile();
        while (fgets(in_line, sizeof(in_line), in_fp) != NULL) {

            // Get fields from input line
            int input_column = 0;

            for (char *f = strtok(in_line, " \n"); f; f = strtok(NULL, " \n")) {

                // Calculate to get length of the longest fields.
                int len_field = count_length(f);

                if (max_length_fields[input_column] < len_field) {
                    max_length_fields[input_column] = len_field;
                }
                // Store length of fields to temporary file
                if (input_column > 0) {
                    fprintf(temp_length_fields, "%s", " ");
                }

                fprintf(temp_length_fields, "%d", len_field);

                input_column++;
            }

            fprintf(temp_length_fields, "%s", "\n");
        }
    }

    // Move file pointer to top of file
    fseek(in_fp, 0, SEEK_SET);
    fseek(temp_length_fields, 0, SEEK_SET);

    ot_fp = stdout;

    // Create empty fields
    for (int i = 0; i < MAXSIZE_FIELD; i++) {
        empty_fields[i] = ' ';
    }
    empty_fields[MAXSIZE_FIELD - 1] = '\0';

    // ファイル読み取りループ
    char in_line_input[MAXSIZE_LINE];
    int num_line = 0;

    while (fgets(in_line_input, sizeof(in_line_input), in_fp) != NULL) {

        num_line++;

        calv_fixlength(in_line_input, output_line);
        if ((fputs(output_line, ot_fp)) == EOF) {
            msgerr("異常終了 結果出力でエラーが発生(LINE=%d)\n", num_line);
            return 1;
        }


    }

    fclose(in_fp);
    fclose(temp_length_fields);

    // 正常終了
    msginf("%s\n", "正常終了");
    return 0;
}
