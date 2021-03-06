#pragma once

#include <stdio.h>
#include <string>

#define panic(x...) do \
{ \
	fprintf(stderr, x); \
	exit(1); \
} while(0)

#define panic_if(cond, x...) \
		do { if ((cond)) panic(x); } while(0)


std::string fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

unsigned int get_number_of_cores(void);

bool getIntFromString(std::string str, unsigned int offset, unsigned int *out);

bool getDoubleFromString(std::string str, unsigned int offset, double *out);

int write_file(const void *data, size_t len, const char *fmt, ...);
