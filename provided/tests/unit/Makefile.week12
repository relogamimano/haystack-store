# ======================================================================
# Useful for feedback

CC = clang

TARGETS := imgfsstruct imgfstools imgfslist
TARGETS += imgfscreate imgfsdelete
TARGETS += imgfsdedup imgfscontent
TARGETS += imgfsresolutions imgfsinsert imgfsread
TARGETS += http

CFLAGS += -g

CPPFLAGS += -fsanitize=address
LDFLAGS  += -fsanitize=address
LDLIBS   += -fsanitize=address

CFLAGS	 += $(shell pkg-config --cflags vips)
LDLIBS	 += $(shell pkg-config --libs vips)

EXECS=$(foreach name,$(TARGETS),unit-test-$(name))

.PHONY: unit-tests all $(TARGETS) execs

## removed undesired feature
## # first creates all exec, **then** launch them
## all: execs unit-tests
all: unit-tests

unit-tests: $(TARGETS)

execs: $(EXECS)

# some target shortcuts : compile & run the tests
imgfsstruct: unit-test-imgfsstruct
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
imgfstools: unit-test-imgfstools
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
imgfslist: unit-test-imgfslist
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
imgfscreate: unit-test-imgfscreate
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
imgfsdelete: unit-test-imgfsdelete
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
imgfsdedup: unit-test-imgfsdedup
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
imgfscontent: unit-test-imgfscontent
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
imgfsresolutions: unit-test-imgfsresolutions
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
imgfsinsert: unit-test-imgfsinsert
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
imgfsread: unit-test-imgfsread
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# some target shortcuts : compile & run the tests
http: unit-test-http
	./$^ && echo "==== " $< " SUCCEEDED =====" || { echo "==== " $< " FAILED ====="; false; }
	@printf '\n'

# ======================================================================
DATA_DIR ?= ../data/
SRC_DIR  ?= ../../done
CFLAGS  += '-I$(SRC_DIR)' -DCS202_TEST -DDATA_DIR='"$(DATA_DIR)"'
LDFLAGS += '-L$(SRC_DIR)'

LDLIBS += -lcheck -lm -lrt -pthread -lsubunit -lcrypto

OBJS = $(SRC_DIR)/imgfs_list.o $(SRC_DIR)/imgfs_tools.o $(SRC_DIR)/imgfscmd_functions.o
OBJS += $(SRC_DIR)/util.o $(SRC_DIR)/error.o

OBJS += $(SRC_DIR)/imgfs_create.o $(SRC_DIR)/imgfs_delete.o

OBJS += $(SRC_DIR)/image_dedup.o $(SRC_DIR)/image_content.o

OBJS += $(SRC_DIR)/imgfs_insert.o $(SRC_DIR)/imgfs_read.o

OBJS += $(SRC_DIR)/http_prot.o

# ======================================================================
unit-test-imgfsstruct.o: unit-test-imgfsstruct.c $(SRC_DIR)/imgfs.h

# ======================================================================
unit-test-imgfstools.o: unit-test-imgfstools.c $(SRC_DIR)/imgfs.h
unit-test-imgfstools: unit-test-imgfstools.o $(SRC_DIR)/imgfs_tools.o $(SRC_DIR)/error.o

# ======================================================================
unit-test-imgfslist.o: unit-test-imgfslist.c $(SRC_DIR)/imgfs.h
unit-test-imgfslist: unit-test-imgfslist.o $(OBJS)

# ======================================================================
unit-test-imgfscreate.o: unit-test-imgfscreate.c $(SRC_DIR)/imgfs.h
unit-test-imgfscreate: unit-test-imgfscreate.o $(OBJS)

# ======================================================================
unit-test-imgfsdelete.o: unit-test-imgfsdelete.c $(SRC_DIR)/imgfs.h
unit-test-imgfsdelete: unit-test-imgfsdelete.o $(OBJS)

# ======================================================================
unit-test-imgfsdedup.o: unit-test-imgfsdedup.c $(SRC_DIR)/imgfs.h
unit-test-imgfsdedup: unit-test-imgfsdedup.o $(OBJS)

# ======================================================================
unit-test-imgfscontent.o: unit-test-imgfscontent.c $(SRC_DIR)/imgfs.h
unit-test-imgfscontent: unit-test-imgfscontent.o $(OBJS)

# ======================================================================
unit-test-imgfsresolutions.o: unit-test-imgfsresolutions.c $(SRC_DIR)/imgfs.h
unit-test-imgfsresolutions: unit-test-imgfsresolutions.o $(OBJS)

# ======================================================================
unit-test-imgfsinsert.o: unit-test-imgfsinsert.c $(SRC_DIR)/imgfs.h
unit-test-imgfsinsert: unit-test-imgfsinsert.o $(OBJS)

# ======================================================================
unit-test-imgfsread.o: unit-test-imgfsread.c $(SRC_DIR)/imgfs.h
unit-test-imgfsread: unit-test-imgfsread.o $(OBJS)

# ======================================================================
unit-test-http.o: unit-test-http.c $(SRC_DIR)/imgfs.h
unit-test-http: unit-test-http.o $(OBJS)

# ======================================================================
.PHONY: clean dist-clean reset

clean::
	-$(RM) *.o *~

dump-clean:
	-$(RM) $(DATA_DIR)/dump*

dist-clean: clean dump-clean
	-$(RM) $(foreach T,$(TARGETS),unit-test-$(T))


reset: dist-clean all
