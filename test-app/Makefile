-include ../.config
-include ../tools/config.mk
-include ../options.mk
-include ./wcs/pkcs11.mk

TARGET?=none
ARCH?=ARM
MCUXPRESSO_CMSIS?=$(MCUXPRESSO)/CMSIS
CFLAGS+=-I. -I..
ifeq ($(TZEN),1)
  # wcs directory contains a user_settings.h, which will conflict with
  # the one in the include directory if the test app needs it (e.g. wolfHSM)
  CFLAGS+=-I./wcs
endif
DEBUG?=1
DELTA_DATA_SIZE?=2000
USE_GCC?=1
USE_GCC_HEADLESS?=1

ifeq ($(SIGN),RSA2048)
  IMAGE_HEADER_SIZE:=512
endif

ifeq ($(SIGN),RSA4096)
  IMAGE_HEADER_SIZE:=1024
endif
ifeq ($(HASH),SHA256)
  WOLFCRYPT_OBJS+=./lib/wolfssl/wolfcrypt/src/sha256.o
  CFLAGS+=-D"WOLFBOOT_HASH_SHA256"
endif

ifeq ($(NVM_FLASH_WRITEONCE),1)
  CFLAGS+=-D"NVM_FLASH_WRITEONCE"
endif


ifeq ($(HASH),SHA3_384)
  WOLFCRYPT_OBJS+=./lib/wolfssl/wolfcrypt/src/sha3.o
  CFLAGS+=-D"WOLFBOOT_HASH_SHA3_384"
endif



ifeq ($(TARGET),ti_hercules)
  APP_OBJS:=app_$(TARGET).o ../test-app/libwolfboot.o
  CFLAGS+=-I"../include"
else
  CFLAGS+=-Wall -Wstack-usage=1024 -ffreestanding -Wno-unused -nostartfiles
  CFLAGS+=-DTARGET_$(TARGET) -I../include
  CFLAGS+=-g
  ifeq ($(USE_GCC),1)
    CFLAGS+=-ggdb3
  else
    CFLAGS+=-gstabs
  endif

  ifeq ($(ARCH),RENESAS_RX)
    APP_OBJS:=app_renesas_rx.o ../test-app/libwolfboot.o ../src/boot_renesas.o ../src/boot_renesas_start.o ../hal/renesas-rx.o
    LDFLAGS+=-ffreestanding -nostartfiles
    CFLAGS+=-DWOLFBOOT_RENESAS_APP
  else
    APP_OBJS:=app_$(TARGET).o led.o system.o timer.o ../test-app/libwolfboot.o
  endif
endif

include ../arch.mk

# Setup default linker flags
LDFLAGS+=-T $(LSCRIPT) -Wl,-gc-sections -Wl,-Map=image.map

# Setup default objcopy flags
OBJCOPY_FLAGS+=--gap-fill $(FILL_BYTE)

ifeq ($(DEBUG_UART),1)
    APP_OBJS+=../src/string.o
endif

ifeq ($(TZEN),1)
    CFLAGS+=-DNONSECURE_APP
    CFLAGS+=-I./
    APP_OBJS+=../hal/$(TARGET)_ns.o
    ifeq ($(WOLFCRYPT_TZ),1)
        APP_OBJS+=../src/wc_secure_calls.o
        ifeq ($(WOLFCRYPT_TZ_PKCS11),1)
            CFLAGS+=-DWOLFSSL_USER_SETTINGS -DWOLFBOOT_PKCS11_APP -DSECURE_PKCS11
            CFLAGS+=-I../lib/wolfPKCS11
            APP_OBJS+=./wcs/pkcs11_test_ecc.o
            APP_OBJS+=./wcs/pkcs11_stub.o
            APP_OBJS+=./wcs/ecc.o
            APP_OBJS+=./wcs/rsa.o
            APP_OBJS+=./wcs/asn.o
            APP_OBJS+=./wcs/aes.o
            APP_OBJS+=./wcs/hmac.o
            APP_OBJS+=./wcs/pwdbased.o
            APP_OBJS+=./wcs/hash.o
            APP_OBJS+=./wcs/sha256.o
            APP_OBJS+=./wcs/sha512.o
            APP_OBJS+=./wcs/sha3.o
            APP_OBJS+=./wcs/integer.o
            APP_OBJS+=./wcs/tfm.o
            APP_OBJS+=./wcs/sp_c32.o
            APP_OBJS+=./wcs/sp_int.o
            APP_OBJS+=./wcs/cryptocb.o
            APP_OBJS+=./wcs/wc_pkcs11.o
            APP_OBJS+=./wcs/memory.o
            APP_OBJS+=./wcs/wolfmath.o
            APP_OBJS+=./wcs/dh.o
            APP_OBJS+=./wcs/random.o
            APP_OBJS+=./wcs/coding.o
            APP_OBJS+=./wcs/wc_encrypt.o
            APP_OBJS+=./wcs/wc_port.o
        endif
        CFLAGS+=-DWOLFBOOT_SECURE_CALLS -Wstack-usage=12944
    endif
else
    APP_OBJS+=../hal/$(TARGET).o
endif

ifeq ($(ARCH),RISCV)
  APP_OBJS+=startup_riscv.o vector_riscv.o
endif

ifeq ($(ARCH),ARM)
  APP_OBJS+=startup_arm.o
endif
ifeq ($(ARCH),ARM_BE)
  APP_OBJS+=startup_arm.o
  CFLAGS+=-DSTACK_PAINTING=0
endif

ifeq ($(ENCRYPT),1)
    CFLAGS+=-D"EXT_ENCRYPTED=1"
endif

ENTRY_POINT=`cat .entry-point-address`
LSCRIPT:=../config/target-app.ld
LSCRIPT_TEMPLATE:=$(ARCH).ld

ifeq ($(TARGET),stm32f7)
  LSCRIPT_TEMPLATE=ARM-stm32f7.ld
  CFLAGS+=-DDUALBANK_SWAP
endif

ifeq ($(TARGET),stm32h7)
  LSCRIPT_TEMPLATE=ARM-stm32h7.ld
endif

ifeq ($(TARGET),stm32c0)
  LSCRIPT_TEMPLATE=ARM-stm32c0.ld
endif

ifeq ($(TARGET),sama5d3)
  APP_OBJS+=./boot_arm32_start.o
  LSCRIPT_TEMPLATE:=$(ARCH)-$(TARGET).ld
endif

ifeq ($(TARGET),stm32l4)
  APP_OBJS+=$(STM32CUBE)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.o
  APP_OBJS+=$(STM32CUBE)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.o
  CFLAGS+=-DSTM32L4A6xx -DUSE_HAL_DRIVER -Isrc -Ihal \
    -I$(STM32CUBE)/Drivers/STM32L4xx_HAL_Driver/Inc/ \
    -I$(STM32CUBE)/Drivers/BSP/STM32L4xx_Nucleo_144/ \
    -I$(STM32CUBE)/Drivers/CMSIS/Device/ST/STM32L4xx/Include/ \
    -I$(STM32CUBE)/Drivers/CMSIS/Include/
endif

ifeq ($(TARGET),stm32l5)
  ifeq ($(TZEN),1)
    LSCRIPT_TEMPLATE=ARM-stm32l5-ns.ld
    APP_OBJS+=wcs/wolfcrypt_secure.o
  else
    LSCRIPT_TEMPLATE=ARM-stm32l5.ld
  endif
  CFLAGS+=-mcpu=cortex-m33 -ffunction-sections -fdata-sections -fno-common
  LDFLAGS+=-mcpu=cortex-m33
  LDFLAGS+=-Wl,-gc-sections -Wl,-Map=image.map
  CFLAGS+=-I..
  APP_OBJS+=../hal/uart/uart_drv_$(UART_TARGET).o
endif

ifeq ($(TARGET),stm32h5)
  ifeq ($(TZEN),1)
    LSCRIPT_TEMPLATE=ARM-stm32h5-ns.ld
    APP_OBJS+=wcs/wolfcrypt_secure.o
    ifeq ($(WOLFCRYPT_TZ),1)
      APP_OBJS+=../lib/wolfssl/wolfcrypt/src/logging.o
      APP_OBJS+=../lib/wolfssl/wolfcrypt/test/test.o
      APP_OBJS+=../lib/wolfssl/wolfcrypt/benchmark/benchmark.o
    endif
  else
    LSCRIPT_TEMPLATE=ARM-stm32h5.ld
  endif
  CFLAGS+=-DAPP_HAS_SYSTICK
  CFLAGS+=-DRAMFUNCTION='__attribute__((used,section(".ramcode"),long_call))'
  CFLAGS+=-mcpu=cortex-m33 -ffunction-sections -fdata-sections -fno-common
  CFLAGS+=-mlong-calls
  LDFLAGS+=-mcpu=cortex-m33
  LDFLAGS+=-Wl,-gc-sections -Wl,-Map=image.map
  CFLAGS+=-I..
  APP_OBJS+=../hal/uart/uart_drv_$(UART_TARGET).o
  ifeq ($(FLASH_OTP_KEYSTORE),1)
    APP_OBJS+=../src/flash_otp_keystore.o
  else
    APP_OBJS+=../src/keystore.o
  endif
endif

ifeq ($(TARGET),stm32u5)
  ifeq ($(TZEN),1)
    LSCRIPT_TEMPLATE=ARM-stm32u5-ns.ld
  else
    LSCRIPT_TEMPLATE=ARM-stm32u5.ld
  endif
  CFLAGS+=-mcpu=cortex-m33
  LDFLAGS+=-mcpu=cortex-m33
endif

ifeq ($(TARGET),nrf5340_net)
  APP_OBJS:=app_$(TARGET).o ../test-app/libwolfboot.o
  LSCRIPT_TEMPLATE=ARM-nrf5340_net.ld
endif

ifeq ($(TARGET),ti_hercules)
  LSCRIPT_TEMPLATE=ARM-r5be.ld
  # Override linker flags
  LDFLAGS=--run_linker $(LSCRIPT) --map_file=image.map
  OBJCOPY_FLAGS=
endif

ifeq ($(TARGET),sim)
  # LD on MacOS does not support "-Map="
  LDMAPSUPPORTED=$(shell $(CC) -Wl,-Map=image.map 2>&1 | grep 'unknown option')
  LDFLAGS=
  ifeq ($(FORCE_32BIT),1)
    LDFLAGS+=-m32
    CFLAGS+=-m32
  endif
  ifeq ($(LDMAPSUPPORTED),)
    # Override linker flags
    LDFLAGS+=-Wl,-Map=image.map
  endif
  ifeq ($(ELF_FLASH_SCATTER),1)
    LSCRIPT_TEMPLATE=sim_scattered.ld
    APP_OBJS=app_sim_scattered.o ../src/string.o
    CFLAGS+=-D"WOLFBOOT_ELF_FLASH_SCATTER=1" -nostartfiles -ffreestanding -static -nostdlib
    LDFLAGS+=-ffreestanding -nostartfiles -static -T$(LSCRIPT) -nostdlib
  else
    APP_OBJS=app_sim.o
    APP_OBJS+=../test-app/libwolfboot.o ../hal/$(TARGET).o
  endif
endif

ifeq ($(EXT_FLASH),1)
  CFLAGS+=-D"EXT_FLASH=1" -D"PART_UPDATE_EXT=1"
endif

ifeq ($(SPI_FLASH),1)
  CFLAGS+=-D"SPI_FLASH"
  APP_OBJS+=../src/spi_flash.o
  ifeq ($(ARCH),RENESAS_RX)
    APP_OBJS+=../hal/spi/spi_drv_renesas_rx.o
  else
    APP_OBJS+=../hal/spi/spi_drv_$(SPI_TARGET).o
  endif
endif

ifeq ($(OCTOSPI_FLASH),1)
  QSPI_FLASH = 1
  CFLAGS+=-D"OCTOSPI_FLASH"
endif

ifeq ($(QSPI_FLASH),1)
  CFLAGS+=-D"QSPI_FLASH"
  APP_OBJS+=../src/qspi_flash.o
  ifeq ($(ARCH),RENESAS_RX)
    APP_OBJS+=../hal/spi/spi_drv_renesas_rx.o
  else
    APP_OBJS+=../hal/spi/spi_drv_$(SPI_TARGET).o
  endif
endif

ifeq ($(UART_FLASH),1)
  CFLAGS+=-D"UART_FLASH=1"
  APP_OBJS+= ../src/uart_flash.o ../hal/uart/uart_drv_$(UART_TARGET).o
else
  ifeq ($(TARGET),stm32wb)
    APP_OBJS+=../hal/uart/uart_drv_$(UART_TARGET).o
  endif
endif

ifeq ($(TARGET),kinetis)
  APP_OBJS+=$(MCUXPRESSO_DRIVERS)/drivers/fsl_clock.o
  ifeq ($(MCUXSDK),1)
    APP_OBJS+=\
      $(MCUXPRESSO)/drivers/flash/fsl_ftfx_flash.o \
      $(MCUXPRESSO)/drivers/flash/fsl_ftfx_cache.o \
      $(MCUXPRESSO)/drivers/flash/fsl_ftfx_controller.o
  else
    APP_OBJS+=\
      $(MCUXPRESSO_DRIVERS)/drivers/fsl_ftfx_flash.o \
      $(MCUXPRESSO_DRIVERS)/drivers/fsl_ftfx_cache.o \
      $(MCUXPRESSO_DRIVERS)/drivers/fsl_ftfx_controller.o
  endif
endif

ifeq ($(TARGET),mcxa)
    LSCRIPT_TEMPLATE=ARM-mcxa.ld
    APP_OBJS+=$(MCUXPRESSO_DRIVERS)/project_template/clock_config.o
    APP_OBJS+=$(MCUXPRESSO_DRIVERS)/drivers/fsl_clock.o
    APP_OBJS+=$(MCUXPRESSO_DRIVERS)/drivers/fsl_reset.o
    APP_OBJS+=$(MCUXPRESSO)/drivers/gpio/fsl_gpio.o
    APP_OBJS+=$(MCUXPRESSO)/drivers/mcx_spc/fsl_spc.o
endif

ifeq ($(TARGET),mcxw)
    LSCRIPT_TEMPLATE=ARM-mcxw.ld
    APP_OBJS+=$(MCUXPRESSO_DRIVERS)/drivers/fsl_clock.o
    APP_OBJS+=$(MCUXPRESSO)/drivers/gpio/fsl_gpio.o
    APP_OBJS+=$(MCUXPRESSO)/drivers/spc/fsl_spc.o
    APP_OBJS+=$(MCUXPRESSO_DRIVERS)/project_template/clock_config.o
    APP_OBJS+=$(MCUXPRESSO)/drivers/ccm32k/fsl_ccm32k.o
    APP_OBJS+=$(MCUXPRESSO_DRIVERS)/drivers/fsl_romapi.o
endif

ifeq ($(TARGET),imx_rt)
  LDFLAGS+=\
    -mcpu=cortex-m7 -Wall --specs=nosys.specs -fno-common -ffunction-sections -fdata-sections \
    -ffreestanding -fno-builtin -mthumb -mapcs -Xlinker --gc-sections -Xlinker -static -Xlinker -z \
    -Xlinker muldefs -Xlinker -Map=output.map -static -lm -lc -lnosys
  LSCRIPT_TEMPLATE=imx_rt.ld
  APP_OBJS+=\
    imx_rt_clock_config.o \
    $(MCUXPRESSO_DRIVERS)/drivers/fsl_clock.o \
    $(MCUXPRESSO)/components/uart/fsl_adapter_lpuart.o \
    $(MCUXPRESSO)/components/serial_manager/fsl_component_serial_manager.o \
    $(MCUXPRESSO)/components/lists/fsl_component_generic_list.o \
    $(MCUXPRESSO)/components/serial_manager/fsl_component_serial_port_uart.o \
    $(MCUXPRESSO)/components/flash/nor/flexspi/fsl_flexspi_nor_flash.o
  ifeq ($(MCUXSDK),1)
    APP_OBJS+=\
      $(MCUXPRESSO)/drivers/igpio/fsl_gpio.o \
      $(MCUXPRESSO)/drivers/cache/armv7-m7/fsl_cache.o \
      $(MCUXPRESSO)/drivers/common/fsl_common.o \
      $(MCUXPRESSO)/drivers/common/fsl_common_arm.o \
      $(MCUXPRESSO)/drivers/flexspi/fsl_flexspi.o \
      $(MCUXPRESSO)/utilities/str/fsl_str.o \
      $(MCUXPRESSO)/drivers/lpuart/fsl_lpuart.o \
      $(MCUXPRESSO)/utilities/debug_console/fsl_debug_console.o
  else
    APP_OBJS+=\
      $(MCUXPRESSO_DRIVERS)/drivers/fsl_gpio.o \
      $(MCUXPRESSO_DRIVERS)/drivers/fsl_cache.o \
      $(MCUXPRESSO_DRIVERS)/drivers/fsl_common.o \
      $(MCUXPRESSO_DRIVERS)/drivers/fsl_common_arm.o \
      $(MCUXPRESSO_DRIVERS)/drivers/fsl_flexspi.o \
      $(MCUXPRESSO_DRIVERS)/utilities/str/fsl_str.o \
      $(MCUXPRESSO_DRIVERS)/drivers/fsl_lpuart.o \
      $(MCUXPRESSO_DRIVERS)/utilities/debug_console/fsl_debug_console.o
  endif

  ifeq ($(MCUXPRESSO_CPU),MIMXRT1042XJM5B)
    CFLAGS+=-I$(MCUXPRESSO_DRIVERS)/project_template/ \
            -I$(MCUXPRESSO)/boards/evkmimxrt1040/xip/
    APP_OBJS+=$(MCUXPRESSO_DRIVERS)/system_MIMXRT1042.o
  else ifeq ($(MCUXPRESSO_CPU),MIMXRT1052DVJ6B)
    CFLAGS+=-I$(MCUXPRESSO_DRIVERS)/project_template/ \
            -I$(MCUXPRESSO)/boards/evkmimxrt1050/xip/
    APP_OBJS+=$(MCUXPRESSO_DRIVERS)/system_MIMXRT1052.o
  else
    ifeq ($(MCUXPRESSO_CPU),MIMXRT1062DVL6A)
      CFLAGS+=-I$(MCUXPRESSO_DRIVERS)/project_template/ \
              -I$(MCUXPRESSO)/boards/evkmimxrt1060/xip/
      APP_OBJS+=$(MCUXPRESSO_DRIVERS)/system_MIMXRT1062.o
    endif
    ifeq ($(MCUXPRESSO_CPU),MIMXRT1062DVL6B)
      CFLAGS+=-I$(MCUXPRESSO_DRIVERS)/project_template/ \
              -I$(MCUXPRESSO)/boards/evkbmimxrt1060/xip/
      APP_OBJS+=$(MCUXPRESSO_DRIVERS)/system_MIMXRT1062.o
    endif
    ifeq ($(MCUXPRESSO_CPU),MIMXRT1064DVL6A)
      CFLAGS+=-I$(MCUXPRESSO_DRIVERS)/project_template/ \
              -I$(MCUXPRESSO)/boards/evkmimxrt1064/xip/
      APP_OBJS+=$(MCUXPRESSO_DRIVERS)/system_MIMXRT1064.o
    endif
  endif
endif

ifeq ($(TARGET),stm32g0)
  CFLAGS+=-DNVM_FLASH_WRITEONCE=1
endif
ifeq ($(TARGET),stm32c0)
  CFLAGS+=-DNVM_FLASH_WRITEONCE=1
endif

ifeq ($(TARGET),hifive1.freedom)
  CFLAGS+=-I$(FREEDOM_E_SDK)/freedom-metal/ -D__METAL_MACHINE_HEADER=\"$(FREEDOM_E_SDK)/bsp/sifive-hifive1/metal.h\"
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/clock.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/led.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/cache.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/cpu.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/gpio.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/interrupt.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/uart.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/tty.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/spi.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/shutdown.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/timer.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,fe310-g000,hfrosc.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,fe310-g000,hfxosc.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,fe310-g000,pll.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,fe310-g000,prci.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,spi0.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,uart0.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,global-external-interrupts0.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,local-external-interrupts0.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,gpio0.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/sifive,gpio-leds.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/riscv,clint0.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/riscv,plic0.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/riscv,cpu.o
  APP_OBJS+=$(FREEDOM_E_SDK)/freedom-metal/src/drivers/fixed-clock.o
endif

ifeq ($(TARGET),hifive1)
  CFLAGS+=-DRAMFUNCTION='__attribute__((used,section(".ramcode")))'
  APP_OBJS+=hifive1_write_page.o
endif

# $(CYPRESS_PDL)/devices/templates/COMPONENT_MTB/COMPONENT_CM0P/system_psoc6_cm0plus.o

ifeq ($(TARGET),psoc6)
  LSCRIPT_TEMPLATE:=ARM-psoc6.ld
  APP_OBJS+= $(CYPRESS_PDL)/drivers/source/cy_gpio.o
  APP_OBJS+= $(CYPRESS_PDL)/drivers/source/cy_device.o
  CFLAGS+=-I$(CYPRESS_PDL)/drivers/include/
  CFLAGS+=-I$(CYPRESS_PDL)/devices/include
  CFLAGS+=-I$(CYPRESS_PDL)/cmsis/include
  CFLAGS+=-I$(CYPRESS_TARGET_LIB)
  CFLAGS+=-I$(CYPRESS_CORE_LIB)/include
  CFLAGS+=-I$(CYPRESS_PDL)/devices/include/ip
  CFLAGS+=-I$(CYPRESS_PDL)/devices/templates/COMPONENT_MTB
  CFLAGS+=-DCY8C624ABZI_D44
endif

ifeq ($(TARGET),x86_fsp_qemu)
  APP_OBJS:=app_$(TARGET).o ../hal/x86_uart.o ../src/x86/common.o ../src/string.o
  LSCRIPT_TEMPLATE:=x86_fsp.ld
  LDFLAGS=
endif

ifeq ($(TARGET),nxp_ls1028a)
  LSCRIPT_TEMPLATE:=AARCH64-ls1028a.ld
endif

ifeq ($(TARGET),pic32ck)
  APP_OBJS+=../hal/pic32c.o
endif

ifeq ($(TARGET), pic32cz)
  APP_OBJS+=../hal/pic32c.o
endif

CFLAGS+=-I../lib/wolfssl

ifeq ($(WOLFHSM_CLIENT),1)
  CFLAGS += -DWOLFSSL_USER_SETTINGS -DSTRING_USER -I../lib/wolfssl
  APP_OBJS += $(WOLFHSM_OBJS)
  APP_OBJS += $(sort $(patsubst ./lib/wolfssl/%, ../lib/wolfssl/%, $(WOLFCRYPT_OBJS)))
endif

ifeq ($(WOLFHSM_SERVER),1)
  CFLAGS += -DWOLFSSL_USER_SETTINGS -DSTRING_USER -I../lib/wolfssl
  APP_OBJS += $(WOLFHSM_OBJS)
  APP_OBJS += $(sort $(patsubst ./lib/wolfssl/%, ../lib/wolfssl/%, $(WOLFCRYPT_OBJS)))
endif


standalone:CFLAGS+=-D"TEST_APP_STANDALONE"
standalone:LDFLAGS:=-T standalone.ld -Wl,-gc-sections -Wl,-Map=image.map

delta-extra-data:CFLAGS+=-D"TEST_DELTA_DATA=$(DELTA_DATA_SIZE)" -ffunction-sections -fdata-sections
delta-extra-data:LDFLAGS=-Wl,-Map=image.map

image.bin: image.elf
	@echo "\t[BIN] $@"
	$(Q)$(OBJCOPY) $(OBJCOPY_FLAGS) -O binary $^ $@

image.hex: image.elf
	@echo "\t[HEX] $@"
	$(Q)$(OBJCOPY) $(OBJCOPY_FLAGS) -O ihex $^ $@

image.srec: image.elf
	@echo "\t[SREC] $@"
	$(Q)$(OBJCOPY) $(OBJCOPY_FLAGS) -O srec $^ $@

ifeq ($(ELF_FLASH_SCATTER),1)
# When ELF_FLASH_SCATTER=1, preprocess the ELF file with the squashelf tool
SQUASHELF_TOOL = ../tools/squashelf/squashelf
image-orig.elf: $(APP_OBJS) $(LSCRIPT)
	@echo "\t[LD] $@"
	$(Q)$(LD) $(LDFLAGS) $(APP_OBJS) $(OUTPUT_FLAG) $@

image.elf: image-orig.elf
	@echo "\t[SQUASHELF] $@"
	$(Q)$(SQUASHELF_TOOL) -v --nosht $< $@
else
# Default behavior when ELF_FLASH_SCATTER is not set
image.elf: $(APP_OBJS) $(LSCRIPT)
	@echo "\t[LD] $@"
	$(Q)$(LD) $(LDFLAGS) $(APP_OBJS) $(OUTPUT_FLAG) $@
endif

standalone: image.bin

delta-extra-data: image.bin

../test-app/libwolfboot.o: ../src/libwolfboot.c FORCE
	@echo "\t[CC-$(ARCH)] $@"
	$(Q)$(CC) $(CFLAGS) -c $(OUTPUT_FLAG) $@ ../src/libwolfboot.c

../hal/$(TARGET)_ns.o: ../hal/$(TARGET).c FORCE
	$(Q)$(CC) $(CFLAGS) -c -o $(@) ../hal/$(TARGET).c -DNONSECURE_APP

%.o:%.c
	@echo "\t[CC-$(ARCH)] $@"
	$(Q)$(CC) $(CFLAGS) -c $(OUTPUT_FLAG) $@ $^

%.o:%.S
	@echo "\t[AS-$(ARCH)] $@"
	$(Q)$(CC) $(CFLAGS) -c $(OUTPUT_FLAG) $@ $^

clean:
	$(Q)rm -f *.bin *.elf tags *.o $(LSCRIPT) $(APP_OBJS) wcs/*.o

$(LSCRIPT): $(LSCRIPT_TEMPLATE) FORCE
	$(Q)printf "%d" $(WOLFBOOT_PARTITION_BOOT_ADDRESS) > .wolfboot-offset
	$(Q)printf "%d" $(WOLFBOOT_PARTITION_SIZE) > .partition-size
	$(Q)printf "%d" $(IMAGE_HEADER_SIZE) > .header-size
	$(Q)expr `cat .wolfboot-offset` + `cat .header-size` > .entry-point
	$(Q)printf "0x%X" `cat .entry-point` > .entry-point
	$(Q)expr `cat .partition-size` - `cat .header-size` > .app-size
	$(Q)printf "0x%X" `cat .app-size` > .app-size
	$(Q)cat $(LSCRIPT_TEMPLATE) | \
		sed -e "s/@WOLFBOOT_TEST_APP_SIZE@/`cat .app-size`/g" | \
		sed -e "s/@WOLFBOOT_TEST_APP_ADDRESS@/`cat .entry-point`/g" | \
		sed -e "s/@WOLFBOOT_LOAD_ADDRESS@/$(WOLFBOOT_LOAD_ADDRESS)/g" | \
		sed -e "s/@WOLFBOOT_PARTITION_BOOT_ADDRESS@/$(WOLFBOOT_PARTITION_BOOT_ADDRESS)/g" | \
		sed -e "s/@WOLFBOOT_PARTITION_SIZE@/$(WOLFBOOT_PARTITION_SIZE)/g" | \
		sed -e "s/@WOLFBOOT_PARTITION_UPDATE_ADDRESS@/$(WOLFBOOT_PARTITION_UPDATE_ADDRESS)/g" | \
		sed -e "s/@WOLFBOOT_PARTITION_SWAP_ADDRESS@/$(WOLFBOOT_PARTITION_SWAP_ADDRESS)/g" \
		> $(@)
	$(Q)rm -f .app-size .entry-point .wolfboot-offset .partition-size .header-size



FORCE:

.PHONY: FORCE clean
