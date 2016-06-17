#include "common.h"

int calc_size(int attr_type)
{
	if (attr_type == -1 || attr_type == -2)
		return 4;
	else
		return attr_type;
}
