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

C_INC = $(wildcard module/*/inc)
C_SRC = $(wildcard module/*/*.c)
ASM_SRC =

include target/$(TARGET)/specific.mk

CFLAGS += $(MCU) $(C_DEFS) $(addprefix -I, $(C_INC)) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SRC:.c=.o)))
vpath %.c $(sort $(dir $(C_SRC)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SRC:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SRC)))

$(BUILD_DIR)/%.o: %.c makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@
	
$(BUILD_DIR):
	mkdir $@

flash: $(BUILD_DIR)/$(TARGET).elf
	$(OPENOCD) -f interface/stlink.cfg -f target/$(TARGET_FAMILY).cfg -c "program $< verify reset exit"

clean:
	@if exist $(BUILD_DIR) (rmdir /S /Q $(BUILD_DIR))

-include $(wildcard $(BUILD_DIR)/*.d)
