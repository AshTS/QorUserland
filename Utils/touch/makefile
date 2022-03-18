CC = clang
CFLAGS = --target=riscv64 -march=rv64gc -mno-relax
INCLUDE_DIR = ${qorIncludePath}

LINK = ld.lld
LINKFLAGS = --gc-sections

INCLUDES = libc/stdio.h 
LIB_DIR = ${qorLibPath}

OUTPUT_DIR = bin
BUILD_DIR = bin
SRC_DIR = src

_LIBS = libc.a
LIBS = $(patsubst %,$(LIB_DIR)/%,$(_LIBS))

_OBJ = main.o
OBJ = $(patsubst %,$(BUILD_DIR)/%,$(_OBJ))

RAW_INCLUDES = $(patsubst %, $(INCLUDE_DIR)/%, $(INCLUDES))

$(OUTPUT_DIR)/touch : $(BUILD_DIR) $(OBJ) $(LIBS)
	$(LINK) $(LINKFLAGS) $(OBJ) $(LIBS) -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(RAW_INCLUDES)
	$(CC) $(CFLAGS) -isystem $(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s $(RAW_INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR) :
	[ ! -d "$(BUILD_DIR)" ] && mkdir $(BUILD_DIR)

.PHONY: clean

clean:
	rm -rf build/*
