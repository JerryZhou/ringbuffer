#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdio.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

// Declare The iRingBuffer;
typedef char* iringbuffer;

// RingBuffer Flags
typedef enum irbflag {
    irbflag_blockread = 1,  // Block Read Will Got What We Needed
    irbflag_blockwrite = 2, // Block Write Util We Got Empty Spaces

    irbflag_override = 4,   // Override The Ring Buffer
}irbflag;

// Alloc The RingBuffer
iringbuffer irb_alloc(int capacity, int flag);

// Free Memory Hold By RingBuffer
void irb_free(iringbuffer buffer);

// Write The C String To Buffer
int irb_writestr(iringbuffer buffer, const char* str);

// Write the value in length to Buffer
int irb_write(iringbuffer buffer, const char* value, int length);

// Return the Number Of Readed, Read Content to Dst with Max Length
int irb_read(iringbuffer buffer, char* dst, int length);

// Return the Length of Content
int irb_ready(iringbuffer buffer);

// Return the Buffering Begin
const char* irb_buf(iringbuffer buffer);
    
/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif
