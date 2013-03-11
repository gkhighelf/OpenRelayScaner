/**
 * Created by GKHighElf - gkhighelf@gmail.com
 * in near 2000 year, for self education and use.
**/

#include "OpenRelayScanner.h"
#include "sql.h"

#define CNST_FREE 0
#define CNST_READING 1
#define CNST_SENDING 2
#define CNST_PAUSING 3
#define CNST_LINGERING 4

int num_ready, num_connects, max_connects, cnum, terminate = 0, num_threads = 0;
int subnet_count, subnet_position;
int min_time=999999999, max_time=0, con_processed=0;
struct timeval tv;

typedef struct {
    unsigned int subnet;
    unsigned int mask;
    unsigned int size;
    unsigned int position;
} subnet;

typedef struct {
    int     id;
    char    ip[256];
    int     state;
    int     socket;
    long    secs;
    int     status;
    clock_t start, stop;
    int     bpos;
    int     not;
    unsigned int subnet;
    unsigned int mask;
} url_item;

char sql_update[32*1024];
pthread_t   pt;
int sql_count = 0, smtp_count = 0;
char update_buffer[65535];
pthread_mutex_t sql_mutex;
pthread_cond_t sql_condition;

static url_item* urls;
static subnet* subnets;
int got_exit = 0;
struct hostent* pi;
char tmp_b[32];
float min_t=999999999, max_t=0, avg_t;
long cf=0;
int skipped_subnets = 0, ips_count = 0;


int e200, e404, e503, eTO, eOther;


void set_ndelay( int fd ){int flags, newflags;flags = fcntl( fd, F_GETFL, 0 );if ( flags != -1 ){newflags = flags | (int) O_NDELAY;if ( newflags != flags )(void) fcntl( fd, F_SETFL, newflags );}}
void clear_ndelay( int fd ){int flags, newflags;flags = fcntl( fd, F_GETFL, 0 );if ( flags != -1 ){newflags = flags & ~ (int) O_NDELAY;if ( newflags != flags )(void) fcntl( fd, F_SETFL, newflags );}}

void olp( char *str )
{
    printf( "\r" );
    printf( "%s", str );
    fflush( stdout );
}

void clean_entry( url_item *c, int close_ )
{
//    printf( "Clean up for entry %s\n", c->ip );
    if( close_ ) close( c->socket );
    num_connects--;
    memset( c, 0, sizeof( c ) );
    c->not = 0;
    c->state = CNST_FREE;
}

void progress( void )
{
    char buf[128];
    for(;;)
    {
        usleep(100);
        sprintf( buf, "SMTPs count [ %.4u ] SQLs count [ %.2u ] IPs queued [ %.10u ] Active connections [ %.5d ] Threads [ %.5d ]", smtp_count, sql_count, ips_count, num_connects, num_threads );
        olp( buf );
    }
}
char insert_buffer[65535*2];
void commit()
{
    sprintf( insert_buffer, "insert into smtps VALUES %s;", update_buffer );
    mysql_query( mysql, insert_buffer );
    sql_count = 0;
    update_buffer[0]='\0';
}

void doExit( int exitCode )
{
    char buf[1024];
    if( sql_count > 0 )
    {
        sprintf( buf, "SMTPs count [ %.4u ] SQLs count [ %.2u ] IPs queued [ %.10u ] Active connections [ %.5d ]", smtp_count, sql_count, ips_count, num_connects );
        olp( buf );
        printf( "\nCommiting tailed sqls\n" );
        commit();
    }
    exit( exitCode );
}

void process_sql_updates( void *a )
{
    int i;
    mysql_thread_init();
    pthread_mutex_lock( &sql_mutex );
    for(;;)
    {
        pthread_cond_wait( &sql_condition, &sql_mutex );
        if( sql_count >= 10 )
        {
            commit();
        }
    }
    pthread_mutex_unlock( &sql_mutex );
    mysql_thread_end();
}

void rl( int s, char *buf, int l )
{
    int sz;
    char c;
    memset( buf, 0, l );
    sz = read( s, buf, l );
    buf[ strlen( buf ) - 2 ] = '\0';
}

void process_connection( void *a )
{
    char sql_buf[4096];
    char lb[1024];
    char r1[1024],r2[1024],r3[1024],r4[1024];
    char c;
    int p = 0;
    int s;
    url_item *ui;

    num_threads++;
    memset( sql_buf, 0, 4096 );
    ui = (url_item*)a;
    s = ui->socket;
    clear_ndelay( s );
    smtp_count++;
    rl( s, r1, 1024 );
    sprintf( lb, "HELO some.host.name.com\n" );
    write( s, lb, strlen( lb ) );
    rl( s, r2, 1024 );
    sprintf( lb, "MAIL FROM: <name@some.host.name.com>\n" );
    write( s, lb, strlen( lb ) );
    rl( s, r3, 1024 );
    sprintf( lb, "RCPT TO: <name@some.host.name.com>\n" );
    write( s, lb, strlen( lb ) );
    rl( s, r4, 1024 );
    sprintf( sql_buf, "( %u, %u, inet_aton( '%s' ), \"%s\", \"%s\", \"%s\", \"%s\")", ui->subnet, ui->mask, ui->ip, r1, r2, r3, r4 );
    pthread_mutex_lock( &sql_mutex );
    if( sql_count > 0 ) strcat( update_buffer, "," );
    strcat( update_buffer, sql_buf );
    sql_count++;
    pthread_cond_signal( &sql_condition );
    clean_entry( ui, 1 );
    pthread_mutex_unlock( &sql_mutex );
    num_threads--;
    pthread_exit();
}

static void handle_term( int sig )
{
    doExit( 1 );
}
static void handle_hup( int sig )
{
    doExit( 1 );
}
static void handle_usr1( int sig )
{
    got_exit = 1;
}

int connect_host( url_item* ui )
{
    int res;
    struct  sockaddr_in sin;
    struct   tms t_start;

    ui->socket = socket(AF_INET,SOCK_STREAM,0);
    if( ui->socket == -1 )
    {
        printf( "Error creating socket\n" );
        return -1;
    }
    set_ndelay( ui->socket );

//    printf( "connect_host: connect to host -> %s ip -> %s, socket no -> %d\n", ui->address, ui->ip, ui->socket );
    if( ( sin.sin_addr.s_addr = inet_addr( ui->ip ) ) <=0 ) return -1;
    sin.sin_port        = htons( 25 );
    sin.sin_family      = AF_INET;

    res = connect( ui->socket, (struct sockaddr*)(&sin), sizeof( sin ) );
//    printf( "connect_host: host -> %s ip -> %s errno -> %d res -> %d\n", ui->address, ui->ip, errno, res );
    if( res == -1 && errno != EINTR && errno != EINPROGRESS )
    {
        clean_entry( ui, 1 );
//        printf( "Error connect to host %s\n", ui->ip );
        return -1;
    }
    (void) gettimeofday( &tv, (struct timezone*) 0 );
    ui->secs = tv.tv_sec;
    memset( &t_start, 0, sizeof( t_start ) );
    ui->start = times( &t_start );
    return 1;
}

float
elapsed_time( clock_t time )
{
    long tps = sysconf( _SC_CLK_TCK );
    return (float)time/tps;
}
    

void handle_read( url_item *c )
{
    char    buf[1024];
    int     cnt;
    float   etime;
    struct  tms t_stop;
    char cc;
    int i,el;

    memset( buf, 0, 1024 );
    
    cnt = recvfrom( c->socket, &buf, 1024, MSG_PEEK, NULL, NULL );
    if( cnt == 0 && errno == EINPROGRESS ) return;
    if( cnt < 0 && ( errno == ETIMEDOUT || errno == EHOSTUNREACH || errno == ECONNREFUSED || errno == ECONNRESET || errno == ECONNABORTED ) )
    {
        fdwatch_del_fd( c->socket );
        clean_entry( c, 1 );
        return;
    }
    else
    {
        if( cnt <= 0 ) return;
        if( buf[0] == '2' && buf[1] == '2' && buf[2] == '0' )
        {
            fflush(stdout);
            c->not = 1;
            pthread_create( &pt, NULL, process_connection, c );
            fdwatch_del_fd( c->socket );
            return;
        }
    }
}

void handle_send( url_item *c )
{
    int sz, cnt;
    char buf[1024];

    cnt = recvfrom( c->socket, &buf, 1, MSG_PEEK, NULL, NULL );
    if( ( cnt < 0 && ( errno == ECONNRESET || errno == ECONNABORTED ) ) || cnt == 0 )
    {
        printf( "handle_send: connection error host %s\n", c->ip );
        fdwatch_del_fd( c->socket );
        clean_entry( c, 1 );
    }
}

int find_next_free_entry( url_item* items )
{
    int i;
    for( i=0; i<max_connects; i++ )
      if( items[i].state == CNST_FREE )
        return( i );
    return -1;
}

int clean_timeout( url_item* items )
{
    url_item *c;
    int i;
    for( i=0; i<max_connects; i++ )
    {
        if( items[i].state == CNST_FREE || items[i].not == 1 ) continue;
        (void) gettimeofday( &tv, (struct timezone*) 0 );
        c = &items[i];
        if( c->secs+1 <= tv.tv_sec )
        {
            fdwatch_del_fd( c->socket );
            clean_entry( c, 1 );
            continue;
        }
    }
}

int next_subnet()
{
    if( ++subnet_position >= subnet_count )
    {
        if( skipped_subnets == subnet_count ) return 1;
        subnet_position = skipped_subnets = 0;
    }
    return 0;
}

int main( int argc, char** argv )
{
    char buf[32*1024],buf_[128];
    char bip[32];
    unsigned int cnt;
    int cfi;
    char *ta;
    url_item *c;
    subnet *sn;
    int done;
    unsigned long ccc;

#ifdef HAVE_SIGSET
    (void) sigset( SIGTERM, handle_term );
    (void) sigset( SIGINT, handle_term );
    (void) sigset( SIGPIPE, SIG_IGN );          /* get EPIPE instead */
    (void) sigset( SIGHUP, handle_hup );
    (void) sigset( SIGUSR1, handle_usr1 );
#else /* HAVE_SIGSET */
    (void) signal( SIGTERM, handle_term );
    (void) signal( SIGINT, handle_term );
    (void) signal( SIGPIPE, SIG_IGN );          /* get EPIPE instead */
    (void) signal( SIGHUP, handle_hup );
    (void) signal( SIGUSR1, handle_usr1 );
#endif /* HAVE_SIGSET */
    pthread_mutex_init(&sql_mutex, NULL);
    pthread_cond_init (&sql_condition, NULL);

#ifdef USE_MYSQL4
    mysql = mysql_init(NULL);
    mysql_real_connect(mysql, SQL_HOST, SQL_LOGIN, SQL_PWD, SQL_DB, 0, NULL, 0);
#else
    if (!(mysql_connect(&mysql,SQL_HOST,SQL_LOGIN,SQL_PWD))) exiterr(1);
    if (mysql_select_db(&mysql,SQL_DB)) exiterr(2);
#endif
    if( mysql_query( mysql, "select count(subnet) from world_subnets where country = \"UA\";" ) ) { printf("Error : %s\n", mysql_error(mysql) ); exit(0); }
    res=mysql_store_result(mysql);
    if( ( row = mysql_fetch_row(res) ) != NULL )
    {
        subnet_count = strtol(row[0],NULL,10);
        subnets = NEW( subnet, subnet_count );
    }
    mysql_free_result( res );
    sprintf( buf, "select subnet, mask, ( mask ^ 0xFFFFFFFF ) + 1, pos from world_subnets where country = \"UA\" order by subnet limit 0,%d;", subnet_count );
    if( mysql_query( mysql, buf ) ) { printf("Error : %s\n", mysql_error(mysql) ); exit(0); }
    res=mysql_store_result(mysql);
    cnt = 0;
    while( ( row = mysql_fetch_row(res) ) != NULL )
    {
        if( strtoul( row[0], NULL, 10 ) == 0 ) continue;
        subnets[cnt].subnet = strtoul( row[0], NULL, 10 );
        subnets[cnt].mask = strtoul( row[1], NULL, 10 );
        subnets[cnt].size = strtoul( row[2], NULL, 10 );
        subnets[cnt].position = strtoul( row[3], NULL, 10 );
        if( subnets[cnt].position == 0 ) subnets[cnt].position = 1;
        cnt++;
    }
    mysql_free_result( res );

    max_connects = fdwatch_get_nfiles();
    max_connects = MIN( 1024, max_connects );
    urls = NEW( url_item, max_connects );
    if ( urls == (url_item*) 0 )
	{
	    printf( "out of memory allocating a connecttab\n" );
	    exit( 1 );
	}
    for ( cnum = 0; cnum < max_connects; ++cnum )
	{
	    urls[cnum].state = CNST_FREE;
        memset( urls[cnum].ip, 0, 256 );
	    urls[cnum].socket = 0;
	}
    num_connects = 0;


    done = ccc = cnt = ips_count = subnet_position = 0;
    pthread_create( &pt, NULL, progress, NULL );
    pthread_create( &pt, NULL, process_sql_updates, NULL );
    for(;;)
    {
        clean_timeout( urls );
        while( ! ( ( cfi = find_next_free_entry( urls ) ) == -1 ) )
        {
            if( done ) break;
            while( ( subnets[subnet_position].position >= subnets[subnet_position].size ) || subnets[subnet_position].size == 0 )
            { 
                skipped_subnets++;
                if( ( done = next_subnet() ) == 1 ) break;
            }
            if( done ) break;
            sn = &subnets[subnet_position];
            cnt = sn->subnet + sn->position++;
            sprintf( bip, "%u.%u.%u.%u", cnt >> 24 & 0xFF, cnt >> 16 & 0xff, cnt >> 8 & 0xFF, cnt & 0xff );
            c = &urls[ cfi ];
            
            strcpy( c->ip, bip );
            c->subnet = sn->subnet;
            c->mask = sn->mask;

            if( connect_host( c ) )
            {
                ips_count++;
                num_connects++;
                c->state = CNST_READING;
                fdwatch_add_fd( c->socket, c, FDW_READ );
            }
            if( ( done = next_subnet() ) == 1 ) break;
        }

        if( done && num_connects <= 0 ) break;
        num_ready = fdwatch( 100 );
        if ( num_ready <= 0 )
        {
            continue;
        }

        while ( ( c = (url_item*) fdwatch_get_next_client_data() ) != (url_item*) -1 )
        {
            if ( c == (url_item*) 0 ) continue;
            if ( ! fdwatch_check_fd( c->socket ) )
            {
                fdwatch_del_fd( c->socket );
                clean_entry( c, 1 );
                continue;
            }
            else
            switch ( c->state )
            {
                case CNST_READING: handle_read( c ); break;
                case CNST_SENDING: handle_send( c ); break;
            }
        }
        usleep( 100 );
        if( got_exit ) break; 
    }
    doExit( 0 );
}
