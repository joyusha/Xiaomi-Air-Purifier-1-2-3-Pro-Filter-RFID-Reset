#ifndef UTILS_SHA1_H_
#define UTILS_SHA1_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdio.h"

//SHA-1 context structure
typedef struct
{
  uint32_t total[2];         //number of bytes processed
  uint32_t state[5];         //intermediate digest state
  unsigned char buffer[64];  //data block being processed
} iot_sha1_context;

//Initialize SHA-1 context
//SHA-1 context to be initialized
void utils_sha1_init(iot_sha1_context *ctx);

//Clear SHA-1 context
// SHA-1 context to be cleared
void utils_sha1_free(iot_sha1_context *ctx);

//Clone (the state of) a SHA-1 context
//The destination context
//The context to be cloned
void utils_sha1_clone(iot_sha1_context *dst, const iot_sha1_context *src);


//SHA-1 context setup
//context to be initialized
void utils_sha1_starts(iot_sha1_context *ctx);


//  SHA-1 process buffer
//  SHA-1 context
//  buffer holding the  data
//  length of the input data
void utils_sha1_update(iot_sha1_context *ctx, const unsigned char *input, size_t ilen);

//SHA-1 final digest
//SHA-1 context
//SHA-1 checksum result
void utils_sha1_finish(iot_sha1_context *ctx, unsigned char output[20]);

//Internal use
void utils_sha1_process(iot_sha1_context *ctx, const unsigned char data[64]);

// Output = SHA-1( input buffer )
//input    buffer holding the  data
//ilen     length of the input data
// output   SHA-1 checksum result
void utils_sha1(const unsigned char *input, size_t ilen, unsigned char output[20]);

//transform hex format
int8_t utils_hb2hex(uint8_t hb);

void utils_jww(void);

#ifdef __cplusplus
}
#endif

#endif
