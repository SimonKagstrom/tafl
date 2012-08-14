#include <utils.hh>

#include <stdarg.h>
#include <thread>

std::string fmt(const char *fmt, ...)
{
	char buf[4096];
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	panic_if(res >= (int)sizeof(buf),
			"Buffer overflow");

	return std::string(buf);
}

unsigned int get_number_of_cores(void)
{
	unsigned int out = std::thread::hardware_concurrency();

	if (out == 0)
		out = 1;

	return out;
}
