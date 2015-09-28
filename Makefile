CONFIG:=default.conf

-include $(CONFIG)

EXE = allocator_test

obj-y := $(ALLOCATOR_OBJ) main.o

PRE_BUILD_SCRIPT ?= true

CFLAGS += -Dcall_alloc=$(FUNC_ALLOC) \
          -Dcall_free=$(FUNC_FREE) \
	  -DFREE_NEEDS_SIZE=$(FREE_NEEDS_SIZE) \
	  -DVERBOSE=$(VERBOSE) \
	  -DMAX_ALLOC_SIZE=$(MAX_ALLOC_SIZE)

ifneq ($(FUNC_INIT),)
	CFLAGS += -Dcall_init=$(FUNC_INIT)
endif

all: run_pre $(EXE)

run_pre:
	if [ -x $(PRE_BUILD_SCRIPT) ] ; then \
		$(PRE_BUILD_SCRIPT) $(PRE_BUILD_SCRIPT_ARGS) ; \
	fi

$(EXE): $(obj-y)
	$(CC) $(obj-y) -o $(EXE) $(CFLAGS_EXTRA)

clean:
	rm -f $(EXE) *.o *~


.PHONY: run_pre

