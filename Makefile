#
# 'make'        build executable file 'WebCerver'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = clang

# define any compile-time flags
CFLAGS	:= -Wall -Wextra -g -Ilib/sds -Ilib/argtable3 

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = -lm

# define output directory
OUTPUT	:= output

# define source directory
SRC		:= src

# define tests directory
TEST	:= tests

# define include directory
INCLUDE	:= include

# define lib directory
LIB		:= lib

ifeq ($(OS),Windows_NT)
MAIN	:= WebCerver.exe
SOURCEDIRS	:= $(SRC) \ $(LIB)
INCLUDEDIRS	:= $(INCLUDE)
LIBDIRS		:= $(LIB)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /f
MD	:= mkdir
else
MAIN	:= WebCerver
SOURCEDIRS	:= $(shell find $(SRC) -type d) \ $(shell find $(LIB) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
LIBDIRS		:= $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p
endif

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%))

# define the C source files
SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(SOURCEDIRS)))

# define the C object files 
OBJECTS		:= $(SOURCES:.c=.o)

# define the dependency output files
DEPS		:= $(OBJECTS:.o=.d)

# define test source, object, dependency and executable files
TEST_SOURCES	:= $(wildcard $(TEST)/*.c)
TEST_OBJECTS	:= $(TEST_SOURCES:.c=.o)
TEST_DEPS	:= $(TEST_OBJECTS:.o=.d)
TEST_BINS	:= $(patsubst $(TEST)/%.c,$(OUTPUT)/tests/%,$(TEST_SOURCES))

# define http test link dependencies
TEST_HTTP_OBJECTS := src/http/http.o lib/sds/sds.o

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS)

$(OUTPUT)/tests/test_http_parser: tests/test_http_parser.o $(TEST_HTTP_OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LFLAGS) $(LIBS)

$(OUTPUT)/tests:
	$(MD) $(OUTPUT)/tests

$(TEST_BINS): | $(OUTPUT)/tests


# include all .d files
-include $(DEPS) $(TEST_DEPS)

# this is a suffix replacement rule for building .o's and .d's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# -MMD generates dependency output files same name as the .o file
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -MMD $<  -o $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTMAIN)
	$(RM) $(call FIXPATH,$(OBJECTS))
	$(RM) $(call FIXPATH,$(DEPS))
	$(RM) $(call FIXPATH,$(TEST_OBJECTS))
	$(RM) $(call FIXPATH,$(TEST_DEPS))
	$(RM) $(call FIXPATH,$(TEST_BINS))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!


.PHONY: test
test: $(TEST_BINS)
	@for test in $(TEST_BINS); do \
		echo "Running $$test"; \
		./$$test || exit 1; \
	done
