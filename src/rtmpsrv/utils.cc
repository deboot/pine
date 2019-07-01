#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h>

/*
 * Used to do unaligned loads on archs that don't support them. GCC can mostly
 * optimize these away.
 */
uint32_t load_be32(const void *p)
{
	uint32_t val;
	memcpy(&val, p, sizeof val);
	return ntohl(val);
}

uint16_t load_be16(const void *p)
{
	uint16_t val;
	memcpy(&val, p, sizeof val);
	return ntohs(val);
}

uint32_t load_le32(const void *p)
{
	const uint8_t *data = (const uint8_t *) p;
	return data[0] | ((uint32_t) data[1] << 8) |
		((uint32_t) data[2] << 16) | ((uint32_t) data[3] << 24);
}

uint32_t load_be24(const void *p)
{
	const uint8_t *data = (const uint8_t *) p;
	return data[2] | ((uint32_t) data[1] << 8) | ((uint32_t) data[0] << 16);
}

void set_be24(void *p, uint32_t val)
{
	uint8_t *data = (uint8_t *) p;
	data[0] = val >> 16;
	data[1] = val >> 8;
	data[2] = val;
}

void set_le32(void *p, uint32_t val)
{
	uint8_t *data = (uint8_t *) p;
	data[0] = val;
	data[1] = val >> 8;
	data[2] = val >> 16;
	data[3] = val >> 24;
}

const std::string strf(const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	char *buf = NULL;
	vasprintf(&buf, fmt, vl);
	va_end(vl);
	std::string s(buf);
	free(buf);
	return s;
}
