TARGET = rtos_bh1750_nokia5110

# optimization
OPT = -Os

# Build path
BUILD_DIR = build

# C sources
# C_SOURCES =  $(shell find ./ -name '*.c')
C_SOURCES = 
C_SOURCES += ./core/system_stm32f10x.c
C_SOURCES += ./core/core_cm3.c
C_SOURCES += ./src/main.c
C_SOURCES += ./src/citoa.c
C_SOURCES += ./src/nokia5110.c
C_SOURCES += ./src/bh1750.c
C_SOURCES += ./src/usart.c
C_SOURCES += ./src/i2c.c
C_SOURCES += ./lib/src/misc.c
C_SOURCES += ./lib/src/stm32f10x_gpio.c
C_SOURCES += ./lib/src/stm32f10x_can.c
C_SOURCES += ./lib/src/stm32f10x_fsmc.c
C_SOURCES += ./lib/src/stm32f10x_rtc.c
C_SOURCES += ./lib/src/stm32f10x_crc.c
C_SOURCES += ./lib/src/stm32f10x_rcc.c
C_SOURCES += ./lib/src/stm32f10x_exti.c
C_SOURCES += ./lib/src/stm32f10x_dac.c
C_SOURCES += ./lib/src/stm32f10x_usart.c
C_SOURCES += ./lib/src/stm32f10x_pwr.c
C_SOURCES += ./lib/src/stm32f10x_cec.c
C_SOURCES += ./lib/src/stm32f10x_dma.c
C_SOURCES += ./lib/src/stm32f10x_i2c.c
C_SOURCES += ./lib/src/stm32f10x_flash.c
C_SOURCES += ./lib/src/stm32f10x_bkp.c
C_SOURCES += ./lib/src/stm32f10x_sdio.c
C_SOURCES += ./lib/src/stm32f10x_tim.c
C_SOURCES += ./lib/src/stm32f10x_adc.c
C_SOURCES += ./lib/src/stm32f10x_spi.c
C_SOURCES += ./lib/src/stm32f10x_dbgmcu.c
C_SOURCES += ./lib/src/stm32f10x_iwdg.c
C_SOURCES += ./lib/src/stm32f10x_wwdg.c
C_SOURCES += ./FreeRTOS/event_groups.c
C_SOURCES += ./FreeRTOS/croutine.c
C_SOURCES += ./FreeRTOS/list.c
C_SOURCES += ./FreeRTOS/queue.c
C_SOURCES += ./FreeRTOS/tasks.c
C_SOURCES += ./FreeRTOS/timers.c
C_SOURCES += ./FreeRTOS/stream_buffer.c
C_SOURCES += ./FreeRTOS/portable/port.c
C_SOURCES += ./FreeRTOS/portable/heap_4.c

# ASM sources
ASM_SOURCES = startup_stm32f10x_md.s

# binaries
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

# mcu
MCU = -mcpu=cortex-m3 -mthumb

# AS defines
AS_DEFS = 

# C defines
C_DEFS = -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -DRUN_FROM_FLASH=1

# AS includes
AS_INCLUDES = -I core \
           	-I lib/inc \
           	-I lib \
           	-I inc \
           	-I src \
			-I FreeRTOS/include \
		   	-I FreeRTOS/portable \

# C includes
C_INCLUDES = -I core \
           	-I lib/inc \
           	-I lib \
           	-I inc \
           	-I FreeRTOS/include \
			-I FreeRTOS/portable \

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -g -gdwarf-2 -Wall -fdata-sections -ffunction-sections -Os 

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -fomit-frame-pointer -Wall -fverbose-asm

# linker script
LDSCRIPT = stm32f10x_flash.ld

# libraries
LIBS = -lc -lm -lrdimon
LDFLAGS = $(MCU) -nostartfiles -Xlinker -T$(LDSCRIPT) $(LIBS) $(OPT) -g -gdwarf-2 -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref,--gc-sections,--no-warn-rwx-segments,--no-warn-mismatch -Os


all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(BIN) $< $@

$(BUILD_DIR):
	@mkdir $@		

clean:
	@rm -rf .dep $(BUILD_DIR)

erase:
	@st-flash erase

flash:
	@st-flash --reset write $(BUILD_DIR)/$(TARGET).bin 0x8000000

bootloader:
	@./stm32flash_linux -b 115200 -R /dev/ttyUSB0