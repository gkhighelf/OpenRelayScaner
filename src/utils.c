/**
 * Created by GKHighElf - gkhighelf@gmail.com
 * in near 2000 year, for self education and use.
**/

#include "OpenRelayScanner.h"

int lookup_host (const char *host, char *ret )
{
    struct addrinfo hints, *res;
    int errcode;
    void *ptr;

    memset(&hints, 0, sizeof (hints));
//    memset( res, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo (host, NULL, &hints, &res);
    if (errcode != 0)
    {
//        perror("getaddrinfo");
        //printf("Error: can't resolve host %s\n", host );
        return -1;
    }

//    while (res)
//    {
//        inet_ntop (res->ai_family, res->ai_addr->sa_data, res, 100);

        switch (res->ai_family)
        {
            case AF_INET:
                ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                break;
            case AF_INET6:
                ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                break;
        }
        inet_ntop(res->ai_family, ptr, ret, 256);
//    }

    return 0;
}

void resolv_host( char *host, char *res )
{
    struct hostent *pp, *gethostbyname();

    for(;;)
    {
        memset( pp, 0, sizeof( pp ) );
        pp = gethostbyname( host );
        printf( "%s\n", hstrerror( h_errno ) );
        if( h_errno == TRY_AGAIN ) continue;
        else break;
    }
    if( pp == (struct hostent*) 0 )
    {
        printf( "Error %d resolving host %s\n", h_errno, host );
        res[0] = 0x0;
        return;
    }
//    bcopy( pHostInfo.h_addr, &ttt, sizeof( pHostInfo.h_addr ) );
    sprintf( res, "%s", (char*)inet_ntoa( *(struct in_addr*)( pp->h_addr ) ) );
}

