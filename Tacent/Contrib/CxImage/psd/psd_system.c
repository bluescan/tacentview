// created: 2007-01-27
#include <stdio.h>
#include <stdlib.h>
#include "libpsd.h"
#include "psd_system.h"

#pragma warning(disable:4267)

void * psd_malloc(psd_int size)
{
	return malloc(size);
}

void * psd_realloc(void * block, psd_int size)
{
	return realloc(block, size);
}

void psd_free(void * block)
{
	free(block);
}

void psd_freeif(void * block)
{
	if (block != NULL)
		psd_free(block);
}

void * psd_fopen(psd_char * file_name)
{
	return (void *)fopen(file_name, "rb");
}

void psd_fclose(void * file)
{
	fclose((FILE *)file);
}

psd_int psd_std_size(void *obj)
{
	psd_int offset, size;

	offset = ftell((FILE *)obj);
	fseek((FILE *)obj, 0, SEEK_END);
	size = ftell(obj);
	fseek((FILE *)obj, 0, SEEK_SET);
	fseek((FILE *)obj, offset, SEEK_CUR);

	return size;
}

psd_int psd_std_seek(void *obj, int offset, int origin)
{
	return fseek((FILE *)obj, offset, origin/*SEEK_CUR*/);
}

psd_int psd_std_read(void *obj, void *buf, int size, int cnt)
{
	return fread(buf, size/*1*/, cnt, (FILE *)obj);
}

