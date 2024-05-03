# Makefile for CS-202 course - spring 2024
# Author: Ludovic Mermod, Jean-Cédric Chappelier & Édouard Bugnion
# Date: Mars 2024

# main targets

# 	style: astyle formatting
# 	doc: create documentation
#       feedback: execute tests within a container from source repo
#       check: local tests

# Note: builds with address sanitizer by default

TEST_DIR = $(PWD)/../provided/tests

## You can change the tag to run week07, week08, etc.
IMAGE := chappeli/cs202-feedback:latest

#########################################################################
# DO NOT EDIT BELOW THIS LINE
#

# ======================================================================
# Compiler and Link

CC = clang
CFLAGS += -g
CFLAGS += -pedantic -Wall
CFLAGS += -Wextra -Wfloat-equal -Wshadow -Wpointer-arith -Wbad-function-cast -Wwrite-strings \
-Wconversion -Wunreachable-code -Wcast-qual -W -Wformat=2 -Winit-self -Wuninitialized

CFLAGS += -Wcast-align

## may require: export ASAN_OPTIONS=allocator_may_return_null=1
#               export ASAN_OPTIONS=verify_asan_link_order=0
# different -fsanitize options are available, including -fsanitize=memory
CPPFLAGS += -fsanitize=address
LDFLAGS  += -fsanitize=address
LDLIBS   += -fsanitize=address

CFLAGS   += -DWEEK=12

ifdef DEBUG
# add the debug flag, may need to comment this line when doing make feedback
#TODO : Make feedback should build with -UDEBUG
CPPFLAGS += -DDEBUG
endif

.PHONY: all all-deferred

EXCLUDE_SRCS = imgfscmd.c tcp-test-client.c tcp-test-server.c http-test-server.c imgfs_server.c
SRCS = $(filter-out $(EXCLUDE_SRCS), $(wildcard *.c))

LDLIBS += -lm -lssl -lcrypto

OBJS=$(subst .c,.o,$(SRCS))

all:: all-deferred


imgfscmd: $(OBJS) imgfscmd.o

imgfs_server: $(OBJS) imgfs_server.o

tcp: tcp-test-client tcp-test-server
tcp-test-client: util.o tcp-test-client.o socket_layer.o
tcp-test-server: util.o tcp-test-server.o socket_layer.o

http-test-server: http-test-server.o http_net.o http_prot.o socket_layer.o error.o util.o

# Computes the valid targets for `all`
TARGETS = imgfscmd

ifneq (,$(wildcard ./imgfs_server.c))
TARGETS += imgfs_server
endif

ifneq (,$(wildcard ./tcp-test-*.c))
TARGETS += tcp tcp-test-client tcp-test-server
endif

ifneq (,$(wildcard ./http-test-server.c))
TARGETS += http-test-server
endif

all-deferred:: $(TARGETS)


.PHONY: depend clean new static-check check release doc

# automatically generate the dependencies
# including .h dependencies !

depend: .depend

.depend: $(SRCS)
	bash -c "cat $(foreach SRC,$(SRCS),<($(CC) $(CFLAGS) -MM -MT $(SRC:.c=.o) $(SRC))) > ./.depend"

ifneq ($(MAKECMDGOALS),clean)
-include .depend
endif

clean::
	-@/bin/rm -f *.o *~  .depend $(TARGETS)
	$(MAKE) -C $(TEST_DIR)/unit dist-clean

new: clean all

static-check:
	CCC_CC=$(CC) scan-build -analyze-headers --status-bugs -maxloop 64 make -j1 new

style:
	astyle -n -o -A8 -xt0 *.[ch]

unit-tests:
	$(MAKE) SRC_DIR=$${PWD} -B -C $(TEST_DIR)/unit

test-%:
	$(MAKE) SRC_DIR=$${PWD} -B -C $(TEST_DIR)/unit $*

$(TEST_DIR)/unit/%:
	$(MAKE) SRC_DIR=$${PWD} -B -C $(TEST_DIR)/unit unit-test-$*



dbg: $(TEST_DIR)/unit/$(EXE)
ifndef TEST
	$(error Missing test name)
endif
ifndef EXE
	$(error Missing executable name)
endif
	gdb -ex 'set debuginfod enabled off' \
	    -ex 'cd $(TEST_DIR)/unit/' \
	    -ex 'set environment CK_FORK=no' \
	    -ex 'break $(TEST)' -ex 'run'    \
	    -ex 'n' -ex 'layout src'         \
	    '$(TEST_DIR)/unit/unit-test-$(EXE)'

define e2e_test
robot -v srcdir:$(PWD) -v datadir:$(TEST_DIR)/data $(TEST_DIR)/end-to-end/$1 ;
endef
end2end-tests: $(TARGETS)
	$(call e2e_test,week07.robot)
	$(call e2e_test,week08.robot)
	$(call e2e_test,week10.robot)

check: end2end-tests unit-tests

## --------------------------------------------------
# target to do all checks before releasing a new version by staff
release: new check style static-check clean

## --------------------------------------------------
# target to make documentation
doc: Doxyfile
	doxygen Doxyfile

## ======================================================================
## Feedback

REPO := $(shell git config --get remote.origin.url)
SSH_DIR := $(HOME)/.ssh

PLATFORM := linux/amd64 # Set your platform; currently supported: linux/amd64, linux/arm/v7, linux/arm64; use x86_64 for ARM hosts
PLATFORM_FLAG = --platform=$(PLATFORM)

#DOCKER_OPTIONS += --privileged
DOCKER_OPTIONS += -e REPO=$(REPO) -v $(SSH_DIR):/opt/.ssh $(PLATFORM_FLAG) $(IMAGE)

.PHONY: feedback feedback-pull feedback-debug feedback-VM-CO feedback-debug-VM-CO clone-ssh clean-fake-ssh

feedback-pull:
	@echo Will use $(REPO) inside container
	@docker pull $(PLATFORM_FLAG) $(IMAGE)
feedback: feedback-pull
	@docker run -it --rm $(DOCKER_OPTIONS)
feedback-debug: feedback-pull
	@docker run -it --rm --entrypoint=./debugmode.sh $(DOCKER_OPTIONS)

clone-ssh:
	@-$(eval SSH_DIR := $(HOME)/.$(shell date "+%s;$$"|sha256sum|cut -c-32))
	@cp -r $(HOME)/.ssh/. $(SSH_DIR)

clean-fake-ssh:
	@case $(SSH_DIR) in $(HOME)/\.????????????????????????????????) $(RM) -fr $(SSH_DIR) ;; *) echo "Dare not remove \"$(SSH_DIR)\"" ;; esac

feedback-VM-CO: clone-ssh feedback clean-fake-ssh

feedback-debug-VM-CO: clone-ssh feedback-debug clean-fake-ssh

## ======================================================================
## Submit

SUBMIT_SCRIPT=../provided/submit.sh

.PHONY: submit1 submit

submit1: $(SUBMIT_SCRIPT)
	@$(SUBMIT_SCRIPT) 1

submit: submit1
