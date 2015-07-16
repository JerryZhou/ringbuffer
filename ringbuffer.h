#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdio.h>

struct iringbuffer; 

struct iringbuffer *irb_alloc(int capacity, int flag);

void irb_free(struct iringbuffer *buffer);

int irb_writestr(struct iringbuffer *buffer, const char* str);

int irb_write(struct iringbuffer *buffer, const char* value, int length);

int irb_read(struct iringbuffer *buffer, char* dst, int length);

#endif
