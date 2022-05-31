#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmnutil.h"

int jsmn_compare(const char *js, jsmntok_t *token, const char *str)
{

    if (token->type == JSMN_STRING && 
       ((int) strlen(str) == token->end - token->start) &&
        strncmp(js + token->start, str, token->end - token->start) == 0) {
           
           return 0;

    } else {
    
        return -1;

    }

}

/*
   In order to find a particular value among tokens, one needs to
   look through all tokens until the content of one matches the
   JSON key. Then the next token certainly holds the coresponding
   value.
*/
jsmntok_t *find_by_key(const char *js, const char *key, jsmntok_t tokens[], size_t toklen)
{

    for (int i = 0; i < toklen; i++) {

        if (jsmn_compare(js, &tokens[i], key) == 0) {

            return &tokens[i + 1];

        }

    }

    return NULL;

}

/*
    While find_by_key returns the token corresponding to the value,
    that is only useful for extracting values out of tokens.
    When searching through an array, one needs the actual index of
    the start of the array. This function can be used to find the index.
*/
int find_idx_by_key(const char *js, const char *key, jsmntok_t tokens[], size_t toklen)
{

    for (int i = 0; i < toklen; i++) {

        if (jsmn_compare(js, &tokens[i], key) == 0) {

            return (i + 1);

        }

    }

    return -1;

}

/*
    Allocates a new string on the heap and copies (zero-terminated)
    the string value which the token represents into the newly
    allocated string.

    The string has to be freed manually once it is no longer useful.
*/
char *get_simple(const char *js, jsmntok_t *token)
{

    int len = token->end - token->start;
    char *str = malloc(sizeof *str * (len + 1));

    if (str == NULL) {

        printf("Could not allocate enough memory to store token value!\n");

        int errcode = errno;

        char *errstr = strerror(errcode);
        printf("Err: %s\n", errstr);
        exit(EXIT_FAILURE);

    }

    str[0] = '\0';
    strncat(str, (js + token->start), len);

    return str;

}

/*
    Allocates an array of strings onto the heap and then copies all
    string values from the JSON array into the newly allocated array.

    Essentially works the same as get_simple, except it treats each
    new string as the column of a 2D array.

    The array has to be freed manually once it is no longer useful.
*/
char **get_array(const char *js, int idx, jsmntok_t tokens[], size_t arrlen)
{

    char **arr = malloc(arrlen * sizeof(*arr));

    /*
        i represents the offset (idx + 1; since idx points to '['
        instead of first element) for token search

        j is the index of each row
    */
    for (int i = idx + 1, j = 0; j < arrlen; i++, j++) {

        jsmntok_t curr = tokens[i];
        int len = curr.end - curr.start;

        arr[j] = malloc(sizeof(arr[0]) * (len + 1));

        if (arr[j] == NULL) {

        printf("Could not allocate enough memory to store array string value!\n");

        int errcode = errno;

        char *errstr = strerror(errcode);
        printf("Err: %s\n", errstr);
        exit(EXIT_FAILURE);

        }

        arr[j][0] = '\0';
        strncat(arr[j], (js + curr.start), len);

    }

    return arr;

}

void free_array(char **arr, size_t arrlen)
{
	
	for (int i = 0; i < arrlen; i++) {

		free(arr[i]);

	}

	free(arr);

}
