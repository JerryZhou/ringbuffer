#include "ringbuffer.h"
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>

#define _i_u(v) (void)v
#define _i_u_min(u, v) ((u) < (v) ? (u) : (v))

/**
 * */
typedef unsigned long long ilen_t;

/**
 * */
typedef struct _iringbuffer {
    volatile int write;
    volatile int read;

    volatile ilen_t writelen; // may be int64
    volatile ilen_t readlen;  // may be int64

    int flag;

    int  capacity;
    char buf[];
}_iringbuffer;

/**
 * */
#define _irb(rb) ((_iringbuffer*)(rb - sizeof(_iringbuffer)))
#define _irbget(buffer) _iringbuffer *rb = _irb(buffer)

/**
 * */
iringbuffer irb_alloc(int capacity, int flag) {
    _iringbuffer *buffer = (_iringbuffer*)calloc(sizeof(struct _iringbuffer) + capacity + 1, 1);
    buffer->capacity = capacity;
    buffer->buf[capacity] = 0;
    buffer->flag = flag;

    buffer->write = 0;
    buffer->writelen = 0;

    buffer->read = 0;
    buffer->readlen = 0;

    return (char*)buffer->buf;
}

/**
 * */
void irb_free(iringbuffer buffer) {
    _irbget(buffer);

    free(rb);
}

/**
 * */
int irb_writestr(iringbuffer buffer, const char* str) {
    return irb_write(buffer, str, (int)strlen(str));
}

/**
 * */
int irb_write(iringbuffer buffer, const char* value, int length) {
    int empty;
    int write;
    int finish = 0;
    int content;
    _irbget(buffer);

    if (finish < length) do {
        content = (int)(rb->writelen - rb->readlen);

        if (rb->flag & irbflag_override) {
            empty = length;
        } else {
            empty = rb->capacity - content;
        }

        if (empty > 0) do {
            write = rb->capacity - rb->write;
            write = _i_u_min(write, empty);

            memcpy(rb->buf + rb->write, value + finish, write);
            rb->write += write;
            rb->writelen += write;
            if (rb->write >= rb->capacity) {
                rb->write = 0;
            }

            finish += write;
            empty -= write;
        } while(empty > 0);

    } while(finish < length && (rb->flag & irbflag_blockwrite));

    return finish;
}

/**
 * */
int irb_read(iringbuffer buffer, char* dst, int length) {
    int full;
    int read;
    int finish = 0;
    _irbget(buffer);

    if (finish < length) do {

        if (rb->flag & irbflag_override) {
            full = length;
        } else {
            full = (int)(rb->writelen - rb->readlen);
        }

        if (full > 0) do {
            read = rb->capacity - rb->read;
            read = _i_u_min(read, full);

            memcpy(dst + finish, rb->buf + rb->read, read);
            rb->read += read;
            rb->readlen += read;
            if (rb->read >= rb->capacity) {
                rb->read = 0;
            }

            finish += read;
            full -= read;
        } while(full > 0);

    } while(finish < length && (rb->flag & irbflag_blockread));

    return 0;
}

/**
 * */
int irb_ready(iringbuffer buffer) {
    _irbget(buffer);
    return rb->writelen - rb->readlen;
}

/**
 * */
const char* irb_buf(iringbuffer buffer) {
    _irbget(buffer);
    return rb->buf;
}

/* Helper for irg_print() doing the actual number -> string
 * conversion. 's' must point to a string with room for at least
 * _IRB_LLSTR_SIZE bytes.
 *
 * The function returns the length of the null-terminated string
 * representation stored at 's'. */
#define _IRB_LLSTR_SIZE 21

int _irb_ll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    v = (value < 0) ? -value : value;
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p++ = '-';

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Identical _irb_ll2str(), but for unsigned long long type. */
int _irb_ull2str(char *s, unsigned long long v) {
    char *p, aux;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* This function is similar to sdscatprintf, but much faster as it does
 * not rely on sprintf() family functions implemented by the libc that
 * are often very slow. Moreover directly handling the sds string as
 * new data is concatenated provides a performance improvement.
 *
 * However this function only handles an incompatible subset of printf-alike
 * format specifiers:
 *
 * %s - C String
 * %i - signed int
 * %I - 64 bit signed integer (long long, int64_t)
 * %u - unsigned int
 * %U - 64 bit unsigned integer (unsigned long long, uint64_t)
 * %% - Verbatim "%" character.
 */
int irb_print(iringbuffer rb, const char * fmt, ...) {
    const char *f = fmt;
    int i;
    va_list ap;

    char next, *str;
    unsigned int l;
    long long num;
    unsigned long long unum;

    char buf[_IRB_LLSTR_SIZE];

    va_start(ap,fmt);
    f = fmt;    /* Next format specifier byte to process. */
    i = 0;
    while(*f) {
        
        /* Make sure there is always space for at least 1 char. */
        switch(*f) {
        case '%':
            next = *(f+1);
            f++;
            switch(next) {
            case 's':
            case 'S':
                str = va_arg(ap,char*);
                l = strlen(str);//(next == 's') ?  : sdslen(str);
                irb_write(rb, str, l);
                i += l;
                break;
            case 'i':
            case 'I':
                if (next == 'i')
                    num = va_arg(ap,int);
                else
                    num = va_arg(ap,long long);
                {
                    l = _irb_ll2str(buf,num);
                    irb_write(rb, buf, l);
                    i += l;
                }
                break;
            case 'u':
            case 'U':
                if (next == 'u')
                    unum = va_arg(ap,unsigned int);
                else
                    unum = va_arg(ap,unsigned long long);
                {
                    l = _irb_ull2str(buf,unum);
                    irb_write(rb, buf, l);
                    i += l;
                }
                break;
            default: /* Handle %% and generally %<unknown>. */
                irb_write(rb, f, 1);
                break;
            }
            break;
        default:
            irb_write(rb, f, 1);
            break;
        }
        f++;
    }
    va_end(ap);

    return i;
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

    iringbuffer rb = irb_alloc(4096*2, irbflag_override);
    int64_t tick = ccgetcurnano();
    const char * simplelog = "Ts %I, SimpleLog End %% %% %%, %s In It\n";
    for(int i=0; i<10000; ++i) {
        irb_print(rb, simplelog, ccgetcurnano(), "Nothing");
    }
    int64_t since = ccgetcurnano() - tick;
    irb_print(rb, "it take %lld nanos\n", since);
    printf("%s\n", rb);

    irb_free(rb);
    return 0;
}
