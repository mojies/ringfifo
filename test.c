#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "ringfifo.h"
#include "Debug.h"


#define T_FILE_SOU      "sour.pcm"
#define T_FILE_DES1     "des1.pcm"
#define T_FILE_DES2     "des2.pcm"


void *ringfifo;
unsigned debug_level = 8;



void *send_pthread( void* pdata ){
    int fd;
    char buf[ 480 ];
    int ret;

    fd = open( T_FILE_SOU, O_RDONLY );
    while(1){
        if( ( ret = read( fd, buf, 480 ) ) <= 0 ){
            break;
        }
        // DLLOGW( "read -> %d", ret );
        ringfifo_api_put( ringfifo, buf, ret );
        usleep( 1000 );
    }
}

void *recv_pthread( void* pdata ){
    char *filename = pdata;
    char buf[3200];
    int user_index;
    int ret;
    int fd;

    fd = open( filename, O_WRONLY );
    user_index = ringfifo_api_user_add( ringfifo );
    // DLLOGW("user index -> %d", user_index);
    while(1){
        ret = ringfifo_api_get( ringfifo, user_index, buf, 3200 );
        write( fd, buf, ret );
    }
}

int main(void){

    system( "test -f "T_FILE_DES1" && rm "T_FILE_DES1 );
    system( "test -f "T_FILE_DES2" && rm "T_FILE_DES2 );

    system( "touch "T_FILE_DES1" && chmod 777 "T_FILE_DES1 );
    system( "touch "T_FILE_DES2" && chmod 777 "T_FILE_DES2 );

    // ----------------------------------------
    ringfifo = ringfifo_api_create( 48000*2, 3 );
    {
        pthread_t thrd_id;
        pthread_create( &thrd_id, NULL, recv_pthread, T_FILE_DES1 );
        pthread_create( &thrd_id, NULL, recv_pthread, T_FILE_DES2 );
        pthread_create( &thrd_id, NULL, send_pthread, NULL );
        pthread_join( thrd_id, NULL );
    }

    ringfifo_api_destroy( ringfifo );
    // ----------------------------------------

    return 0;
}
