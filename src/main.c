/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <string.h>
#include <math.h>
#include "include/world.h"
#include "include/rng.h"
#include "include/util.h"
#include "include/render.h"

#define max(a, b) ({             \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a > _b ? _a : _b;       \
})

#define min(a, b) ({             \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a < _b ? _a : _b;       \
})

struct args {
	unsigned int width;
	unsigned int height;
	unsigned int iters_per_frame;
	unsigned int max_generations;
	unsigned int food_to_generate;
	unsigned int iters_per_food_gen;
	unsigned int cells_per_world;
	unsigned int number_of_worlds;

	unsigned int starting_gen_number;
	unsigned int starting_world_number;
	char *dest_folder;

	/*  Bitflags controlling a few things
	*   bit 0 - rgba
	*   bit 1 - stop at extinction
	*   bit 2 - use ffmpeg
	*   bit 3 - if using ffmpeg, 1 for mp4, 0 for gif
	*/
	int flags;
};

generator_handle main_rng;

static inline void parse_long_args(struct args *args, int i, int argc, 
	const char *arg, const char **argv) 
{
	if(strcmp(arg, "--rgba") == 0)
		args->flags |= 1 << 0;
	else if(strcmp(arg, "--end-at-extinction") == 0)
		args->flags |= 1 << 1;
	else if(strcmp(arg, "--use-ffmpeg") == 0)
		args->flags |= 1 << 2;
	else if(strcmp(arg, "--mp4") == 0)
		args->flags |= 1 << 3;
	else if(strcmp(arg, "--gif") == 0)
		args->flags &= ~(1 << 3);
	else
		fprintf(stderr, "Error: \"%s\" is an invalid argument.", arg);

	/* TODO: FINISH */
}

static inline struct args parse_args(int argc, const char **argv) 
{
	struct args args = {
		/* 16:9 */
		.width = 400,
		.height = 225,
		
		/* 15 seconds of 24 fps video */
		.iters_per_frame = 10,
		.max_generations = 15*24*10,

		.cells_per_world = 2000,
		.number_of_worlds = 1,

		.starting_gen_number = 0,
		.starting_world_number = 0,

		.dest_folder = NULL,

		.flags = 0,
	};

	for(int i = 1; i < argc; i++) {
		if(argv[i] == NULL) {
			break;
		}

		if(strncmp(argv[i], "--", 2) == 0) {
			
		}
	}

	/* TODO: FINISH */

	die(args.dest_folder != NULL, "Bad arguments");
	return args;
}

static inline void simulation(struct args *args, int w) 
{
	struct world world;
	struct statistics highest; /* each of the highest values */
	struct statistics lowest; /* each of the lowest values */
	struct statistics average; /* more like totals */
	struct statistics current; /* essentially a temporary variable */
	int i;
	init_world(
		&world, args->width, args->height, 
		args->food_to_generate, args->cells_per_world
	);

	ZERO_STRUCT(highest);
	ZERO_STRUCT(lowest);
	ZERO_STRUCT(average);
	ZERO_STRUCT(current);

	puts("Starting simulation");
	for(i = 0; i < args->max_generations; i++) {
		if(i % args->iters_per_frame) {
			char *file = malloc(snprintf(NULL, 0, "%s/%i-%i.png", 
				args->dest_folder, 
				args->starting_world_number+w,
				args->starting_gen_number+i)
			);
			char *format[13];
			struct render_settings rs = render_defaults();
			
			switch(args->dest_folder[strlen(args->dest_folder)-2]){
				case '/':
				case '\\':
				strcpy(format, "%s%i-%i.png");
				break;

				default:
				strcpy(format, "%s/%i-%i.png");
				break;
			}

			sprintf(file, format, args->dest_folder, 
				args->starting_world_number+w,
				args->starting_gen_number+i
			);

			rs.write_to_file = true;
			render(&world, file, rs);
		}
		
		step_world(&world, &current);

		/* statistics */
		highest.births   = max(current.births,  highest.births);
		highest.death    = max(current.death,   highest.death);
		highest.food     = max(current.food,    highest.food);
		highest.murder   = max(current.murder,  highest.murder);
		highest.old_age  = max(current.old_age, highest.old_age);
		highest.pop      = max(current.pop,     highest.pop);
		highest.starve   = max(current.starve,  highest.starve);
		highest.suicide  = max(current.suicide, highest.suicide);

		lowest.births    = min(current.births,  lowest.births);
		lowest.death     = min(current.death,   lowest.death);
		lowest.food      = min(current.food,    lowest.food);
		lowest.murder    = min(current.murder,  lowest.murder);
		lowest.old_age   = min(current.old_age, lowest.old_age);
		lowest.pop       = min(current.pop,     lowest.pop);
		lowest.starve    = min(current.starve,  lowest.starve);
		lowest.suicide   = min(current.suicide, lowest.suicide);

		average.births  += current.births;
		average.death   += current.death;
		average.food    += current.food;
		average.murder  += current.murder;
		average.old_age += current.old_age;
		average.pop     += current.pop;
		average.starve  += current.starve;
		average.suicide += current.suicide;

		ZERO_STRUCT(current);

		if(world_has_life(&world) && ((args->flags >> 1) & 1))
			break;
	}

	puts("Simulation finished. Printing statistics.");
	/* TODO: Statistics */
}

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wchar.h>

static inline wchar_t *convert_to_utf16(const char *utf8) 
{
	int wchar_count = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* utf16;
	assert(wchar_count != 0);


	utf16 = malloc(wchar_count * sizeof(wchar_t));
	assert(utf16 != NULL);

	wchar_count = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, wchar_count);
	assert(wchar_count != 0);

	return utf16;
}

static inline void make_dir(const char *path) 
{
	/* Stupid Windows and it's utf16le */
	wchar_t *tmp = convert_to_utf16(path);

	/* in what world is success non-zero */
	die(CreateDirectoryW(tmp, NULL) == 0, "Couldn't make a directory");
	free(tmp);
}

static inline bool dir_exists(const char *path) 
{
	DWORD ftyp = GetFileAttributesA(path);

	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;

        return false;
}

/* Windows is annoying. */
int wmain(int argc, wchar_t **wargv)
#else

#include <sys/stat.h>
#include <sys/types.h>

static inline void make_dir(const char *path) 
{
	die(mkdir(path, 0777) != 0, "Couldn't make a directory");
}

static inline bool dir_exists(const char *path) 
{
	struct stat sb;
	if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
		return true;
	else
		return false;
}

int main(int argc, char **argv)
#endif
{
	struct args args;
	#ifdef _WIN32
	/* Convert our arguments to utf8 like any sane person */
	
	char **argv = calloc(argc, sizeof(char*));

	for(int i = 0; i < argc - 1; i++) {
		int size = WideCharToMultiByte(
			CP_UTF8, 0, wargv[i], -1, 
			NULL,0, NULL, NULL
		);

		die(size != 0, "Failed to read arguments.");

    		argv[i] = (char*)malloc(size);
    		die(argv[i] != NULL, "Failed to read arguments.");

		die(WideCharToMultiByte(
			CP_UTF8, 0, wargv[i], -1,
			argv[i], size, NULL, NULL
		) == 0, "Failed to read arguments.");
	}
	#endif

	args = parse_args(argc, argv);

	/* Validate args */
	args.width = args.width > 4 ? args.width : 400;
	args.height = args.height > 4 ? args.height : 225;
	args.max_generations = args.max_generations > 1? 
		args.max_generations : 100;
	args.number_of_worlds = args.number_of_worlds == 0? 
		1 : args.number_of_worlds;

	if(dir_exists(args.dest_folder) == false)
		make_dir(args.dest_folder);

	main_rng = new_generator();

	puts("Starting simulations.");

	for(int i = 0; i < args.number_of_worlds; i++) {
		printf("World %d of %d.\n", i+1, args.number_of_worlds);
		simulation(&args, i);
		
		/* Five newlines per world */
		for(int j = 0; j < 5; j++) putchar('\n');
	}
}