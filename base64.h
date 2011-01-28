#ifndef _BASE64_H
#define _BASE64_H

/*
 *     Credits:
 *         https://github.com/adamvr/arduino-base64/blob/master/Base64.c
 *         https://github.com/pkrumins/node-base64/blob/master/base64.cc
 */


/* base64_chars:
 *         Description: Base64 alphabet table, a mapping between integers
 *                      and base64 digits
 */
extern const char base64_chars[];

/* base64_encode:
 *         Description:
 *             Encode a string of characters as base64
 *         Parameters:
 *             output: the output buffer for the encoding, stores the encoded string
 *             input: the input buffer for the encoding, stores the binary to be encoded
 *             inputLen: the length of the input buffer, in bytes
 *         Return value:
 *             Returns the length of the encoded string
 *         Requirements:
 *             1. output must not be null or empty
 *             2. input must not be null
 *             3. inputLen must be greater than or equal to 0
 */
int base64_encode(char *output, char *input, int inputLen);

/* base64_decode:
 *         Description:
 *             Decode a base64 encoded string into bytes
 *         Parameters:
 *             output: the output buffer for the decoding,
 *                     stores the decoded binary
 *             input: the input buffer for the decoding,
 *                    stores the base64 string to be decoded
 *             inputLen: the length of the input buffer, in bytes
 *         Return value:
 *             Returns the length of the decoded string
 *         Requirements:
 *             1. output must not be null or empty
 *             2. input must not be null
 *             3. inputLen must be greater than or equal to 0
 */
int base64_decode(char *output, char *input, int inputLen);

#endif