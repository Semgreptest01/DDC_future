////////////////////////////////////////////////////////////////////////////////
//
// システム名：横断集計
//
// プログラム名：v_sidecat
//
// 機能概要：
//   指定した[マスタ入力ファイル]と[トラン入力ファイル]を横に連結する。
//   連結する際は各入力ファイルのレコード順に単純結合する。
//   出力は[マスタ入力ファイル]の右横に[トラン入力ファイル]を連結して、標準出力に書き出す。
//   [トラン入力ファイル]は、[マスタ入力ファイル]の最大レコード長から１バイトのスペースを挟んで右に連結する。
//
// 作成日：2014/12/08
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
#include <wchar.h>
#include <unistd.h>

#define MAXSIZE_LINE    4096*16
#define MAXSIZE_OPTION  1024

#define msginf(fmt, ...) fprintf(stderr,"%s:%d:%s[INFO]"  fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#define msgerr(fmt, ...) fprintf(stderr,"%s:%d:%s[ERROR]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#ifdef DEBUG
#define msgdbg(fmt, ...) fprintf(stderr,"%s:%d:%s[DEBUG]" fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#else
#define msgdbg(fmt, ...) (void)0
#endif

int max_length_master;          // Length of longest line in master file
int max_length_tran;            // Length of longest line in tran file
char input_file_master[MAXSIZE_OPTION]; // マスタ入力ファイルパス名
char input_file_tran[MAXSIZE_OPTION];   // トラン入力ファイルパス名
char empty_line[MAXSIZE_LINE];
FILE *temp_length_masterfile;
FILE *temp_length_tranfile;

/* Japanese 1 byte */
char kana_one[100][4] = { {"ﾊ"}, {"ﾋ"}, {"ﾌ"}, {"ﾍ"}, {"ﾎ"}, {"ｱ"},
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
int is_asc_one_byte(char c)
{

    int i_char = c;
    if (i_char > 31 && i_char < 127) {
        return 1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Check if character is Japanese 1 byte
////////////////////////////////////////////////////////////////////////////////
int is_japanese_one_byte(char *c)
{

    for (int i = 0; i < 100; i++) {
        if (strcmp(c, kana_one[i]) == 0) {
            return 1;
        }
    }

    return 0;

}

////////////////////////////////////////////////////////////////////////////////
// Count the length of line
////////////////////////////////////////////////////////////////////////////////
int count_length(char *line)
{
    int len = 0;
    int pre_len = strlen(line);
    for (int i = 0; i < pre_len; i++) {

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

    input_file_master[0] = '\0';
    input_file_tran[0] = '\0';

    // マスタ入力ファイルパス名を取得する。
    int opt_index = 1;
    if (argv[opt_index] == NULL) {
        msgerr
            ("異常終了 標準入力から複数指定することができません%s\n",
             "");
        return 1;
    } else {
        strncpy(input_file_master, argv[opt_index], sizeof(input_file_master) - 1);
        input_file_master[sizeof(input_file_master) - 1] = '\0';
        opt_index++;
    }

    // トラン入力ファイルパス名を取得する。
    if (opt_index < argc) {
        strncpy(input_file_tran, argv[opt_index], sizeof(input_file_tran) - 1);
        input_file_tran[sizeof(input_file_tran) - 1] = '\0';
        if (!strcmp(input_file_master, "-") && !strcmp(input_file_tran, "-")) {
            msgerr
                ("異常終了 標準入力から複数指定することができません%s\n",
                 "");
            return 1;
        }
    } else {
        if (!strcmp(input_file_master, "-")) {
            msgerr
                ("異常終了 標準入力から複数指定することができません%s\n",
                 "");
            return 1;
        } else {
            // 上書きファイルを設定しない
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Calculate v_sidecat for corresponding lines from master_file and tran_file
////////////////////////////////////////////////////////////////////////////////
int calv_sidecat(char *master_line, char *tran_line, FILE * ot_fp)
{

//      char output_line[MAXSIZE_LINE];
    int length_line_master = 0;
    int length_line_tran = 0;

    // This is buffer for length of the master line (A integer which have maximum is 4096*16)
    char buf_master[6];
    if (fgets(buf_master, sizeof(buf_master), temp_length_masterfile) != NULL) {
        length_line_master = atol(buf_master);
    }
    // This is buffer for length of the tran line (A integer which have maximum is 4096*16)
    char buf_tran[6];
    if (fgets(buf_tran, sizeof(buf_tran), temp_length_tranfile) != NULL) {
        length_line_tran = atol(buf_tran);
    }
    // Ignore enter character
    if (strstr(master_line, "\n") != NULL) {
        master_line[strlen(master_line) - 1] = '\0';
    }

    if (strstr(tran_line, "\n") != NULL) {
        tran_line[strlen(tran_line) - 1] = '\0';
    }

    if ((fputs(master_line, ot_fp)) == EOF) {
        return 1;
    }

    empty_line[max_length_master - length_line_master + 1] = '\0';
    if ((fputs(empty_line, ot_fp)) == EOF) {
        return 1;
    }
    empty_line[max_length_master - length_line_master + 1] = ' ';

    if ((fputs(tran_line, ot_fp)) == EOF) {
        return 1;
    }

    empty_line[max_length_tran - length_line_tran] = '\0';
    if ((fputs(empty_line, ot_fp)) == EOF) {
        return 1;
    }
    empty_line[max_length_tran - length_line_tran] = ' ';

    if ((fputs("\n", ot_fp)) == EOF) {
        return 1;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Create temporary master file
////////////////////////////////////////////////////////////////////////////////
FILE *create_temp_masterfile()
{

    FILE *tmp_fp;
    char temp_file[MAXSIZE_OPTION];
    char tpl_line[MAXSIZE_LINE];
    sprintf(temp_file, "/tmp/%d-v_sidecat", getpid());
    tmp_fp = fopen(temp_file, "w+");

    // Call unlink so that whenever the file is closed or the program exited,
    // the temporary file will be deleted
    unlink(temp_file);
    while (fgets(tpl_line, sizeof(tpl_line), stdin) != NULL) {

        // Calculate to get length of the longest line in master file.
        int len = count_length(tpl_line);
        if (max_length_master < len) {
            max_length_master = len;
        }
        fprintf(temp_length_masterfile, "%d\n", len);
        fputs(tpl_line, tmp_fp);
    }
    return tmp_fp;
}

////////////////////////////////////////////////////////////////////////////////
// Create temporary tran file
////////////////////////////////////////////////////////////////////////////////
FILE *create_temp_tranfile()
{

    FILE *tmp_fp;
    char temp_file[MAXSIZE_OPTION];
    char tpl_line[MAXSIZE_LINE];
    sprintf(temp_file, "/tmp/%d-v_sidecat", getpid());
    tmp_fp = fopen(temp_file, "w+");

    // Call unlink so that whenever the file is closed or the program exited,
    // the temporary file will be deleted
    unlink(temp_file);
    while (fgets(tpl_line, sizeof(tpl_line), stdin) != NULL) {

        // Calculate to get length of the longest line in master file.
        int len = count_length(tpl_line);
        if (max_length_tran < len) {
            max_length_tran = len;
        }
        fprintf(temp_length_tranfile, "%d\n", len);
        fputs(tpl_line, tmp_fp);
    }
    return tmp_fp;
}

////////////////////////////////////////////////////////////////////////////////
// Create temporary length_master file for save length of line
////////////////////////////////////////////////////////////////////////////////
FILE *create_length_temp_masterfile()
{
    FILE *tmp_fp;
    char temp_file[MAXSIZE_OPTION];
    sprintf(temp_file, "/tmp/%d-v_sidecat-length-masterfile", getpid());
    tmp_fp = fopen(temp_file, "w+");

    // Call unlink so that whenever the file is closed or the program exited,
    // the temporary file will be deleted
    unlink(temp_file);
    return tmp_fp;
}

////////////////////////////////////////////////////////////////////////////////
// Create temporary length_tran file for save length of line
////////////////////////////////////////////////////////////////////////////////
FILE *create_length_temp_tranfile()
{
    FILE *tmp_fp;
    char temp_file[MAXSIZE_OPTION];
    sprintf(temp_file, "/tmp/%d-v_sidecat-length-tranfile", getpid());
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
    FILE *in_fp, *ov_fp, *ot_fp;
    max_length_master = 0;
    max_length_tran = 0;

    if (strcmp(input_file_master, "-") == 0) {
        temp_length_masterfile = create_length_temp_masterfile();
        in_fp = create_temp_masterfile();
    } else {
        if ((in_fp = fopen(input_file_master, "r")) == NULL) {
            msgerr("異常終了 入力ファイルが読み取り不可(%s)\n",
                   input_file_master);
            return (1);
        }
        // Calculate to get length of the longest line in master file.
        char in_line[MAXSIZE_LINE];
        temp_length_masterfile = create_length_temp_masterfile();
        while (fgets(in_line, sizeof(in_line), in_fp) != NULL) {
            int len = count_length(in_line);
            if (max_length_master < len) {
                max_length_master = len;
            }
            fprintf(temp_length_masterfile, "%d\n", len);
        }
    }

    // Move file pointer to top of file
    fseek(in_fp, 0, SEEK_SET);
    fseek(temp_length_masterfile, 0, SEEK_SET);

    if (strcmp(input_file_tran, "") == 0 || strcmp(input_file_tran, "-") == 0) {
        temp_length_tranfile = create_length_temp_tranfile();
        ov_fp = create_temp_tranfile();
    } else {
        if ((ov_fp = fopen(input_file_tran, "r")) == NULL) {
            msgerr("異常終了 入力ファイルが読み取り不可(%s)\n",
                   input_file_tran);
            return (1);
        }
        // Calculate to get length of the longest line in tran file.
        char in_line[MAXSIZE_LINE];
        temp_length_tranfile = create_length_temp_tranfile();
        while (fgets(in_line, sizeof(in_line), ov_fp) != NULL) {
            int len = count_length(in_line);
            if (max_length_tran < len) {
                max_length_tran = len;
            }
            fprintf(temp_length_tranfile, "%d\n", len);
        }
    }
    // Move file pointer to top of file
    fseek(ov_fp, 0, SEEK_SET);
    fseek(temp_length_tranfile, 0, SEEK_SET);
    ot_fp = stdout;

    // Create empty lines
    for (int i = 0; i < MAXSIZE_LINE; i++) {
        empty_line[i] = ' ';
    }
    empty_line[MAXSIZE_LINE - 1] = '\0';

    // ファイル読み取りループ
    char in_line_input[MAXSIZE_LINE];
    char in_line_tran[MAXSIZE_LINE];
    int num_line = 0;

    while (1) {

        num_line++;

        // In this case, master file is end, number lines of master file <= number lines of tran file
        if (fgets(in_line_input, sizeof(in_line_input), in_fp) == NULL) {

            // Output remain lines of tran file
            while (fgets(in_line_tran, sizeof(in_line_tran), ov_fp) != NULL) {

                num_line++;

                if (calv_sidecat("", in_line_tran, ot_fp) == 1) {
                    msgerr("異常終了 結果出力でエラーが発生(LINE=%d)\n",
                           num_line);
                    return 1;
                }
            }

            break;
        }
        // In this case, tran file is end, number lines of master file > number lines of tran file
        if (fgets(in_line_tran, sizeof(in_line_tran), ov_fp) == NULL) {

            // Output remain lines of master file
            if ((fputs(in_line_input, ot_fp)) == EOF) {
                msgerr("異常終了 結果出力でエラーが発生(LINE=%d)\n",
                       num_line);
                return 1;
            }

            while (fgets(in_line_input, sizeof(in_line_input), in_fp) != NULL) {

                num_line++;

                if ((fputs(in_line_input, ot_fp)) == EOF) {
                    msgerr("異常終了 結果出力でエラーが発生(LINE=%d)\n",
                           num_line);
                    return 1;
                }
            }

            break;
        }
        // Calculate side cat for corresponding lines from master and tran file.
        if (calv_sidecat(in_line_input, in_line_tran, ot_fp) == 1) {
            msgerr("異常終了 結果出力でエラーが発生(LINE=%d)\n", num_line);
            return 1;
        }
    }

    fclose(in_fp);
    fclose(ov_fp);
    fclose(temp_length_masterfile);

    // 正常終了
    msginf("%s\n", "正常終了");
    return 0;
}
