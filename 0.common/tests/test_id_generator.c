/*
 * Copyright (c) 2020 YuQing <384681@qq.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the Lesser GNU General Public License, version 3
 * or later ("LGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "logger.h"
#include "shared_func.h"
#include "sched_thread.h"
#include "ini_file_reader.h"
#include "sockopt.h"
#include "id_generator.h"

int main(int argc, char *argv[])
{
	struct idg_context context;
	int result;
	int i;
	int64_t id;
	const int machine_id = 0;
	const int mid_bits = 8;
	const int extra_bits = 10;
	const int sn_bits = 14;
	
	log_init();
	g_log_context.log_level = LOG_DEBUG;

	//result = id_generator_init(&context, "/tmp/sn.txt");
	result = id_generator_init_extra(&context, "/tmp/sn.txt",
		machine_id, mid_bits, extra_bits, sn_bits);
	if (result != 0)
	{
		return result;
	}

	for (i=0; i<1024; i++)
	{
		result = id_generator_next_extra(&context, i, &id);
		if (result != 0)
		{
			break;
		}
		printf("id: %"PRId64", extra: %d\n", id, id_generator_get_extra(&context, id));
	}

	id_generator_destroy(&context);
	return 0;
}
