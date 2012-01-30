/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef TOKENS_H
#define TOKENS_H

enum FuncToken {
TOKEN_ADD_OP = 0,
TOKEN_SUB_OP = 1,
TOKEN_POW_OP = 2,
TOKEN_DIV_OP = 3,
TOKEN_LPAREN = 4,
TOKEN_RPAREN = 5,
TOKEN_PRIME_OP = 6,
TOKEN_OR = 7,
TOKEN_BAR = 8,
TOKEN_AND = 9,
TOKEN_DOT = 10,
TOKEN_GREATER_EQUAL = 11,
TOKEN_LESS_EQUAL = 12,
TOKEN_NOT_EQUALS = 13,
TOKEN_LESS_THAN = 14,
TOKEN_GREATER_THAN = 15,
TOKEN_NOT = 16,
TOKEN_EQUALS = 17,
TOKEN_MUL_OP = 18,
TOKEN_STRING = 19,
TOKEN_NUMBER = 20,
TOKEN_IF = 21,
TOKEN_THEN = 22,
TOKEN_ELSE = 23,
TOKEN_BOF = 24, // Beginning of file
TOKEN_EOF = 25, // End of file
TOKEN_NULL = 26, // No token
TOKEN_DERIV_OP = 27,
TOKEN_COMMA = 28
};


#endif
