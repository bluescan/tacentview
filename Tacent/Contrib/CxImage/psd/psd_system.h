#ifndef __PSD_SYSTEM_H__
#define __PSD_SYSTEM_H__

#include <string.h>
#include <assert.h>
#include "psd_types.h"

#ifdef __cplusplus
extern "C" {
#endif


#if 1
#define psd_assert(x)			assert(x)
#else
#define psd_assert(x)			do {} while(0)
// or
// #define psd_assert(x)		return psd_status_unkown_error
#endif


void * psd_malloc(psd_int size);
void * psd_realloc(void * block, psd_int size);

void psd_free(void * block);
void psd_freeif(void * block);

void * psd_fopen(psd_char * file_name);
void psd_fclose(void * file);

//psd_int psd_fsize(void * file);
//psd_int psd_fread(psd_uchar * buffer, psd_int count, void * file);
//psd_int psd_fseek(void * file, psd_int length);

psd_int   psd_std_size(void *obj);
psd_int   psd_std_seek(void *obj, int offset, int origin);
psd_int   psd_std_read(void *obj, void *buf, int size, int cnt);
//psd_int   psd_std_write(void *obj, void *buf, int size, int cnt);
//psd_int   psd_std_close(void *obj);
//psd_char* psd_std_gets(void *obj, char *string, int n);
//psd_int   psd_std_eof(void *obj);
//psd_int   psd_std_tell(void *obj);
//psd_int   psd_std_getc(void *obj);
//psd_int   psd_std_scanf(void *obj,const char *format, void* output);


#ifdef __cplusplus
}
#endif

#endif
