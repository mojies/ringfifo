/*
	debug 	color and level
	info	normal(white or green)	
	debug	blue
	warning	yellow
	error	red
*/
#ifndef __MAJ_DEBUG_H
#define __MAJ_DEBUG_H

#include <stdio.h>
#include <sys/time.h>

#define DEBUG_COLOR
extern unsigned debug_level;

// =============================================================================
#define INPUTDEV_DEBUG          0
#define AUDIO_DEVICE_DEBUG      0
#define HWD_ALEXA_DEBUG         0
#define AUDIO_AOUT_DEBUG        0
#define AUDIO_AIN_DEBUG         0
#define AUDIO_RECORD_DEBUG      0
#define BLUETOOTH_DEBUG         0
#define RINGFIFO_DEBUG          0





// =============================================================================
enum DLOG_LEVEL{
    DL_SILENCE = 0,
    DL_ERROR   = 1,
    DL_WARN    = 2,
    DL_INFO    = 3,
    DL_DEBUG   = 4,
    DL_VERBOSE = 5,
};
#define DLOGCOMLN(level,x,y...)    \
do{\
    if(level <= debug_level) printf(x"\n",##y);\
}while(0)

#define DLOGCOM(level,x,y...)    \
do{\
    if(level <= debug_level) printf(x,##y);\
}while(0)

// =============================================================================
// belong to test application
#define CL_NONE         "\e[m"
#define CL_RED          "\e[0;32;31m"
#define CL_LIGHT_RED    "\e[1;31m"
#define CL_GREEN        "\e[0;32;32m"
#define CL_LIGHT_GREEN  "\e[1;32m"
#define CL_BLUE         "\e[0;32;34m"
#define CL_LIGHT_BLUE   "\e[1;34m"
#define CL_DARY_GRAY    "\e[1;30m"
#define CL_CYAN         "\e[0;36m"
#define CL_LIGHT_CYAN   "\e[1;36m"
#define CL_PURPLE       "\e[0;35m"
#define CL_LIGHT_PURPLE "\e[1;35m"
#define CL_BROWN        "\e[0;33m"
#define CL_YELLOW       "\e[1;33m"
#define CL_LIGHT_GRAY   "\e[0;37m"
#define CL_WHITE        "\e[1;37m"

// =============================================================================
#ifdef DEBUG_COLOR
#define DLLOGE(x, y...)   DLOGCOMLN( DL_ERROR,  CL_LIGHT_RED     x CL_NONE, ##y)
#define DLLOGW(x, y...)   DLOGCOMLN( DL_WARN,   CL_YELLOW        x CL_NONE, ##y)
#define DLLOGI(x, y...)   DLOGCOMLN( DL_INFO,   CL_LIGHT_BLUE    x CL_NONE, ##y)
#define DLLOGD(x, y...)   DLOGCOMLN( DL_DEBUG,  CL_LIGHT_CYAN    x CL_NONE, ##y)
#define DLLOGV(x, y...)   DLOGCOMLN( DL_VERBOSE,CL_LIGHT_GREEN   x CL_NONE, ##y)
#define DLOGE(x, y...)   DLOGCOM( DL_ERROR,  CL_LIGHT_RED     x CL_NONE, ##y)
#define DLOGW(x, y...)   DLOGCOM( DL_WARN,   CL_YELLOW        x CL_NONE, ##y)
#define DLOGI(x, y...)   DLOGCOM( DL_INFO,   CL_LIGHT_BLUE    x CL_NONE, ##y)
#define DLOGD(x, y...)   DLOGCOM( DL_DEBUG,  CL_LIGHT_CYAN    x CL_NONE, ##y)
#define DLOGV(x, y...)   DLOGCOM( DL_VERBOSE,CL_LIGHT_GREEN   x CL_NONE, ##y)
#else
#define DLLOGE(x, y...)   DLOGCOMLN( DL_ERROR,  x, ##y)
#define DLLOGW(x, y...)   DLOGCOMLN( DL_WARN,   x, ##y)
#define DLLOGI(x, y...)   DLOGCOMLN( DL_INFO,   x, ##y)
#define DLLOGD(x, y...)   DLOGCOMLN( DL_DEBUG,  x, ##y)
#define DLLOGV(x, y...)   DLOGCOMLN( DL_VERBOSE,x, ##y)
#define DLOGE(x, y...)   DLOGCOM( DL_ERROR,  x, ##y)
#define DLOGW(x, y...)   DLOGCOM( DL_WARN,   x, ##y)
#define DLOGI(x, y...)   DLOGCOM( DL_INFO,   x, ##y)
#define DLOGD(x, y...)   DLOGCOM( DL_DEBUG,  x, ##y)
#define DLOGV(x, y...)   DLOGCOM( DL_VERBOSE,x, ##y)
#endif


// =============================================================================
#define DEBUG_PRINT_Bin          DbgPrinStr
// #define DEBUG_PRINT_ASCIIBar     Dbg_ShowBarForChar
// #define DEBUG_PRINT_Ipv4Ntoa     Dbg_ipv4_ntoa
// #define DEBUG_PRINT_Ipv6Ntoa     Dbg_ipv6_ntoa
#define DEBUG_PRINT_Time            Dbg_Show_Now
// #define DEBUG_PRINT_DiffTime     Dbg_ShowDiffTime
// #define DEBUG_PRINT_HTime        Dbg_time_human


#ifdef DEBUG_PRINT_Bin
extern void DbgPrinStr(void *iStr,void *iPD,int iLen);
#endif
#ifdef DEBUG_PRINT_ASCIIBar
extern void Dbg_ShowBarForChar( short div, short _data );
#endif
#ifdef DEBUG_PRINT_Ipv4Ntoa
extern char* Dbg_ipv4_ntoa( unsigned long iIP );
#endif
#ifdef DEBUG_PRINT_Ipv6Ntoa
extern char* Dbg_ipv6_ntoa( char *iIP );
#endif
#ifdef DEBUG_PRINT_Time
extern void Dbg_Show_Now( void );
#endif
#ifdef DEBUG_PRINT_DiffTime
extern void Dbg_ShowDiffTime( char *, struct timeval *, struct timeval *);
#endif
#ifdef DEBUG_PRINT_HTime
extern char* Dbg_time_human( unsigned long long iTime, char *iBuf );
#endif

#endif
