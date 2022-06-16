//
// Created by francesco on 26/11/21.
//

#include "lexer.h"

#include <string.h>
#include <ctype.h>

/// -------------------------------------------------
/// PRIVATE DATA
/// -------------------------------------------------

typedef struct {
    const char *word;
    TokenKind kind;
} Reserved;

/// -------------------------------------------------
/// PRIVATE FUNCTIONS
/// -------------------------------------------------

static void init_keywords(Map(TokenKind) *m) {

    static const Reserved keywords[] = {
            {"as",     TK_AS},
            {"break",  TK_BREAK},
            {"const",  TK_CONST},
            {"else",   TK_ELSE},
            {"fn",     TK_FN},
            {"if",     TK_IF},
            {"let",    TK_LET},
            {"loop",   TK_LOOP},
            {"mut",    TK_MUT},
            {"return", TK_RETURN},
            {"static", TK_STATIC},
            {"while",  TK_WHILE},
            {"true",   TK_BOOL_LIT},
            {"false",  TK_BOOL_LIT},
    };

    usize count = sizeof(keywords) / sizeof(Reserved);
    for (usize i = 0; i < count; ++i) {
        StringSlice s;
        init_string(&s, keywords[i].word, strlen(keywords[i].word));
        m->insert(m, &s, &keywords[i].kind);
    }
}

// -------------------------------------------------
// IDENTIFIER PARSING
// -------------------------------------------------

static TokenKind is_reserved(Map(TokenKind) *m, const StringSlice *v) {

    TokenKind *tk = m->get(m, v);
    if (tk != NULL) {
        return *tk;
    } else {
        return TK_NAME_ID;
    }
}

static void skip_spaces(Lexer *l) {

    char c = current_char(l->fb);
    while (isspace(c)) {
        c = next_char(l->fb);
    }
}

static void skip_until_newline(Lexer *l) {

    char c = current_char(l->fb);
    while (c != '\n') {
        c = next_char(l->fb);
    }
    next_char(l->fb);
}

static void skip_until_end_comment(Lexer *l) {

    char c;
    while (true) {
        c = current_char(l->fb);
        if (c == '*') {
            c = next_char(l->fb);
            if (c == '/') {
                next_char(l->fb);
                break;
            } else {
                next_char(l->fb);
            }
        } else {
            next_char(l->fb);
        }
    }
}

static bool is_op_char(char c) {

    switch (c) {
        case '!':
        case '%':
        case '^':
        case '&':
        case '*':
        case '-':
        case '+':
        case '=':
        case ':':
        case '@':
        case '~':
        case '#':
        case '|':
        case '\\':
        case '<':
        case '>':
        case '?':
        case '/':
            return true;
        default:
            return false;
    }
}

/// parses ids which correspond to [_a-zA-Z]([_a-zA-Z0-9])*
/// and parses underscore token
static void parse_named_id(Lexer *l, Token *t) {

    Coordinates s = *get_coord(l->fb);
    u64 off_s = get_offset(l->fb);
    char first = current_char(l->fb);
    char c = next_char(l->fb);

    if (isalnum(c) || c == '_') { // '[_a-zA-z][_a-zA-Z]+'

        do {
            c = peek_char(l->fb);
            if (isalnum(c) || c == '_') {
                next_char(l->fb);
            } else {
                break;
            }
        } while (true);

        u64 off_e = get_offset(l->fb);
        StringSlice lex;
        get_lexeme(l->fb, &lex, off_s, off_e);

        TokenKind kind = is_reserved(&l->keywords, &lex);
        if (kind == TK_NAME_ID) {
            init_named_id(t, &lex, &s, get_coord(l->fb));
        } else if (kind == TK_BOOL_LIT) {
            init_bool_token(t, lex.data[0] == 't', &s, get_coord(l->fb));
        } else {
            init_token(t, kind, &s, get_coord(l->fb));
            t->keyword = lex;
        }

        next_char(l->fb);

    } else if (first == '_') { // '_'
        init_token(t, TK_UNDERSCORE, &s, &s);
    } else { // '[a-zA-z]'
        StringSlice lex;
        get_lexeme(l->fb, &lex, off_s, off_s);
        init_named_id(t, &lex, &s, &s);
    }
}

/// pratt_parser operator ids and eq token ('=')
static void parse_op_id(Lexer *l, Token *t) {

    const Coordinates s = *get_coord(l->fb);
    u64 off_s = get_offset(l->fb);

    char c = current_char(l->fb);
    if (c == '=') {
        c = next_char(l->fb);
        if (!is_op_char(c)) {
            init_token(t, TK_EQ, &s, &s);
            return;
        }
    } else if (c == ':') {
        c = next_char(l->fb);
        if (!is_op_char(c)) {
            init_token(t, TK_COLON, &s, &s);
            return;
        }
    } else if (c == '@') {
        c = next_char(l->fb);
        if (!is_op_char(c)) {
            init_token(t, TK_AT, &s, &s);
            return;
        }
    }

    u8 comment = 0;
    do {
        c = peek_char(l->fb);
        if (c == '/') {
            c = peek_char_index(l->fb, 2);
            if (c == '/') {
                comment = 1;
                break;
            } else if (c == '*') {
                comment = 2;
                break;
            }
        } else if (is_op_char(c)) {
            next_char(l->fb);
        } else {
            break;
        }
    } while (true);

    u64 off_e = get_offset(l->fb);

    StringSlice id;
    get_lexeme(l->fb, &id, off_s, off_e);

    init_op_id(t, &id, &s, get_coord(l->fb));

    switch (comment) {
        case 0: { // no comment
            next_char(l->fb);
            break;
        }
        case 1: { // line comment
            skip_until_newline(l);
            break;
        }
        case 2: { // multiline comment
            skip_until_end_comment(l);
            break;
        }
        default: {
            fprintf(stderr, "CODE-ERROR: strange type of comment\n");
            exit(1);
        }
    }
}

// -------------------------------------------------
// NUMBER PARSING
// -------------------------------------------------

typedef enum {
    NUM_BIN,
    NUM_OCT,
    NUM_DEC,
    NUM_HEX,
} NumKind;

static bool is_valid_digit(char c, NumKind nk) {

    switch (nk) {
        case NUM_BIN: {
            return c == '0' || c == '1';
        }
        case NUM_OCT: {
            return c >= '0' && c <= '7';
        }
        case NUM_DEC: {
            return isdigit(c);
        }
        case NUM_HEX: {
            return isxdigit(c);
        }
        default: {
            fprintf(stderr, "ERROR CODE: NumKind must be bin/oct/dec/hex\n");
            exit(1);
        }
    }
}

// note that the result of this function is not equal to !is_valid_digit()
// here we verify if a char (that cannot be part of a suffix) is a valid digit for the kind specified
// for example, 2..9 are not valid digit of a binary number
static bool is_not_valid_digit(char c, NumKind nk) {

    switch (nk) {
        case NUM_BIN: {
            return c >= 2 && c <= 9;
        }
        case NUM_OCT: {
            return c == '8' || c == '9';
        }
        case NUM_DEC:
        case NUM_HEX: {
            return false;
        }
        default: {
            fprintf(stderr, "ERROR CODE: NumKind must be bin/oct/dec/hex\n");
            exit(1);
        }
    }
}

static u64 update_int_value(u64 current, char c, NumKind nk) {

    switch (nk) {
        case NUM_BIN: {
            return (current * 2) + (c - '0');
        }
        case NUM_OCT: {
            return (current * 8) + (c - '0');
        }
        case NUM_DEC: {
            return (current * 10) + (c - '0');
        }
        case NUM_HEX: {
            if (c >= '0' && c <= '9') {
                return (current * 16) + (c - '0');
            } else if (c >= 'a' && c <= 'f') {
                return (current * 16) + (c - 'a' + 10);
            } else {
                return (current * 16) + (c - 'A' + 10);
            }
        }
        default: {
            fprintf(stderr, "ERROR CODE: NumKind must be bin/oct/dec/hex\n");
            exit(1);
        }
    }
}

static f64 update_float_value(f64 current, char c, NumKind nk) {

    switch (nk) {
        case NUM_BIN: {
            return (current * 2.0) + (c - '0');
        }
        case NUM_OCT: {
            return (current * 8.0) + (c - '0');
        }
        case NUM_DEC: {
            return (current * 10.0) + (c - '0');
        }
        case NUM_HEX: {
            if (c >= '0' && c <= '9') {
                return (current * 16.0) + (c - '0');
            } else if (c >= 'a' && c <= 'f') {
                return (current * 16.0) + (c - 'a' + 10.0);
            } else {
                return (current * 16.0) + (c - 'A' + 10.0);
            }
        }
        default: {
            fprintf(stderr, "ERROR CODE: NumKind must be bin/oct/dec/hex\n");
            exit(1);
        }
    }
}

static f64 get_exponent(NumKind nk) {

    switch (nk) {
        case NUM_BIN: {
            return 2.0;
        }
        case NUM_OCT: {
            return 8.0;
        }
        case NUM_DEC: {
            return 10.0;
        }
        case NUM_HEX: {
            return 16.0;
        }
        default: {
            fprintf(stderr, "ERROR CODE: NumKind must be bin/oct/dec/hex\n");
            exit(1);
        }
    }
}

static void not_valid_digit_error(Lexer *l, char c, NumKind nk) {

    Coordinates c_error = *get_coord(l->fb);
    ++c_error.column;
    switch (nk) {
        case NUM_BIN: {
            post_error(l->eh, MSG_INVALID_DIGIT, &c_error, NULL, c, "binary");
            break;
        }
        case NUM_OCT: {
            post_error(l->eh, MSG_INVALID_DIGIT, &c_error, NULL, c, "octal");
            break;
        }
        case NUM_DEC: {
            post_error(l->eh, MSG_INVALID_DIGIT, &c_error, NULL, c, "decimal");
            break;
        }
        case NUM_HEX: {
            post_error(l->eh, MSG_INVALID_DIGIT, &c_error, NULL, c, "hexadecimal");
            break;
        }
        default: {
            fprintf(stderr, "ERROR CODE: NumKind must be bin/oct/dec/hex\n");
            exit(1);
        }
    }
}

static void parse_number(Lexer *l, Token *t) {

    Coordinates c_start = *get_coord(l->fb);

    char c = current_char(l->fb);
    NumKind nk = NUM_DEC;
    u64 i_value = 0;
    f64 f_value;
    bool is_float = false;

    if (c == '0') {
        c = peek_char(l->fb);
        switch (c) {
            case 'b': {
                nk = NUM_BIN;
                next_char(l->fb);
                break;
            }
            case 'o': {
                nk = NUM_OCT;
                next_char(l->fb);
                break;
            }
            case 'x': {
                nk = NUM_HEX;
                next_char(l->fb);
                break;
            }
            default: {
                break;
            }
        }
    } else {
        i_value += c - '0';
    }

    do {
        c = peek_char(l->fb);
        if (is_valid_digit(c, nk)) {
            i_value = update_int_value(i_value, c, nk);
            next_char(l->fb);
        } else if (c == '.' || c == 'e' || c == 'E') {
            is_float = true;
            break;
        } else if (c == '\'') {
            next_char(l->fb);
            continue;
        } else if (is_not_valid_digit(c, nk)) {
            not_valid_digit_error(l, c, nk);
            next_char(l->fb);
        } else {
            break;
        }
    } while (true);

    if (is_float) {
        f_value = i_value;
        i64 exp = 0;

        c = peek_char(l->fb);
        if (c == '.') {
            f_value = i_value;
            next_char(l->fb);

            do {
                c = peek_char(l->fb);
                if (is_valid_digit(c, nk)) {
                    f_value = update_float_value(f_value, c, nk);
                    --exp;
                    next_char(l->fb);
                } else if (is_not_valid_digit(c, nk)) {
                    not_valid_digit_error(l, c, nk);
                    next_char(l->fb);
                } else if (c == '\'') {
                    next_char(l->fb);
                    continue;
                } else {
                    break;
                }
            } while (true);
        }

        c = peek_char(l->fb);
        if (c == 'e' || c == 'E') {
            next_char(l->fb);
            i64 sign = 1;
            c = peek_char(l->fb);
            if (c == '+') {
                next_char(l->fb);
            } else if (c == '-') {
                sign = -1;
                next_char(l->fb);
            }

            u64 exp2 = 0;
            do {
                c = peek_char(l->fb);
                if (is_valid_digit(c, nk)) {
                    exp2 = update_int_value(exp2, c, nk);
                    next_char(l->fb);
                } else if (c == '\'') {
                    next_char(l->fb);
                } else if (is_not_valid_digit(c, nk)) {
                    not_valid_digit_error(l, c, nk);
                    next_char(l->fb);
                } else {
                    break;
                }
            } while (true);

            exp += ((i64) exp2 * sign);
        }

        f64 p = get_exponent(nk);
        while (exp > 0) {
            f_value *= p;
            --exp;
        }

        while (exp < 0) {
            f_value /= p;
            ++exp;
        }
    }

    c = peek_char(l->fb);
    if (c == '_') {
        do {
            c = peek_char(l->fb);
            if (c == '_') {
                next_char(l->fb);
            } else {
                break;
            }
        } while (true);
    }

    StringSlice suffix;
    c = peek_char(l->fb);
    if (isalnum(c)) {
        next_char(l->fb);
        u64 suf_start = get_offset(l->fb);
        do {
            c = peek_char(l->fb);
            if (isalnum(c)) {
                next_char(l->fb);
            } else {
                break;
            }
        } while (true);

        u64 suf_end = get_offset(l->fb);
        get_lexeme(l->fb, &suffix, suf_start, suf_end);
    } else {
        init_string(&suffix, NULL, 0);
    }

    Coordinates c_end = *get_coord(l->fb);
    if (is_float) {
        init_float_token(t, f_value, &suffix, &c_start, &c_end);
    } else {
        init_int_token(t, i_value, &suffix, &c_start, &c_end);
    }
    next_char(l->fb);
}

static void read_token(Lexer *l, Token *t) {

    while (true) {
        skip_spaces(l);
        char c = current_char(l->fb);
        switch (c) {
            case '\0': {
                init_token(t, TK_EOF, get_coord(l->fb), get_coord(l->fb));
                next_char(l->fb);
                return;
            }
            case '(': {
                init_token(t, TK_OPEN_PAREN, get_coord(l->fb), get_coord(l->fb));
                next_char(l->fb);
                return;
            }
            case ')': {
                init_token(t, TK_CLOSE_PAREN, get_coord(l->fb), get_coord(l->fb));
                next_char(l->fb);
                return;
            }
            case '[': {
                init_token(t, TK_OPEN_SQUARE, get_coord(l->fb), get_coord(l->fb));
                next_char(l->fb);
                return;
            }
            case ']': {
                init_token(t, TK_CLOSE_SQUARE, get_coord(l->fb), get_coord(l->fb));
                next_char(l->fb);
                return;
            }
            case '{': {
                init_token(t, TK_OPEN_CURLY, get_coord(l->fb), get_coord(l->fb));
                next_char(l->fb);
                return;
            }
            case '}': {
                init_token(t, TK_CLOSE_CURLY, get_coord(l->fb), get_coord(l->fb));
                next_char(l->fb);
                return;
            }
            case ',': {
                init_token(t, TK_COMMA, get_coord(l->fb), get_coord(l->fb));
                next_char(l->fb);
                return;
            }
            case ';': {
                init_token(t, TK_SEMI, get_coord(l->fb), get_coord(l->fb));
                next_char(l->fb);
                return;
            }
            case '_': {
                parse_named_id(l, t);
                return;
            }
            case '/': {
                c = peek_char(l->fb);
                if (c == '/') {
                    skip_until_newline(l);
                    break;
                } else if (c == '*') {
                    skip_until_end_comment(l);
                    break;
                } else {
                    parse_op_id(l, t);
                    return;
                }
            }
            default: {
                if (isalpha(c)) {
                    parse_named_id(l, t);
                    return;
                } else if (isdigit(c)) {
                    parse_number(l, t);
                    return;
                } else if (is_op_char(c)) {
                    parse_op_id(l, t);
                    return;
                } else {
                    post_error(l->eh, MSG_UNKNOWN_TOKEN, get_coord(l->fb), NULL);
                    next_char(l->fb);
                }
            }
        }
    }
}

/// -------------------------------------------------
/// PUBLIC API
/// -------------------------------------------------

void init_lexer(Lexer *l, FileBuffer *fb, ErrorHandler *eh) {

    l->fb = fb;
    l->eh = eh;
    l->keywords = new_map(TokenKind);
    init_keywords(&l->keywords);

    l->valid = false;
    l->valid2 = false;
}

void free_lexer(Lexer *l) {

    if (l == NULL) { return; }
    l->keywords.free(&l->keywords);
}

// -------------------------------------------------

void next_token(Lexer *l, Token *t) {

    if (l->valid) {
        if (t != NULL) {
            *t = l->next;
        }
        l->valid = false;
    } else if (l->valid2) {
        if (t != NULL) {
            *t = l->next2;
        }
        l->valid2 = false;
    } else {
        read_token(l, t);
    }
}

void peek_token(Lexer *l, Token *t) {

    if (!l->valid) {
        if (l->valid2) {
            l->next = l->next2;
            l->valid2 = false;
        } else {
            next_token(l, &l->next);
        }
        l->valid = true;
    }

    if (t != NULL) {
        *t = l->next;
    }
}

TokenKind peek_kind(Lexer *l) {

    if (!l->valid) {
        if (l->valid2) {
            l->next = l->next2;
            l->valid2 = false;
        } else {
            next_token(l, &l->next);
        }
        l->valid = true;
    }

    return l->next.kind;
}

const Coordinates *peek_start_coordinates(Lexer *l) {

    if (!l->valid) {
        if (l->valid2) {
            l->next = l->next2;
            l->valid2 = false;
        } else {
            next_token(l, &l->next);
        }
        l->valid = true;
    }

    return get_token_start(&l->next);
}

const Coordinates *peek_end_coordinates(Lexer *l) {

    if (!l->valid) {
        if (l->valid2) {
            l->next = l->next2;
            l->valid2 = false;
        } else {
            next_token(l, &l->next);
        }
        l->valid = true;
    }

    return get_token_end(&l->next);
}

// -------------------------------------------------

TokenKind peek_kind_2(Lexer *l) {

    if (!l->valid2) {
        read_token(l, &l->next2);
        l->valid2 = true;
    }

    return l->next2.kind;
}