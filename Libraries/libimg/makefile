CC = clang
CFLAGS = --target=riscv64 -march=rv64gc -mabi=lp64d -mno-relax -fPIC -fdata-sections -ffunction-sections
INCLUDE = ${qorIncludePath}
LINK = ar
LINKFLAGS = rvs

INCLUDES = 

LIB_DIR = ${qorLibPath}
OUTPUT_DIR = bin
BUILD_DIR = build
SRC_DIR = src

_LIBS = 
LIBS = $(patsubst %,$(LIB_DIR)/%,$(_LIBS))

_OBJ = generic.o bmp.o png.o
OBJ = $(patsubst %,$(BUILD_DIR)/%,$(_OBJ))

$(OUTPUT_DIR)/libimg.a : $(OUTPUT_DIR) $(BUILD_DIR) $(OBJ) $(LIBS)
	$(LINK) $(LINKFLAGS) $@ $(OBJ)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(INCLUDES)
	$(CC) $(CFLAGS) -isystem $(INCLUDE) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.s $(INCLUDES)
	$(CC) $(CFLAGS) -isystem  $(INCLUDE) -c $< -o $@

$(BUILD_DIR) :
	[ ! -d "$(BUILD_DIR)" ] && mkdir $(BUILD_DIR)

$(OUTPUT_DIR) :
	[ ! -d "$(OUTPUT_DIR)" ] && mkdir $(OUTPUT_DIR)


.PHONY: clean

clean:
	rm -rf build/*
