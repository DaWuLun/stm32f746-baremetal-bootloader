CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-ld
OBJDUMP = arm-none-eabi-objdump
NM      = arm-none-eabi-nm
READELF = arm-none-eabi-readelf
OBJCPY  = arm-none-eabi-objcopy

INCLUDE_DIR = include

SRC_DIR    = src
LOADER_DIR = loader

BUILD_DIR        = build
BUILD_LOADER_DIR = build_loader
BIN_DIR          = bin

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

APP_SRCS = $(wildcard $(SRC_DIR)/*.c)
APP_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(APP_SRCS))

LOADER_SRCS = $(wildcard $(LOADER_DIR)/*.c)
LOADER_OBJS = $(patsubst $(LOADER_DIR)/%.c,$(BUILD_LOADER_DIR)/%.o,$(LOADER_SRCS))

DEPS = $(APP_OBJS:.o=.d) $(LOADER_OBJS:.o=.d)

APP_ELF = $(BIN_DIR)/application.elf
APP_BIN = $(BIN_DIR)/application.bin

LOADER_ELF = $(BIN_DIR)/loader.elf
LOADER_BIN = $(BIN_DIR)/loader.bin

.DEFAULT_GOAL := all

.PHONY: all app loader inspect clean

all: $(APP_BIN) $(LOADER_BIN)

app: $(APP_BIN)

loader: $(LOADER_BIN)

$(APP_BIN): $(APP_ELF)
	$(OBJCPY) -O binary $< $@

$(LOADER_BIN): $(LOADER_ELF)
	$(OBJCPY) -O binary $< $@

$(APP_ELF): $(APP_OBJS) $(SRC_DIR)/linker_app.ld | $(BIN_DIR)
	$(LD) -T $(SRC_DIR)/linker_app.ld $(APP_OBJS) -o $@

$(LOADER_ELF): $(LOADER_OBJS) $(LOADER_DIR)/linker_loader.ld | $(BIN_DIR)
	$(LD) -T $(LOADER_DIR)/linker_loader.ld $(LOADER_OBJS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_LOADER_DIR)/%.o: $(LOADER_DIR)/%.c | $(BUILD_LOADER_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR) $(BUILD_LOADER_DIR) $(BIN_DIR):
	mkdir -p $@

inspect: $(APP_ELF) $(LOADER_ELF)
	@echo "========== APPLICATION =========="
	$(OBJDUMP) -h $(APP_ELF)
	$(NM) -n $(APP_ELF)
	$(READELF) -S $(APP_ELF)

	@echo "========== BOOTLOADER =========="
	$(OBJDUMP) -h $(LOADER_ELF)
	$(NM) -n $(LOADER_ELF)
	$(READELF) -S $(LOADER_ELF)

clean:
	rm -rf $(BUILD_DIR) $(BUILD_LOADER_DIR) $(BIN_DIR)

-include $(DEPS)
