TARGET = NUCLEO-F767ZI
CUBE_DIR = NUCLEO-F767ZI
BUILD_DIR = build

all: $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)
	cd $(CUBE_DIR) && $(MAKE) BUILD_DIR=../build

$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

flash: $(BUILD_DIR)/$(TARGET).bin
	STM32_Programmer_CLI -c port=SWD -d $< 0x08000000 -v -rst

clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
