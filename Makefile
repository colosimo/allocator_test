ALLOCATOR_OBJ ?=
FUNC_INIT ?=
FUNC_ALLOC ?= malloc
FUNC_FREE ?= free
FUNC_PRINTF ?=
FREE_NEEDS_SIZE ?= 0
MAX_ALLOC_SIZE ?= 1024
VERBOSE ?= 0

EXE = allocator_test

obj-y := $(ALLOCATOR_OBJ) main.o

CFLAGS += -Dcall_alloc=$(FUNC_ALLOC) \
          -Dcall_free=$(FUNC_FREE) \
	  -DFREE_NEEDS_SIZE=$(FREE_NEEDS_SIZE) \
	  -DVERBOSE=$(VERBOSE) \
	  -DMAX_ALLOC_SIZE=$(MAX_ALLOC_SIZE)

ifneq ($(FUNC_INIT),)
	CFLAGS += -Dcall_init=$(FUNC_INIT)
endif

ifneq ($(FUNC_PRINTF),)
	CFLAGS += -Dcall_printf=$(FUNC_PRINTF)
endif

all: $(EXE)

$(EXE): $(obj-y)
	$(CC) $(obj-y) -o $(EXE)

clean:
	rm -f $(EXE) *.o *~

