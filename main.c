/*
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#define NCHUNKS_DEF 64

int verbose = VERBOSE;

struct chunk {
	void *ptr;
	unsigned size;
};

unsigned nchunks = NCHUNKS_DEF;
struct chunk *chks;
unsigned nalloc = 0;
int ovrlp_idx = -1;

#ifdef call_init
extern int call_init(void);
#endif

extern void *call_alloc(size_t size);

#if FREE_NEEDS_SIZE == 1
extern void call_free(void *p, int size);
#else
extern void call_free(void *p);
#endif

void print_chunks()
{
	int i;
	struct chunk *c;
	const char *mark = "";
	printf("Chunks list: %d chunks\n", nalloc);
	for (i = 0; i < nchunks; i++) {
		c  = &chks[i];
		if (c->ptr) {
			if (i == ovrlp_idx)
				mark = " <--";
			printf("%p - %p (%4d bytes)%s\n",
				c->ptr,	c->ptr + c->size, c->size, mark);
		}
	}
}

int exec_init(void)
{
#ifdef call_init
	return call_init();
#else
	return 0;
#endif
}

int exec_alloc(void)
{
	void *p;
	int size;
	int i;
	struct chunk *newchks;

	ovrlp_idx = -1;
	size = (rand() % MAX_ALLOC_SIZE) + 1;

	if (nchunks == nalloc) {
		newchks = realloc(chks, 2 * nchunks * sizeof(chks[0]));
		if (!newchks) {
			fprintf(stderr, "Fatal: failed to realloc %d chunks\n",
				nchunks);
			return -ENOMEM;
		}
		memset(&newchks[nchunks], 0, nchunks * sizeof(chks[0]));
		chks = newchks;
		nchunks *= 2;
	}

	p = call_alloc(size);
	if (!p) {
		fprintf(stderr, "Warning: alloc %d failed\n", size);
		return 0;
	}

	for (i = 0; i < nchunks; i++) {
		if (chks[i].ptr &&
			p >= chks[i].ptr && p < chks[i].ptr + chks[i].size) {
			printf("%p overlaps\n", p);
			ovrlp_idx = i;
			return -EINVAL;
		}

		if (chks[i].ptr &&
			p + size - 1 >= chks[i].ptr &&
			p + size - 1 < chks[i].ptr + chks[i].size) {
			printf("%p + 0x%x overlaps\n", p, size);
			ovrlp_idx = i;
			return -EINVAL;
		}
	}

	for (i = 0; chks[i].ptr; i++); /* One empty chunk is surely available */
	chks[i].ptr = p;
	chks[i].size = size;
	nalloc++;
	return 0;
}

int exec_free(void)
{
	void *p;
	int i, n;

	if (!nalloc) {
		fprintf(stderr, "Warning: nothing to free\n");
		return 0;
	}

	n = rand() % nalloc;

	for (i = 0; i < nchunks; i++)
		if (chks[i].ptr) {
			if (!n)
				break;
			n--;
		}

#if FREE_NEEDS_SIZE == 1
	call_free(chks[i].ptr, chks[i].size);
#else
	call_free(chks[i].ptr);
#endif
	chks[i].ptr = NULL;
	chks[i].size = 0;
	nalloc--;
	return 0;
}

int main(int argc, char *argv[])
{
	char ln[16];
	int ret;

	srand(time(NULL));
	chks = calloc(nchunks, sizeof(struct chunk));

	if (!chks)
		return -ENOMEM;

	if (exec_init()) {
		fprintf(stderr, "Allocator init failed\n");
		ret = 3;
		goto err;
	}

	while (fgets(ln, sizeof(ln), stdin)) {
		unsigned ncycles;
		int (*fptr)(void);

		printf("Executing: %s", ln);
		if (strncmp(ln, "alloc ", 6) == 0) {
			fptr = exec_alloc;
			ncycles = atoi(&ln[6]);
		} else if (strncmp(ln, "free ", 5) == 0) {
			fptr = exec_free;
			if (strncmp(&ln[5], "all", 3) == 0)
				ncycles = nalloc;
			else
				ncycles = atoi(&ln[5]);
			if (ncycles > nalloc) {
				printf("Invalid free number: %d (max %d)\n",
					ncycles, nalloc);
				continue;
			}
		} else if (strncmp(ln, "status", 6) == 0) {
			print_chunks();
			continue;
		} else {
			printf("Type your commands:\n"
				"\talloc <N>: call alloc N times\n"
				"\tfree <N>|all: call free N times | free all\n"
				"\tstatus: print chunks status\n"
				"\t^D: end\n");
			continue;
		}

		while (ncycles--) {
			if (fptr()) {
				print_chunks();
				printf("Test failed\n");
				ret = 255;
				goto err;
			}
		}

		if (verbose)
			print_chunks();
	}

	free(chks);

	printf("Test ended successfully\n");

	return 0;

err:
	free(chks);
	return ret;
}

