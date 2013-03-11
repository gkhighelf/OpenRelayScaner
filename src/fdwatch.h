#ifndef _FDWATCH_H_
#define _FDWATCH_H_

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#define NEW(t,n) ((t*) malloc( sizeof(t) * (n) ))
#define RENEW(o,t,n) ((t*) realloc( (void*) o, sizeof(t) * (n) ))

#define FDW_READ 0
#define FDW_WRITE 1

#ifndef INFTIM
#define INFTIM -1
#endif

extern int fdwatch_get_nfiles( void );
extern void fdwatch_add_fd( int fd, void* client_data, int rw );
extern void fdwatch_del_fd( int fd );
extern int fdwatch( long timeout_msecs );
extern int fdwatch_check_fd( int fd );
extern void* fdwatch_get_next_client_data( void );
extern void fdwatch_logstats( long secs );

#endif
