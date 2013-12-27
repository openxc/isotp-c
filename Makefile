CC = gcc
INCLUDES = -Isrc
CFLAGS = $(INCLUDES) -c -w -Wall -Werror -g -ggdb
LDFLAGS =
LDLIBS = -lcheck

TEST_DIR = tests

# Guard against \r\n line endings only in Cygwin
OSTYPE := $(shell uname)
ifneq ($(OSTYPE),Darwin)
	OSTYPE := $(shell uname -o)
	ifeq ($(OSTYPE),Cygwin)
		TEST_SET_OPTS = igncr
	endif
endif

SRC = $(wildcard src/**/*.c)
OBJS = $(SRC:.c=.o)
TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(TEST_SRC:.c=.o)
TESTS=$(patsubst %.c,%.bin,$(TEST_SRC))

all: $(OBJS)

test: $(TESTS)
	@set -o $(TEST_SET_OPTS) >/dev/null 2>&1
	@export SHELLOPTS
	@sh runtests.sh $(TEST_DIR)

$(TEST_DIR)/%.bin: $(TEST_DIR)/%.o $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(CC_SYMBOLS) $(INCLUDES) -o $@ $^ $(LDLIBS)

clean:
	rm -rf **/*.o $(TEST_DIR)/*.bin
