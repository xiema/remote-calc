# OS COMMANDS
ifeq ($(OSTYPE),cygwin)
	CLEANUP=rm -f
	MKDIR=mkdir -p
	TARGET_EXTENSION=out
else ifeq ($(OS),Windows_NT)
	CLEANUP=rm -f
	MKDIR=mkdir
	TARGET_EXTENSION=exe
else
	CLEANUP=rm -f
	MKDIR=mkdir -p
	TARGET_EXTENSION=out
endif

# PATHS

UNITY_DIR = vendor/unity/src/
SRC_DIR = src/
TEST_DIR = test/
BUILD_DIR = build/
DEPENDS_DIR = build/depends/
OBJS_DIR = build/objs/
RESULTS_DIR = build/results/

TEST_PREFIX = Test

BUILD_DIRS = $(BUILD_DIR) $(DEPENDS_DIR) $(OBJS_DIR) $(RESULTS_DIR)

SRC_FILES = $(wildcard $(SRC_DIR)*.c)
TEST_FILES = $(wildcard $(TEST_DIR)*.c)

COMPILE=gcc -c
LINK=gcc
DEPEND=gcc -MM -MG -MF
CFLAGS=-I. -I$(UNITY_DIR) -I$(SRC_DIR)

RESULTS = $(patsubst $(TEST_DIR)$(TEST_PREFIX)%.c,$(RESULTS_DIR)$(TEST_PREFIX)%.txt,$(TEST_FILES))

OBJS_STATIC_LIBS = $(OBJS_DIR)unity.o

# SETUP DIRS

$(BUILD_DIR):
	$(MKDIR) $(BUILD_DIR)

$(DEPENDS_DIR):
	$(MKDIR) $(DEPENDS_DIR)

$(OBJS_DIR):
	$(MKDIR) $(OBJS_DIR)

$(RESULTS_DIR):
	$(MKDIR) $(RESULTS_DIR)

clean:
	$(CLEANUP) $(OBJS_DIR)*.o
	$(CLEANUP) $(BUILD_DIR)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(RESULTS_DIR)*.txt
	$(CLEANUP) $(DEPENDS_DIR)*.d

# OBJECT FILE CREATION

$(OBJS_DIR)%.o:: $(TEST_DIR)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

$(OBJS_DIR)%.o:: $(SRC_DIR)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

$(OBJS_DIR)%.o:: $(UNITY_DIR)%.c $(UNITY_DIR)%.h
	$(COMPILE) $(CFLAGS) $< -o $@

$(DEPENDS_DIR)%.d:: $(TEST_DIR)%.c
	$(DEPEND) $@ $<

.PRECIOUS: $(BUILD_DIR)Test%.$(TARGET_EXTENSION)
.PRECIOUS: $(DEPENDS_DIR)%.d
.PRECIOUS: $(OBJS_DIR)%.o
.PRECIOUS: $(RESULTS_DIR)%.txt

test: CFLAGS += -DTEST
test: $(BUILD_DIRS) $(RESULTS)
	@echo "--------------------\nIGNORES:\n--------------------"
	@echo `grep -s IGNORE $(RESULTS_DIR)*.txt`
	@echo "--------------------\FAILURES:\n--------------------"
	@echo `grep -s FAIL $(RESULTS_DIR)*.txt`
	@echo "\nDONE"

$(RESULTS_DIR)%.txt: $(BUILD_DIR)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

$(BUILD_DIR)$(TEST_PREFIX)%.$(TARGET_EXTENSION): $(OBJS_DIR)$(TEST_PREFIX)%.o $(OBJS_DIR)%.o $(OBJS_STATIC_LIBS) #$(DEPENDS_DIR)Test%.d
	$(LINK) -o $@ $^

SRC_OBJS_FILES = $(patsubst $(SRC_DIR)%.c,$(OBJS_DIR)%.o,$(SRC_FILES))

app: $(SRC_OBJS_FILES)
	$(LINK) -o $(BUILD_DIR)app.$(TARGET_EXTENSION) $^ -lws2_32

.PHONY: clean
.PHONY: test
.PHONY: default

# -include $(DEPENDS_DIR)*.d

# TARGET = network_test
# TEST_TARGET = test_network_test

# INCLUDES = -I$(SRCDIR)

# all: $(TARGET)

# $(TARGET): $(SRC)
# 	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# test: $(TEST_TARGET)
# 	./$(TEST_TARGET)

# $(TEST_TARGET): $(TEST_SRC) $(SRC)
# 	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRC) $(SRC) $(INCLUDES)