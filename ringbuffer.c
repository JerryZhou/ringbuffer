#include "ringbuffer.h"
#include <memory.h>
#include <stdlib.h>

#define _i_u(v) (void)v
#define _i_u_min(u, v) ((u) < (v) ? (u) : (v))

typedef struct iringbuffer {
    volatile int write;
    volatile int writelen;

    volatile int read;
    volatile int readlen;

    int flag;

    int  capacity;
    char buf[];
}iringbuffer;

struct iringbuffer *irb_alloc(int capacity, int flag) {
    iringbuffer *buffer = (iringbuffer*)calloc(sizeof(struct iringbuffer) + capacity + 1, 1);
    buffer->capacity = capacity;
    buffer->buf[capacity] = 0;
    buffer->flag = flag;

    buffer->write = 0;
    buffer->writelen = 0;

    buffer->read = 0;
    buffer->readlen = 0;

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
    int content;

    if (finish < length) do {
        content = buffer->writelen - buffer->readlen;

        if (buffer->flag & irbflag_override) {
            empty = length;
        } else {
            empty = buffer->capacity - content;
        }

        if (empty > 0) do {
            write = buffer->capacity - buffer->write;
            write = _i_u_min(write, empty);

            memcpy(buffer->buf + buffer->write, value + finish, write);
            buffer->write += write;
            buffer->writelen += write;
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
    int full;
    int read;
    int finish = 0;

    if (finish < length) do {

        if (buffer->flag & irbflag_override) {
            full = length;
        } else {
            full = buffer->writelen - buffer->readlen;
        }

        if (full > 0) do {
            read = buffer->capacity - buffer->read;
            read = _i_u_min(read, full);

            memcpy(dst + finish, buffer->buf + buffer->read, read);
            buffer->read += read;
            buffer->readlen += read;
            if (buffer->read >= buffer->capacity) {
                buffer->read = 0;
            }

            finish += read;
            full -= read;
        } while(full > 0);

    } while(finish < length && (buffer->flag & irbflag_blockread));

    return 0;
}

void irb_print(iringbuffer *rb, const char* str) {
    irb_writestr(rb, str);
    irb_writestr(rb, "\n");
}

// get current system time in nanos 
#include <sys/time.h>

int64_t ccgetcurnano() {
     struct timeval tv;
         
     gettimeofday(&tv, NULL);
     return tv.tv_sec*1000*1000 + tv.tv_usec;
}

int main(int argc, const char* argv[]) {
    _i_u(argc);
    _i_u(argv);

    iringbuffer * rb = irb_alloc(4096*2, irbflag_override);
    int64_t tick = ccgetcurnano();
    const char * simplelog = "12234534567890qwertyuiopsdfghjkl;zxcvbnm,.sdfghjkertyui1234567890fghjkl;'bnm,./rtyuiwertyusdfghjkdcvbnqwertywerty";
    for(int i=0; i<10000000; ++i) {
        irb_print(rb, simplelog);
    }
    int64_t since = ccgetcurnano() - tick;
    printf("it take %lld nanos\n", since);
    printf("%s\n", rb->buf);

    irb_free(rb);
    return 0;
}
