#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_SOURCE_LEN 65536
#define VALID_ID_CHARS_START "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$"
#define VALID_ID_CHARS_START_LEN 54
#define VALID_ID_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_"
#define VALID_ID_CHARS_LEN 63
#define DELIM_CHARS "\0\n =(){}:;.,"
#define DELIM_CHARS_LEN 12

#define DEBUG_LOG 0
#define LOG(...) if (DEBUG_LOG) printf(__VA_ARGS__)

typedef struct _StringRef {
    char *str;
    int len;
} StringRef;

typedef enum {
    t_NONE = 0,
    t_IGNORE,
    t_equals,
    t_leftparen,
    t_rightparen,
    t_leftbrace,
    t_rightbrace,
    t_colon,
    t_semicolon,
    t_dot,
    t_comma,
    t_shiftleft,
    t_id,
    t_inthexliteral,
    t_intdecliteral
} TokenType;

typedef struct _Lexeme {
    TokenType t;
    StringRef s;
} Lexeme;

void print_StringRef(StringRef s) {
    for (int i = 0; i < s.len; i++) {
        putc(s.str[i], stdout);
    }
}

char *token_type_to_static_string(TokenType token_type) {
    switch (token_type) {
        case t_NONE: return "NONE";
        case t_IGNORE: return "IGNORE";
        case t_equals: return "equals";
        case t_leftparen: return "leftparen";
        case t_rightparen: return "rightparen";
        case t_leftbrace: return "leftbrace";
        case t_rightbrace: return "rightbrace";
        case t_colon: return "colon";
        case t_semicolon: return "semicolon";
        case t_dot: return "dot";
        case t_comma: return "comma";
        case t_shiftleft: return "shiftleft";
        case t_id: return "id";
        case t_inthexliteral: return "inthexliteral";
        case t_intdecliteral: return "intdecliteral";
        default: return "ERROR_INVALID_TOKEN_TYPE";
    }
}

bool char_is_in(char c, char *valid, int valid_len) {
    for (int i = 0; i < valid_len; i++) {
        if (c == valid[i]) {
            return true;
        }
    }
    return false;
}

bool is_comment(StringRef str) {
    if (str.len < 2) {
        return false;
    }
    if (str.str[0] == '#' && str.str[str.len - 1] == '\n') {
        return true;
    }
    return false;
}
bool is_id(StringRef str, char *lookahead) {
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    if (!char_is_in(str.str[0], VALID_ID_CHARS_START, VALID_ID_CHARS_START_LEN)) {
        return false;
    }
    for (int i = 1; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], VALID_ID_CHARS, VALID_ID_CHARS_LEN)) {
            LOG("invalid body because %c\n", c);
            return false;
        }
    }
    return true;
}

bool is_inthexliteral(StringRef str, char *lookahead) {
    if (str.len < 3) {
        return false;
    }
    if (str.str[0] != '0' || str.str[1] != 'x') {
        return false;
    }
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    for (int i = 2; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], "1234567890abcdefABCDEF", 22)) {
            LOG("invalid body because %c\n", c);
            return false;
        }
    }
    return true;
}

bool is_intdecliteral(StringRef str, char *lookahead) {
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    for (int i = 0; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], "1234567890", 10)) {
            LOG("invalid body because %c\n", c);
            return false;
        }
    }
    return true;
}

TokenType token_type(StringRef str, char *lookahead) {
    if (DEBUG_LOG) print_StringRef(str);
    LOG(" evaluating token: ");
    if (strncmp(str.str, "=", str.len) == 0) {
        return t_equals;
    } else if (strncmp(str.str, "(", str.len) == 0) {
        return t_leftparen;
    } else if (strncmp(str.str, ")", str.len) == 0) {
        return t_rightparen;
    } else if (strncmp(str.str, "{", str.len) == 0) {
        return t_leftbrace;
    } else if (strncmp(str.str, "}", str.len) == 0) {
        return t_rightbrace;
    } else if (strncmp(str.str, ":", str.len) == 0) {
        return t_colon;
    } else if (strncmp(str.str, ";", str.len) == 0) {
        return t_semicolon;
    } else if (strncmp(str.str, ".", str.len) == 0) {
        return t_dot;
    } else if (strncmp(str.str, ",", str.len) == 0) {
        return t_comma;
    } else if (strncmp(str.str, "<<", str.len) == 0) {
        return t_shiftleft;
    } else if (is_id(str, lookahead)) {
        return t_id;
    } else if (is_inthexliteral(str, lookahead)) {
        return t_inthexliteral;
    } else if (is_intdecliteral(str, lookahead)) {
        return t_intdecliteral;
    } else if (str.len == 1 && char_is_in(str.str[0], " \n", 2)) {
        return t_IGNORE;
    } else if (is_comment(str)) {
        return t_IGNORE;
    }
    return t_NONE;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "ERROR: One argument is required: the path to the Lang808 source file.\n");
        return 1;
    }
    char *source_file_name = argv[1];
    printf("Compiling %s\n", source_file_name);

    char source[MAX_SOURCE_LEN];
    int source_len = 0;
    memset(source, 0, MAX_SOURCE_LEN);

    FILE *source_file = fopen(source_file_name, "r");
    if (source_file == NULL) {
        fprintf(stderr, "ERROR: Can't open source file.\n");
        return 2;
    }

    fseek(source_file, 0L, SEEK_END);
    source_len = ftell(source_file);

    if (source_len >= MAX_SOURCE_LEN) {
        fprintf(stderr, "ERROR: Source file is too big. Maximum is %d.\n", MAX_SOURCE_LEN - 1);
        return 2;
    }
    rewind(source_file);
    for (int i = 0; i < source_len; i++) {
        source[i] = getc(source_file);
    }

    Lexeme lexemes[MAX_SOURCE_LEN];
    memset(lexemes, 0, MAX_SOURCE_LEN * sizeof(Lexeme));
    int lexeme_num = 0;

    StringRef curr_str = {source, 0};
    TokenType curr_tok = t_NONE;
    int i_lexed_so_far = 0;
    for (int i = 1; i < source_len; i++) {
        curr_str = (StringRef){source + i_lexed_so_far, i - i_lexed_so_far};
        curr_tok = token_type(curr_str, source + i);
        LOG("got %s\n", token_type_to_static_string(curr_tok));
        if (curr_tok == t_IGNORE) {
            i_lexed_so_far = i;
        } else if (curr_tok != t_NONE) {
            lexemes[lexeme_num] = (Lexeme){curr_tok, curr_str};
            lexeme_num += 1;
            i_lexed_so_far = i;
        }
    }

    char buf[256];
    for (int i = 0; lexemes[i].t != t_NONE; i++) {
        strncpy(buf, lexemes[i].s.str, lexemes[i].s.len);
        buf[lexemes[i].s.len] = 0;
        printf("%s(%s) ", token_type_to_static_string(lexemes[i].t), buf);
    }
    printf("\n");
}