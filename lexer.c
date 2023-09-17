
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "lexer.h"

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
        case t_at: return "at";
        case t_bang: return "bang";

        case t_shiftleft: return "shiftleft";

        case t_mmp: return "mmp";
        case t_unused: return "unused";
        case t_initialize: return "initialize";
        case t_on_interrupt: return "on_interrupt";
        case t_fun: return "fun";

        case t_inttype: return "inttype";
        case t_bf: return "bf";
        case t_be: return "be";

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

bool is_be(StringRef str, char *lookahead) {
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    if (str.len < 7 || strncmp(str.str, "BitEnum", 7) != 0) {
        return false;
    }
    for (int i = 7; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], "1234567890", 10)) {
            return false;
        }
    }
    return true;
}
bool is_bf(StringRef str, char *lookahead) {
    if (!char_is_in(lookahead[0], DELIM_CHARS, DELIM_CHARS_LEN)) {
        return false;
    }
    if (str.len < 8 || strncmp(str.str, "BitField", 8) != 0) {
        return false;
    }
    for (int i = 8; i < str.len; i++) {
        char c = str.str[i];
        if (!char_is_in(str.str[i], "1234567890", 10)) {
            return false;
        }
    }
    return true;
}

bool is_inttype(StringRef str, char *lookahead) {
    if (str.len == 2 && strncmp(str.str, "u8", str.len) == 0) {
        return true;
    } else if (str.len == 3 && strncmp(str.str, "u16", str.len) == 0) {
        return true;
    } else if (str.len == 3 && strncmp(str.str, "u32", str.len) == 0) {
        return true;
    } else {
        return false;
    }
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
    } else if (strncmp(str.str, "@", str.len) == 0) {
        return t_at;
    } else if (strncmp(str.str, "!", str.len) == 0) {
        return t_bang;
    } else if (str.len == 2 && strncmp(str.str, "<<", str.len) == 0) {
        return t_shiftleft;
    } else if (str.len == 22 && strncmp(str.str, "MemoryMappedPeripheral", str.len) == 0) {
        return t_mmp;
    } else if (str.len == 7 && strncmp(str.str, "$unused", str.len) == 0) {
        return t_unused;
    } else if (str.len == 10 && strncmp(str.str, "initialize", str.len) == 0) {
        return t_initialize;
    } else if (str.len == 12 && strncmp(str.str, "on_interrupt", str.len) == 0) {
        return t_on_interrupt;
    } else if (str.len == 3 && strncmp(str.str, "fun", str.len) == 0) {
        return t_fun;
    } else if (is_inttype(str, lookahead)) {
        return t_inttype;
    } else if (is_bf(str, lookahead)) {
        return t_bf;
    } else if (is_be(str, lookahead)) {
        return t_be;
    } else if (is_id(str, lookahead)) {
        return t_id;
    } else if (is_inthexliteral(str, lookahead)) {
        return t_inthexliteral;
    } else if (is_intdecliteral(str, lookahead)) {
        return t_intdecliteral;
    } else if (str.len == 1 && char_is_in(str.str[0], " \n", 2)) {
        // ignore whitespace
        return t_IGNORE;
    } else if (is_comment(str)) {
        // ignore lines starting with #
        return t_IGNORE;
    }
    return t_NONE;
}

int int_value_for_token(Token token) {
    if (token.type == t_intdecliteral || token.type == t_inthexliteral) {
        char buf[256];
        memset(buf, 0, 256);
        // yes this is an overflow risk
        strncpy(buf, token.lexeme.str, token.lexeme.len);
        return strtol(buf, NULL, 0);
    } else if (token.type == t_inttype) {
        char buf[256];
        memset(buf, 0, 256);
        // yes this is an overflow risk
        strncpy(buf, token.lexeme.str + 1, token.lexeme.len - 1);
        return strtol(buf, NULL, 0);
    } else if (token.type == t_bf) {
        char buf[256];
        memset(buf, 0, 256);
        // yes this is an overflow risk
        strncpy(buf, token.lexeme.str + 8, token.lexeme.len - 8);
        return strtol(buf, NULL, 0);
    } else {
        return 0;
    }

}

int lex(char *source, int source_len, Token *tokens) {
    int token_num = 0;

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
            tokens[token_num] = (Token){curr_tok, curr_str};
            tokens[token_num].int_value = int_value_for_token(tokens[token_num]);
            token_num += 1;
            i_lexed_so_far = i;
        }
    }
    return token_num;
}