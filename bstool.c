#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsdiff.h"
#include "bspatch.h"

void *read_file(FILE *f, int64_t *size)
{
	void *out;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	rewind(f);

	out = malloc(*size);
	if (out == NULL)
		goto out;

	fread(out, 1, *size, f);

out:
	return out;
}

int diff_write(struct bsdiff_stream *stream, const void *buffer, int len)
{
	FILE *out_f = stream->opaque;
	fwrite(buffer, 1, len, out_f);
	return 0;
}

int diff(const char *old_file, const char *new_file, const char *out_file)
{
	int ret = -1;
	FILE *old_f = NULL, *new_f = NULL, *out_f = NULL;
	void *old_dat, *new_dat;
	int64_t old_sz, new_sz;
	struct bsdiff_stream ctx;

	old_f = fopen(old_file, "rb");
	if (old_f == NULL)
	{
		fprintf(stderr, "Unable to open old file: %s\n", strerror(errno));
		goto out;
	}

	new_f = fopen(new_file, "rb");
	if (new_f == NULL)
	{
		fprintf(stderr, "Unable to open new file: %s\n", strerror(errno));
		goto out;
	}

	out_f = fopen(out_file, "wb");
	if (out_f == NULL)
	{
		fprintf(stderr, "Unable to open output file: %s\n", strerror(errno));
		goto out;
	}

	ctx.opaque = out_f;
	ctx.malloc = malloc;
	ctx.free = free;
	ctx.write = diff_write;

	old_dat = read_file(old_f, &old_sz);
	if (old_dat == NULL)
	{
		fprintf(stderr, "Unable to read old file: %s\n", strerror(errno));
		goto out;
	}

	new_dat = read_file(new_f, &new_sz);
	if (new_dat == NULL)
	{
		fprintf(stderr, "Unable to read new file: %s\n", strerror(errno));
		goto out;
	}

	bsdiff(old_dat, old_sz, new_dat, new_sz, &ctx);

	ret = 0;

out:
	if(old_f != NULL)
		fclose(old_f);

	if(new_f != NULL)
		fclose(new_f);

	if(out_f != NULL)
		fclose(out_f);

	return ret;
}

int patch_read(const struct bspatch_stream *stream, void *buffer, int len)
{
	FILE *patch_f = stream->opaque;
	fread(buffer, 1, len, patch_f);
	return 0;
}

int patch(const char *patch_file, const char *old_file, const char *out_file)
{
	int ret = -1;
	FILE *old_f = NULL, *patch_f = NULL, *out_f = NULL;
	void *old_dat, *new_dat;
	int64_t old_sz, new_sz;
	struct bspatch_stream ctx;

	patch_f = fopen(patch_file, "rb");
	if (patch_f == NULL)
	{
		fprintf(stderr, "Unable to open new file: %s\n", strerror(errno));
		goto out;
	}

	old_f = fopen(old_file, "rb");
	if (old_f == NULL)
	{
		fprintf(stderr, "Unable to open old file: %s\n", strerror(errno));
		goto out;
	}

	out_f = fopen(out_file, "wb");
	if (out_f == NULL)
	{
		fprintf(stderr, "Unable to open output file: %s\n", strerror(errno));
		goto out;
	}

	ctx.opaque = patch_f;
	ctx.read = patch_read;

	old_dat = read_file(old_f, &old_sz);
	if (old_dat == NULL)
	{
		fprintf(stderr, "Unable to read old file: %s\n", strerror(errno));
		goto out;
	}

	new_sz = 0;
	fread(&new_sz, 1, sizeof(int64_t), patch_f);
	fprintf(stdout, "Output file size will be %lld\n", new_sz);
	rewind(patch_f);
	new_dat = malloc(new_sz);
	if (new_dat == NULL)
	{
		fprintf(stderr, "Unable to allocate new file: %s\n", strerror(errno));
		goto out;
	}

	ret = bspatch(old_dat, old_sz, new_dat, new_sz, &ctx);
	if (ret != 0)
	{
		fprintf(stderr, "Unable to patch file: %d\n", ret);
	}

	fwrite(new_dat, 1, new_sz, out_f);

	ret = 0;

out:
	if (old_f != NULL)
		fclose(old_f);

	if (patch_f != NULL)
		fclose(patch_f);

	if (out_f != NULL)
		fclose(out_f);

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;

	if (argc != 5)
	{
		fputs("Usage:\n"
			" bstool --diff old_file new_file output_file\n"
			" bstool --patch patch_file old_file output_file\n", stderr);
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "--diff") == 0)
	{
		if (diff(argv[2], argv[3], argv[4]) != 0)
			goto out;
	}
	else if (strcmp(argv[1], "--patch") == 0)
	{
		if (patch(argv[2], argv[3], argv[4]) != 0)
			goto out;
	}
	else
	{
		fputs("Invalid mode.\n", stderr);
		goto err;
	}

	puts("Successful");
	ret = EXIT_SUCCESS;

out:
	return ret;

err:
	fputs("\nUsage:\n"
		" bstool --diff old_file new_file output_file\n"
		" bstool --patch patch_file old_file output_file]\n", stderr);
	goto out;
}