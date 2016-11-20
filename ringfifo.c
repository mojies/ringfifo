#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ringfifo.h"
// #include "../tests/Debug.h"
#include "Debug.h"

#define DC_DEF_ARING_BUF_LEN            4096
#define DC_DEF_ARING_BUF_USER_SUM       8

typedef struct{
    int                 a_index;
    int                 a_point;
    int                 a_len;
}m_ring_user;

typedef struct {
    pthread_mutex_t     a_mutex;
    pthread_cond_t      a_cond;
    pthread_mutex_t     a_cond_mutex;
    rd_t               *a_buf;
    int                 a_maxlen;
    int                 a_len;
    int                 a_head;
    int                 a_tail;
    int                 a_unmbs;
    m_ring_user       **a_user;
    int                 a_index;
}m_ring;

// ----------------------------------------> audio ring buf
void *ringfifo_api_create( int ilen, int inmbs ){
    m_ring     *tp_ring;
    int         i;

    tp_ring = ( m_ring* )calloc( 1, sizeof( m_ring ) );
    if( tp_ring == NULL ) return NULL;

    if( ilen == 0 ) tp_ring->a_maxlen = DC_DEF_ARING_BUF_LEN;
    else tp_ring->a_maxlen = ilen;
    tp_ring->a_buf = (rd_t *)calloc( 1, sizeof(rd_t)*tp_ring->a_maxlen );
    if( tp_ring->a_buf == NULL ) goto gt_ring_buf_init_freering;

    if( inmbs == 0 ) tp_ring->a_unmbs = DC_DEF_ARING_BUF_USER_SUM;
    else tp_ring->a_unmbs = inmbs;
    tp_ring->a_user = (m_ring_user**)calloc( 1, sizeof( void* ) * tp_ring->a_unmbs  );
    if( tp_ring->a_user == NULL ) goto gt_ring_buf_init_freebuf;
    /*
    for( i = 0; i < tp_ring->a_unmbs; i++ )
        tp_ring->a_user[ i ] = NULL;
    */
    pthread_mutex_init( &tp_ring->a_mutex, NULL );
    pthread_cond_init( &tp_ring->a_cond, NULL );
    pthread_mutex_init( &tp_ring->a_cond_mutex, NULL );

    return tp_ring;
gt_ring_buf_init_freebuf:
    free( tp_ring->a_buf );
gt_ring_buf_init_freering:
    free( tp_ring );
    return NULL;
}

int ringfifo_api_destroy( void  *iring ){
    m_ring     *tp_ring = iring;
    int         i;

    if( tp_ring == NULL ) return -1;
    pthread_mutex_lock( &tp_ring->a_mutex );
    pthread_mutex_unlock( &tp_ring->a_mutex );
    pthread_mutex_destroy( &tp_ring->a_mutex );
    pthread_cond_destroy( &tp_ring->a_cond );
    pthread_mutex_destroy( &tp_ring->a_cond_mutex );
    for( i = 0; i < tp_ring->a_unmbs; i++ )
        if( tp_ring->a_user[ i ] )
            free( tp_ring->a_user[i] );
    free( tp_ring->a_user );
    free( tp_ring->a_buf );
    free( tp_ring );

    return 0;
}

int ringfifo_api_user_add( void  *iring ){
    m_ring         *tp_ring = iring ;
    int             tv_index;
    m_ring_user    *tp_user;
    int             tv_done;
    int             i;

    if( tp_ring == NULL ) return -1;
    pthread_mutex_lock( &tp_ring->a_mutex );
    for( i = 0; i < tp_ring->a_unmbs; i++ )
        if( tp_ring->a_user[i] == NULL )
            break;
    if( i == tp_ring->a_unmbs )
        goto GT_ring_user_add_err;

    tp_user = (m_ring_user*)calloc( 1, sizeof(m_ring_user) );
    if( tp_user == NULL )
        goto GT_ring_user_add_err;
    tp_ring->a_user[ i ] = tp_user;

    do{
        tv_done = 0;
        tp_ring->a_index++;
        if( tp_ring->a_index < 0 ) tp_ring->a_index = 0;
        for( i = 0; i < tp_ring->a_unmbs; i++ ){
            if( ( tp_ring->a_user[i] )\
             && ( tp_ring->a_user[i]->a_index == tp_ring->a_index ) ){
                tv_done = 1;
                continue;
            }
        }
    }while( tv_done );

    tp_user->a_index = tp_ring->a_index;
    tp_user->a_point = tp_ring->a_head;
    tp_user->a_len = 0;

    pthread_mutex_unlock( &tp_ring->a_mutex );
    return tp_user->a_index;
GT_ring_user_add_err:
    pthread_mutex_unlock( &tp_ring->a_mutex );
    return -1;
}



int ringfifo_api_user_del( void  *iring, int iindex ){
    m_ring         *tp_ring = iring;
    m_ring_user    *tp_user;
    int             i;

    if( tp_ring == NULL ) return -1;
    pthread_mutex_lock( &tp_ring->a_mutex );
    for( i = 0; i < tp_ring->a_unmbs; i++ )
        if( ( tp_ring->a_user[ i ] )\
         && ( tp_ring->a_user[ i ]->a_index == iindex ) ){
            tp_user = tp_ring->a_user[ i ];
            tp_ring->a_user[ i ] = NULL;
        }
    pthread_mutex_unlock( &tp_ring->a_mutex );
    if( i == tp_ring->a_unmbs ) return -1;
    free( tp_user );
    return 0;
}



int ringfifo_api_put( void  *iring, rd_t *ibuf, int ilen ){
    m_ring     *tp_ring = iring;
    m_ring_user    *tp_user;
    int              i;

    if( tp_ring->a_buf == NULL ) return -1;
    if( ibuf == NULL ) return -1;
    if( ilen > tp_ring->a_maxlen ) return -1;

    pthread_mutex_lock( &tp_ring->a_mutex );
    // is the ilde space enough
#if RINGFIFO_DEBUG
    DLLOGD( "ring put : ilde space 1-> %d", tp_ring->a_maxlen - tp_ring->a_len  );
#endif
    if( ilen > ( tp_ring->a_maxlen - tp_ring->a_len ) ){
        // free space
        tp_ring->a_tail = tp_ring->a_head + ilen;
        if( tp_ring->a_tail >= tp_ring->a_maxlen )
            tp_ring->a_tail -= tp_ring->a_maxlen;
        // clear user old data
        for( i = 0; i < tp_ring->a_unmbs; i++ ){
            if( tp_ring->a_user[ i ] ){
                tp_user = tp_ring->a_user[ i ];
                if( tp_ring->a_head > tp_ring->a_tail )
                    if( ( tp_user->a_point > tp_ring->a_head )\
                     || ( tp_user->a_point < tp_ring->a_tail )\
                     || ( ( tp_user->a_point == tp_ring->a_head )\
                       && ( tp_user->a_len > tp_ring->a_maxlen/2 ) ) )
                        tp_user->a_point = tp_ring->a_tail;
                if( tp_ring->a_head < tp_ring->a_tail )
                    if( ( ( tp_user->a_point > tp_ring->a_head )\
                       && ( tp_user->a_point < tp_ring->a_tail ) )\
                     || ( ( tp_user->a_point == tp_ring->a_head )\
                       && ( tp_user->a_len > tp_ring->a_maxlen/2 ) ) )
                        tp_user->a_point = tp_ring->a_tail;
                if( tp_ring->a_head >= tp_user->a_point ){
#if RINGFIFO_DEBUG
                    DLLOGW( "POS1 : %d lossing %d", \
                            tp_user->a_index,\
                            tp_user->a_len - tp_ring->a_head + tp_user->a_point );
#endif
                    tp_user->a_len = tp_ring->a_head - tp_user->a_point;
                }else{
#if RINGFIFO_DEBUG
                    DLLOGW( "POS2 : %d lossing %d", \
                            tp_user->a_index,\
                            tp_user->a_len - tp_ring->a_maxlen + tp_user->a_point - tp_ring->a_head );
#endif
                    tp_user->a_len = tp_ring->a_maxlen - tp_user->a_point + tp_ring->a_head;
                }
            }
        }
        tp_ring->a_len = tp_ring->a_maxlen - ilen;
    }
#if RINGFIFO_DEBUG
    // DLLOGD( "ring put : ilde space 2-> %d", tp_ring->a_maxlen - tp_ring->a_len  );
#endif

#if 1
    if( tp_ring->a_head < tp_ring->a_maxlen ){
        unsigned cp_len = tp_ring->a_maxlen - tp_ring->a_head;
        if( cp_len >= ilen  ){
            memcpy( &tp_ring->a_buf[ tp_ring->a_head ], ibuf, ilen );
            tp_ring->a_head += ilen;
            if( tp_ring->a_head == tp_ring->a_maxlen )
                tp_ring->a_head = 0;
        }else if( cp_len < ilen ){
            memcpy( &tp_ring->a_buf[ tp_ring->a_head ], ibuf, cp_len );
            i = cp_len;
            cp_len = ilen - cp_len;
            memcpy( &tp_ring->a_buf[ 0 ], &ibuf[ i ], cp_len );
            tp_ring->a_head = cp_len;
        }
    }
#else
    for( i = 0; ( tp_ring->a_head < tp_ring->a_maxlen ) && ( i < ilen ); i++ )
        tp_ring->a_buf[ tp_ring->a_head++ ] = ibuf[ i ];
    if( i < ilen )
        for( tp_ring->a_head = 0; i < ilen; i++ )
            tp_ring->a_buf[ tp_ring->a_head++ ] = ibuf[ i ];
    if( tp_ring->a_head == tp_ring->a_maxlen )
        tp_ring->a_head = 0;
#endif
    tp_ring->a_len += ilen;
    for( i = 0; i < tp_ring->a_unmbs; i++ )
        if( tp_ring->a_user[ i ] )
            tp_ring->a_user[ i ]->a_len += ilen;
    pthread_mutex_unlock( &tp_ring->a_mutex );
    // pthread_cond_signal( &tp_ring->a_cond );
    pthread_cond_broadcast( &tp_ring->a_cond );

    return 0;
}



int ringfifo_api_get( void  *iring, int iindex, rd_t *ibuf, int ilen ){
    m_ring         *tp_ring = iring;
    m_ring_user    *tp_user;
    int             i;
    int             tv_min_point;
    int             tv_max_len;

    if( tp_ring == NULL ) return -1;
    if( ibuf == NULL ) return -1;
    if( ilen == 0 ) return 0;
#if RINGFIFO_DEBUG
    DLLOGD( "ring get : %p - %p - %d", tp_ring, ibuf, ilen );
#endif
    pthread_mutex_lock( &tp_ring->a_mutex );
    for( i = 0; i < tp_ring->a_unmbs; i++ ){
        if( tp_ring->a_user[ i ] && ( tp_ring->a_user[ i ]->a_index == iindex ) ){
            break;
        }
    }
    pthread_mutex_unlock( &tp_ring->a_mutex );
    if( i == tp_ring->a_unmbs ) return -1;

#if RINGFIFO_DEBUG
    DLLOGD( "ring get : u %d", i );
#endif
    while(1){
        if( ( tp_ring->a_user[ i ] == NULL ) ||\
                ( tp_ring->a_user[ i ]->a_index != iindex ) ){
            return -1;
        }else if( tp_ring->a_user[ i ]->a_len >= ilen ){
            // DLLOGD("usrlen : %d - %d", tp_ring->a_user[ i ]->a_len, tp_ring->a_user[ i ]->a_point );
            pthread_mutex_lock( &tp_ring->a_mutex );
            break;
        }
        pthread_cond_wait( &tp_ring->a_cond, &tp_ring->a_cond_mutex );
        // usleep(100000);
    }
    tp_user = tp_ring->a_user[ i ];

    // copy data
    // 
#if 1
    if( tp_user->a_point < tp_ring->a_maxlen ){
        unsigned cp_len = tp_ring->a_maxlen - tp_user->a_point;
        if( cp_len >= ilen ){
            memcpy( ibuf, &tp_ring->a_buf[ tp_user->a_point ], ilen );
            tp_user->a_point += ilen;
            if( tp_user->a_point >= tp_ring->a_maxlen )
                tp_user->a_point = 0;
        }else if( cp_len < ilen ){
            memcpy( ibuf, &tp_ring->a_buf[ tp_user->a_point ], cp_len );
            i = cp_len;
            cp_len = ilen - cp_len;
            memcpy( &ibuf[ i ], &tp_ring->a_buf[0], cp_len );
            tp_user->a_point = cp_len;
        }
    }
#else
    for( i = 0; ( tp_user->a_point < tp_ring->a_maxlen ) && ( i < ilen ); i++ )
        ibuf[ i ] = tp_ring->a_buf[ tp_user->a_point++ ];
    if( i < ilen )
        for( tp_user->a_point = 0; i < ilen; i++ )
            ibuf[ i ] = tp_ring->a_buf[ tp_user->a_point++ ];
    if( tp_user->a_point >= tp_ring->a_maxlen )
        tp_user->a_point = 0;
#endif

    tp_user->a_len -= ilen;
    // if( tp_user->a_len < 0 ) tp_user->a_len = 0;
    // release space
    tv_min_point = tp_ring->a_maxlen;
    tv_max_len = -1;
    for( i = 0; i < tp_ring->a_unmbs; i++  )
        if( tp_ring->a_user[ i ] ){
            tp_user = tp_ring->a_user[ i ];
            if( tp_user->a_len > tv_max_len ){
                tv_max_len = tp_user->a_len;
                tv_min_point = tp_user->a_point;
            }
        }
    if( ( tv_max_len != -1 )&&( tv_min_point != tp_ring->a_maxlen )){
        tp_ring->a_tail = tv_min_point;
        tp_ring->a_len = tv_max_len;
    }

    pthread_mutex_unlock( &tp_ring->a_mutex );
    return ilen;
}


int ringfifo_api_clr_usr_data( void  *iring, int iindex ){
    m_ring     *tp_ring = iring;
    int i;

    if( tp_ring == NULL ) return -1;
    for( i = 0; i < tp_ring->a_unmbs; i++  ){
        if( ( tp_ring->a_user[ i ] )\
         && ( tp_ring->a_user[ i ]->a_index == iindex ) ){
            pthread_mutex_lock( &tp_ring->a_mutex );
#if RINGFIFO_DEBUG
            DLLOGW("Purposeful to clr old buf. len - %d", tp_ring->a_user[ i ]->a_len);
#endif
            tp_ring->a_user[ i ]->a_len = 0;
            tp_ring->a_user[ i ]->a_point = tp_ring->a_head;
            pthread_mutex_unlock( &tp_ring->a_mutex );
            return 0;
        }
    }
    return -1;
}


