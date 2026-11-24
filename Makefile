CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-ld
OBJDUMP = arm-none-eabi-objdump
NM      = arm-none-eabi-nm
READELF = arm-none-eabi-readelf
OBJCPY  = arm-none-eabi-objcopy

INCLUDE_DIR = include
SRC_DIR   = src
BUILD_DIR = build
BIN_DIR   = bin

CFLAGS = -mcpu=cortex-m7 \
         -mthumb \
         -O0 \
         -g3 \
         -ffreestanding \
         -fno-builtin \
         -Wall \
         -Wextra \
         -I$(INCLUDE_DIR) \
         -MMD -MP

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

ELF = $(BIN_DIR)/firmware.elf
BIN = $(BIN_DIR)/firmware.bin

.DEFAULT_GOAL := all

.PHONY: all bin inspect clean

all: $(BIN)

bin: $(BIN)

$(BIN): $(ELF)
	$(OBJCPY) -O binary $< $@

$(ELF): $(OBJS) src/linker.ld | $(BIN_DIR)
	$(LD) -T src/linker.ld $(OBJS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

inspect: $(ELF)
	$(OBJDUMP) -h $(ELF)
	$(NM) -n $(ELF)
	$(READELF) -S $(ELF)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

-include $(DEPS)
