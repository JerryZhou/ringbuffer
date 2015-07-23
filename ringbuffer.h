#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdio.h>
#include <stdarg.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

// Declare The iRingBuffer;
typedef char* iringbuffer;

// RingBuffer Flags
typedef enum irbflag {
    irbflag_blockread = 1,  // Block Read Will Got What We Needed
    irbflag_blockwrite = 1<<1, // Block Write Util We Got Empty Spaces

    irbflag_override = 1<<2,   // Override The Ring Buffer
    
    irbflag_readchannelshut = 1<<3,
    irbflag_writechannelshut = 1<<4,
    
    irbflag_readsleep = 1<<5,
    irbflag_writesleep = 1<<6,
}irbflag;

// Alloc The RingBuffer
iringbuffer irb_alloc(size_t capacity, int flag);

// Free Memory Hold By RingBuffer
void irb_free(iringbuffer buffer);
    
// Close the Read Channel And Write Channel
void irb_close(iringbuffer buffer);
    
// We Can Be ShutDown the Channel
void irb_shutdown(iringbuffer buffer, int flag);

// Write The C String To Buffer
size_t irb_writestr(iringbuffer buffer, const char* str);

// Write the value in length to Buffer
size_t irb_write(iringbuffer buffer, const char* value, size_t length);

// Return the Number Of Readed, Read Content to Dst with Max Length
size_t irb_read(iringbuffer buffer, char* dst, size_t length);

// Return the Length of Content
size_t irb_ready(iringbuffer buffer);

// Return the Buffering Begin
const char* irb_buf(iringbuffer buffer);
    
    

// Print to rb: support 
// %s(c null end string), 
// %i(signed int), 
// %I(signed 64 bit), 
// %u(unsigned int), 
// %U(unsigned 64bit)
size_t irb_catfmt(iringbuffer rb, const char * fmt, ...);

size_t irb_catvprintf(iringbuffer rb, const char* fmt, va_list ap);

size_t irb_catprintf(iringbuffer rb, const char *fmt, ...);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif
