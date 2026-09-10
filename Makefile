# GNU Arm Embedded toolchain; override with an absolute prefix if needed.
CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE = $(CROSS_COMPILE)size
OPENOCD ?= openocd
SWD_SPEED ?= 1000
OPENOCD_FLAGS := -f interface/cmsis-dap.cfg -c "transport select swd" \
                 -f target/nrf51.cfg -c "adapter speed $(SWD_SPEED)" \
                 -c "reset_config none"
SOFTDEVICE := components/softdevice/s110/hex/s110_nrf51_8.0.0_softdevice.hex
BUILD := build/release

SOURCES := main.c \
  EPD/DEV_Config.c \
  EPD/EPD_4in2.c \
  EPD/EPD_4in2_V2.c \
  EPD/EPD_4in2b_V2.c \
  EPD/EPD_2in13.c \
  EPD/EPD_ble.c \
  components/ble/ble_advertising/ble_advertising.c \
  components/ble/common/ble_advdata.c \
  components/ble/common/ble_conn_params.c \
  components/ble/common/ble_srv_common.c \
  components/drivers_nrf/delay/nrf_delay.c \
  components/drivers_nrf/common/nrf_drv_common.c \
  components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
  components/drivers_nrf/spi_master/nrf_drv_spi.c \
  components/drivers_nrf/pstorage/pstorage.c \
  components/libraries/util/nrf_log.c \
  components/libraries/util/nrf_assert.c \
  components/libraries/util/app_error.c \
  components/libraries/timer/app_timer.c \
  components/softdevice/common/softdevice_handler/softdevice_handler.c \
  components/toolchain/system_nrf51.c
INCLUDES := config \
  EPD \
  components/toolchain \
  components/drivers_nrf/config \
  components/drivers_nrf/common \
  components/drivers_nrf/delay \
  components/drivers_nrf/gpiote \
  components/drivers_nrf/hal \
  components/drivers_nrf/spi_master \
  components/drivers_nrf/pstorage \
  components/drivers_nrf/pstorage/config \
  components/drivers_nrf/twi_master \
  components/libraries/trace \
  components/libraries/timer \
  components/libraries/util \
  components/ble/common \
  components/ble/ble_advertising \
  components/softdevice/common/softdevice_handler \
  components/softdevice/s110/headers \
  components/device \
  components/toolchain/gcc

ifeq ($(DEBUG),1)
BUILD := build/debug
SOURCES += components/drivers_ext/segger_rtt/SEGGER_RTT.c \
           components/drivers_ext/segger_rtt/SEGGER_RTT_printf.c \
           components/drivers_ext/segger_rtt/RTT_Syscalls_GCC.c
INCLUDES += components/drivers_ext/segger_rtt
CPPFLAGS += -DDEBUG -DNRF_LOG_USES_RTT=1
endif

STARTUP := components/toolchain/gcc/gcc_startup_nrf51.s
LINKER_SCRIPT := config/gcc_nrf51.ld
OBJECTS := $(addprefix $(BUILD)/,$(SOURCES:.c=.o)) $(BUILD)/$(STARTUP:.s=.o)
CPUFLAGS := -mcpu=cortex-m0 -mthumb -mfloat-abi=soft
CPPFLAGS += $(addprefix -I,$(INCLUDES)) \
            -DBLE_STACK_SUPPORT_REQD -DS110 -DSWI_DISABLE0 -DSOFTDEVICE_PRESENT -DNRF51
# GNU inline semantics are required by this SDK's external inline functions.
CFLAGS := $(CPUFLAGS) -std=gnu99 -fgnu89-inline -Os -g3 \
          -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall
LDFLAGS := $(CPUFLAGS) --specs=nano.specs --specs=nosys.specs \
           -Lcomponents/toolchain/gcc -T$(LINKER_SCRIPT) \
           -Wl,--gc-sections,-Map=$(BUILD)/EPD.map,--print-memory-usage

.PHONY: all clean size probe flash flash-all
.DELETE_ON_ERROR:
all: $(BUILD)/EPD.hex $(BUILD)/EPD.bin

$(BUILD)/EPD.elf: $(OBJECTS) $(LINKER_SCRIPT) components/toolchain/gcc/nrf5x_common.ld Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/%.o: %.s Makefile
	@mkdir -p $(@D)
	$(CC) $(CPUFLAGS) -x assembler-with-cpp -D__STACK_SIZE=2048 -D__HEAP_SIZE=512 -c $< -o $@

$(BUILD)/%.hex: $(BUILD)/%.elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary $< $@

size: $(BUILD)/EPD.elf
	$(SIZE) $<

# Connect and identify the target without erasing or programming flash.
probe:
	$(OPENOCD) $(OPENOCD_FLAGS) -c "init; targets; shutdown"

# Update only application sectors, preserving SoftDevice and pstorage.
flash: $(BUILD)/EPD.hex
	$(OPENOCD) $(OPENOCD_FLAGS) -c "program {$<} verify reset exit"

# First installation: erases ALL flash and UICR, including saved settings.
# Keep erase and both images in one process and in this order, even with make -j.
flash-all: $(BUILD)/EPD.hex $(SOFTDEVICE)
	@echo "Erasing all firmware, UICR and saved settings; installing S110 and EPD."
	$(OPENOCD) $(OPENOCD_FLAGS) \
		-c "init; reset halt; nrf5 mass_erase" \
		-c "program {$(SOFTDEVICE)} verify" \
		-c "program {$(BUILD)/EPD.hex} verify reset exit"

clean:
	rm -rf build

-include $(OBJECTS:.o=.d)
