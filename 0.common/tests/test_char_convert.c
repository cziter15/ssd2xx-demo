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
#include "char_convert_loader.h"
#include "char_converter.h"

int main(int argc, char *argv[])
{
	int result;
	int count;
	int input_len;
	int out_len;
	char input[8 * 1024];
	char output[10 * 1024];
	FastCharConverter converter;

	if (argc >= 2) {
		input_len = snprintf(input, sizeof(input), "%s", argv[1]);
	} else {
		input_len = read(0, input, sizeof(input) - 1);
		if (input_len < 0) {
			fprintf(stderr, "read from stdin fail");
			return errno;
		}
		*(input + input_len) = '\0';
	}

	log_init();
	printf("input_len: %d\n%s\n\n", (int)strlen(input), input);
    
	result = std_spaces_add_backslash_converter_init(&converter);
	printf("result1: %d\n", result);
	if (result != 0) {
		return result;
	}
	char_converter_set_pair_ex(&converter, ' ',
		FAST_CHAR_OP_ADD_BACKSLASH, '\'');
	
	count = fast_char_convert(&converter, input, input_len,
		output, &out_len, sizeof(output));
	printf("count: %d\n", count);
	printf("out_len: %d\n%.*s\n", out_len, out_len, output);
	return 0;
}

