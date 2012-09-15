#include <utils.hh>

#include <stdarg.h>
#include <string.h>
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

bool getIntFromString(std::string str, unsigned int offset, unsigned int *out)
{
	std::string in = str.substr(offset, 2);

	char *endp;
	const char *inc = in.c_str();

	*out = strtoul(inc, &endp, 16);
	if (endp == inc || (endp && *endp != '\0'))
		return false;

	return true;
}

bool getDoubleFromString(std::string str, unsigned int offset, double *out)
{
	std::string in = str.substr(offset, 16);

	char *endp;
	const char *inc = in.c_str();
	union
	{
		unsigned long long ull;
		double dbl;
	} u;

	u.ull = strtoull(inc, &endp, 16);
	if (endp == inc || (endp && *endp != '\0'))
		return false;

	*out = u.dbl;

	return true;
}
