#include "../core/core.h"
#include "exmap.h"

extern "C" EXPORT const char* init_exmap(int n_args, const char** args)
{
	if (!Core::initialize())
	{
		return "Extools Init Failed";
	}
	return enable_exmap();
}