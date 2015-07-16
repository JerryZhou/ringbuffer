#include "ringbuffer.h"
#include <memory.h>
#include <stdlib.h>

#define _i_u(v) (void)v
#define _i_u_min(u, v) ((u) < (v) ? (u) : (v))

typedef struct iringbuffer {
    volatile int write;
    volatile int read;
    volatile int content;

    int flag;

    int  capacity;
    char buf[];
}iringbuffer;

typedef enum irbflag {
    irbflag_blockread = 1,
    irbflag_blockwrite = 2,

    irbflag_override = 4, 
}irbflag;

struct iringbuffer *irb_alloc(int capacity, int flag) {
    iringbuffer *buffer = (iringbuffer*)calloc(sizeof(struct iringbuffer) + capacity + 1, 1);
    buffer->capacity = capacity;
    buffer->buf[capacity] = 0;
    buffer->flag = flag;
    buffer->write = 0;
    buffer->read = 0;
    buffer->content = 0;

    return buffer;
}

void irb_free(struct iringbuffer *buffer) {
    free(buffer);
}

int irb_writestr(struct iringbuffer *buffer, const char* str) {
    return irb_write(buffer, str, strlen(str));
}

int irb_write(struct iringbuffer *buffer, const char* value, int length) {
    int empty;
    int write;
    int finish = 0;

    if (finish < length) do {
        if (buffer->flag & irbflag_override) {
            empty = length;
        } else {
            empty = buffer->capacity - buffer->content;
        }

        if (empty > 0) do {
            write = buffer->capacity - buffer->write;
            write = _i_u_min(write, empty);

            memcpy(buffer->buf + buffer->write, value + finish, write);
            buffer->write += write;
            buffer->content += write;
            if (buffer->write >= buffer->capacity) {
                buffer->write = 0;
            }

            finish += write;
            empty -= write;
        } while(empty > 0);

    } while(finish < length && (buffer->flag & irbflag_blockwrite));

    return finish;
}

int irb_read(struct iringbuffer *buffer, char* dst, int length) {
    int content;
    int read;
    int finish = 0;

    if (finish < length) do {
        if (buffer->flag & irbflag_override) {
            content = length;
        } else {
            content = buffer->content;
        }

        if (content > 0) do {
            read = buffer->capacity - buffer->read;
            read = _i_u_min(read, content);

            memcpy(dst + finish, buffer->buf + buffer->read, read);
            buffer->read += read;
            buffer->content -= read;
            if (buffer->read >= buffer->capacity) {
                buffer->read = 0;
            }

            finish += read;
            content -= read;
        } while(content > 0);

    } while(finish < length && (buffer->flag & irbflag_blockread));

    return 0;
}

void irb_pint(iringbuffer *rb, const char* str) {
    irb_writestr(rb, str);
    irb_writestr(rb, "\n");
}

int main(int argc, const char* argv[]) {
    _i_u(argc);
    _i_u(argv);

    iringbuffer * rb = irb_alloc(2, irbflag_override);

    irb_pint(rb, "hello");
    irb_pint(rb, "word");

    printf("%s\n", rb->buf);

    irb_free(rb);
    return 0;
}
