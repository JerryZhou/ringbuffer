#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdio.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

struct iringbuffer; 

typedef enum irbflag {
    irbflag_blockread = 1,
    irbflag_blockwrite = 2,

    irbflag_override = 4, 
}irbflag;

struct iringbuffer *irb_alloc(int capacity, int flag);

void irb_free(struct iringbuffer *buffer);

int irb_writestr(struct iringbuffer *buffer, const char* str);

int irb_write(struct iringbuffer *buffer, const char* value, int length);

int irb_read(struct iringbuffer *buffer, char* dst, int length);
    
/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif
