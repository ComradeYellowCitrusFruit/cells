/*  SPDX-License-Identifier: GPL-3.0-only
*   Cellular life simulation following strict rules
*   Copyright (C) 2023 Teresa Maria Rivera
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "include/world.h"
#include "include/render.h"

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

static void parse_long_args(struct args *args, int i, int argc, 
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

static struct args parse_args(int argc, const char **argv) {
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

	assert(args.dest_folder != NULL);
	return args;
}

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>

static wchar_t *convert_to_utf16(const char *utf8) {
	int wchar_count = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* utf16;
	assert(wchar_count != 0);


	utf16 = malloc(wchar_count * sizeof(wchar_t));
	assert(utf16 != NULL);

	wchar_count = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, wchar_count);
	assert(wchar_count != 0);

	return utf16;
}

static void make_dir(const char *path) {
	/* Stupid Windows and it's utf16le */
	wchar_t *tmp = convert_to_utf16(path);

	/* in what world is success non-zero */
	assert(CreateDirectory(tmp, NULL) != 0);
	free(tmp);
}

static bool dir_exists(const char *path) {
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

static void make_dir(const char *path) {
	assert(mkdir(path, 0777) != 0);
}

static bool dir_exists(const char *path) {
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

		assert(size != 0);

    		argv[i] = (char*)malloc(size);
    		assert(argv[i] != NULL);

		assert(WideCharToMultiByte(
			CP_UTF8, 0, wargv[i], -1,
			argv[i], size, NULL, NULL
		) == 0);
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

	struct world world;
	init_world(
		&world, args.width, args.height, 
		args.food_to_generate, args.cells_per_world
	);

	for(int i = 0; i < args.max_generations; i++) {
		/* TODO: render and food gen. */
		/* TODO: statistics */
		step_world(&world);

		if(world_has_life(&world) && (args.flags >> 1) & 1)
			break;
	}

	puts("Simulation finished. Printing statistics.");
}