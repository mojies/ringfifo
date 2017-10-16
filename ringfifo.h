#ifndef __RING_FIFO_H
#define __RING_FIFO_H

// ring data type
typedef char rd_t;

extern void    *ringfifo_api_create( int ilen, int iunmbs );
extern int      ringfifo_api_destroy( void **iring );
extern int      ringfifo_api_user_add( void *iring );
extern int      ringfifo_api_user_del( void *iring, int iindex );
extern int      ringfifo_api_put( void *iring, rd_t *ibuf, int ilen );
extern int      ringfifo_api_put_block( void *iring, rd_t *ibuf, int ilen );
extern int      ringfifo_api_get( void *iring, int iindex, rd_t *ibuf, int ilen );
extern int      ringfifo_api_get_noblock( void *iring, int iindex, rd_t *ibuf, int ilen );
extern int      ringfifo_api_get_data_len( void *iring, int iindex );
extern int      ringfifo_api_clr_usr_data( void  *iring, int iindex, int ilen );
extern int      ringfifo_api_clr_all_usr( void *iring );

#endif
