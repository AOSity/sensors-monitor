TARGET = NUCLEO-F767ZI
BUILD_DIR = build

DEBUG = 1
OPT = -Og

PREFIX = arm-none-eabi-

# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
OPENOCD = openocd

# Collect sources and headers
C_INC = $(wildcard module/*/inc)
C_SRC = $(wildcard module/*/*.c)
ASM_SRC =

# Include target-specific files
include target/$(TARGET)/specific.mk

# Include third-party lib files
LVGL_PATH = third-party/lvgl
C_INC += $(LVGL_PATH)
C_SRC += $(shell find $(LVGL_PATH)/src -type f -name '*.c')

CFLAGS += $(MCU) $(C_DEFS) $(addprefix -I, $(C_INC)) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

CFLAGS += -MMD -MP

# Linker
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# Source to Object mapping
OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SRC))
OBJECTS += $(patsubst %.s,$(BUILD_DIR)/%.o,$(ASM_SRC))

# Default target
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

# Compile C
$(BUILD_DIR)/%.o: %.c makefile
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(basename $@).lst $< -o $@

# Compile assembly (.s)
$(BUILD_DIR)/%.o: %.s makefile
	@mkdir -p $(dir $@)
	$(AS) -c $(CFLAGS) $< -o $@

# Compile assembly (.S)
$(BUILD_DIR)/%.o: %.S makefile
	@mkdir -p $(dir $@)
	$(AS) -c $(CFLAGS) $< -o $@

# Link ELF
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) makefile
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

# Generate HEX and BIN
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(BIN) $< $@

# Flash target
flash: $(BUILD_DIR)/$(TARGET).elf
	$(OPENOCD) -f interface/stlink.cfg -f target/$(TARGET_FAMILY).cfg -c "program $< verify reset exit"

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Auto-include dependency files
-include $(OBJECTS:.o=.d)
