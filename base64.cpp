#include "base64.h"

static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "01234567890+/";

/* pre_declare */
inline unsigned char is_base64(char c);

int base64_encode(char *output, char *input, int inputLen) {

    int i = 0, j = 0, encLen = 0;
    unsigned char a3[3], a4[4];

    while(inputLen--) {
        a3[i++] = *(input++);
        if(i == 3) {

            a4[0] = (a3[0] & 0xfc) >> 2;
            a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
            a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
            a4[3] = (a3[2] & 0x3f);

            for(i = 0; i < 4; i++) {
                output[encLen++] = base64_chars[a4[i]];
            }

            i = 0;
        }
    }

    if(i) {
        for(j = i; j < 3; j++) {
            a3[j] = '\0';
        }

        a4[0] = (a3[0] & 0xfc) >> 2;
        a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
        a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
        a4[3] = (a3[2] & 0x3f);

        for(j = 0; j < i + 1; j++) {
            output[encLen++] = base64_chars[a4[j]];
        }

        while((i++ < 3)) {
            output[encLen++] = '=';
        }
    }
    output[encLen] = '\0';

    return encLen;
}

int base64_decode(char * output, char * input, int inputLen) {

    int i = 0, j = 0, decLen = 0;
    unsigned char a3[3], a4[4];

    while (inputLen--) {
        if(*input == '=') {
            break;
        }

        a4[i++] = *(input++);
        if (i == 4) {
            for (i = 0; i <4; i++) {
                a4[i] = is_base64(a4[i]);
            }

            a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
            a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
            a3[2] = ((a4[2] & 0x3) << 6) + a4[3];

            for (i = 0; i < 3; i++) {
                output[decLen++] = a3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            a4[j] = '\0';
        }

        for (j = 0; j <4; j++) {
            a4[j] = is_base64(a4[j]);
        }

        a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
        a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
        a3[2] = ((a4[2] & 0x3) << 6) + a4[3];

        for (j = 0; j < i - 1; j++) {
            output[decLen++] = a3[j];
        }
    }
    output[decLen] = '\0';
    return decLen;
}

inline unsigned char is_base64(char c) {
    int i;
    for(i = 0; i < 64; i++) {
        if(base64_chars[i] == c) {
            return i;
        }
    }

    return -1;
}
