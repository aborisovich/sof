/*
 * mbox dump to debug log convertor.
 *
 * Copyright (c) 2015, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "convert.h"

#ifdef LOGGER_FORMAT
#define APP_NAME "sof-logger"
#else
#define APP_NAME "sof-rmbox"
#endif

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

static const char *debugfs[] = {
	"dmac0", "dmac1", "ssp0", "ssp1",
	"ssp2", "iram", "dram", "shim",
	"mbox", "etrace",
};

static void usage(void)
{
	fprintf(stdout, "Usage %s <option(s)> <file(s)>\n", APP_NAME);
	fprintf(stdout, "%s:\t \t\t\tDisplay mailbox contents\n", APP_NAME);
	fprintf(stdout, "%s:\t -i infile -o outfile\tDump infile contents to outfile\n", APP_NAME);
#ifdef LOGGER_FORMAT
	fprintf(stdout, "%s:\t -l *.ldc_file\t\t*.ldc files generated by rimage\n", APP_NAME);
	fprintf(stdout, "%s:\t -p \t\t\tInput from stdin\n", APP_NAME);
	fprintf(stdout, "%s:\t -e \t\t\tEnable checking firmware version with default verification file\n", APP_NAME);
	fprintf(stdout, "%s:\t -v ver_file\t\tEnable checking firmware version with ver_file file\n", APP_NAME);
#endif
	fprintf(stdout, "%s:\t -c\t\t\tSet timestamp clock in MHz\n", APP_NAME);
	fprintf(stdout, "%s:\t -s\t\t\tTake a snapshot of state\n", APP_NAME);
	fprintf(stdout, "%s:\t -t\t\t\tDisplay trace data\n", APP_NAME);
	exit(0);
}

static int snapshot(const char *name)
{
	const char *path = "/sys/kernel/debug/sof";
	uint32_t val, addr;
	char pinname[64], poutname[64], buffer[128];
	FILE *in_fd, *out_fd;
	int i, count;

	if (name == NULL) {
		fprintf(stderr, "error: need snapshot name\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(debugfs); i++) {

		sprintf(pinname, "%s/%s", path, debugfs[i]);
		sprintf(poutname, "%s.%s.txt", name, debugfs[i]);

		/* open debugfs for reading */
		in_fd = fopen(pinname, "r");
		if (in_fd == NULL) {
			fprintf(stderr, "error: unable to open %s for reading %d\n",
				pinname, errno);
			continue;
		}

		/* open outfile for reading */
		out_fd = fopen(poutname, "w");
		if (out_fd == NULL) {
			fprintf(stderr, "error: unable to open %s for writing %d\n",
				poutname, errno);
			fclose(in_fd);
			continue;
		}

		fprintf(stdout, "processing %s...\n", pinname);
		addr = 0;

		while (1) {
			count = fread(&val, 1, 4, in_fd);
			if (count != 4)
				break;

			sprintf(buffer, "0x%6.6x: 0x%8.8x\n", addr, val);

			count = fwrite(buffer, 1, strlen(buffer), out_fd);

			addr += 4;
		}

		fclose(in_fd);
		fclose(out_fd);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct convert_config config;
	int opt, count, ret = 0;
	FILE *in_fd = NULL, *out_fd = NULL;
	char c, tmp[8] = {0};
	uint64_t addr = 0, val, timestamp = 0, align = 4, i;

	config.trace = 0;
	config.clock = 19.2;
	config.in_file = NULL;
	config.out_file = NULL;
	config.out_fd = NULL;
	config.in_fd = NULL;
#ifdef LOGGER_FORMAT
	config.ldc_file = NULL;
	config.ldc_fd = NULL;
	config.input_std = 0;
	/* checking fw version is disabled by default */
	config.version_file = NULL;
	config.version_fd = NULL;
	config.version_fw = 0;
#endif

#ifdef LOGGER_FORMAT
	while ((opt = getopt(argc, argv, "ho:i:l:ps:m:c:tev:")) != -1) {
#else
	while ((opt = getopt(argc, argv, "ho:i:s:m:c:t")) != -1) {
#endif
		switch (opt) {
		case 'o':
			config.out_file = optarg;
			break;
		case 'i':
			config.in_file = optarg;
			break;
		case 't':
			config.trace = 1;
			break;
		case 'c':
			config.clock = atof(optarg);
			break;
		case 's':
			return snapshot(optarg);
#ifdef LOGGER_FORMAT
		case 'l':
			config.ldc_file = optarg;
			break;
		case 'p':
			config.input_std = 1;
			break;
		case 'e':
			/* enabling checking fw version with default verification 
			 * file
			 */
			config.version_fw = 1;
			config.version_file = "/sys/kernel/debug/sof/fw_version";
			break;
		case 'v':
			/* enabling checking fw version with ver_file file */
			config.version_fw = 1;
			config.version_file = optarg;
			break;
#endif
		case 'h':
		default: /* '?' */
			usage();
		}
	}

#ifdef LOGGER_FORMAT
	
	if (!config.ldc_file) {
		fprintf(stderr, "error: Missing ldc file\n");
		usage();
	}

	config.ldc_fd = fopen(config.ldc_file, "r");
	if (!config.ldc_fd) {
		fprintf(stderr, "error: Unable to open ldc file %s\n",
			config.ldc_file);
		ret = -errno;
		goto out;
	}

	if (config.version_fw) {
		config.version_fd = fopen(config.version_file, "r");
		if (!config.version_fd) {
			fprintf(stderr, "error: Unable to open ver file %s\n",
				config.version_file);
			ret = -errno;
			goto out;
		}
	}
#endif

	if (config.out_file) {
		config.out_fd = fopen(config.out_file, "w");
		if (!config.out_fd) {
			fprintf(stderr, "error: Unable to open out file %s\n",
				config.out_file);
			ret = -errno;
			goto out;
		}
	} else {
		config.out_fd = stdout;
	}

	/* trace requested ? */
	if (config.trace)
		config.in_file = "/sys/kernel/debug/sof/trace";

	/* default option with no infile is to dump errors/debug data */
	if (!config.in_file)
		config.in_file = "/sys/kernel/debug/sof/etrace";

#ifdef LOGGER_FORMAT
	if (config.input_std) {
		config.in_fd = stdin;
	} else
#endif
	{
		config.in_fd = fopen(config.in_file, "r");
		if (!config.in_fd) {
			fprintf(stderr, "error: Unable to open in file %s\n",
				config.in_file);
			ret = -errno;
			goto out;
		}
	}

	convert(&config);

out:
	/* close files */
	if (config.out_fd)
		fclose(config.out_fd);

	if (config.in_fd)
		fclose(config.in_fd);

#ifdef LOGGER_FORMAT
	if (config.ldc_fd)
		fclose(config.ldc_fd);
	if (config.version_fd)
		fclose(config.version_fd);
#endif

	return ret;
}
