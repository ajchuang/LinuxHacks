# Set this to the name of your program
TARGET = acceleration_d
MY_TARGET = myAcc_d

TARGET_DIR = /data/misc

# Edit this variable to point to all
# of your sources files (make sure
# to put a complete relative path)
SOURCES = accelerationd.c
MY_SOURCES = myAccd.c

CRUNTIME = crtbegin_dynamic.S \
           crtend.S

# -----------------------------------------------
# 
# Don't touch things below here unless
# you know what you're doing :-)
# 
OBJECTS = $(SOURCES:%.c=%.c.o) $(CRUNTIME:%.S=%.S.o)
MY_OBJECTS = $(MY_SOURCES:%.c=%.c.o) $(CRUNTIME:%.S=%.S.o)
INCLUDE = -I. -I./inc -I../flo-kernel/include -I../flo-kernel/arch/arm/include
EXTRA_LIBS = -lc -lhardware
CFLAGS = -g -O2 -Wall $(INCLUDE) $(EXTRA_CFLAGS)
LDFLAGS = --entry=_start --dynamic-linker /system/bin/linker \
          -nostdlib -rpath /system/lib -rpath ./system/lib \
          -L ./system/lib $(EXTRA_LIBS)
CC = arm-none-linux-gnueabi-gcc
LD = arm-none-linux-gnueabi-ld

default: $(SOURCES) $(TARGET) $(MY_SOURCES) $(MY_TARGET)

$(TARGET): $(OBJECTS)
	@echo [ARM-LD] $@
	@$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(MY_TARGET): $(MY_OBJECTS)
	@echo [ARM-LD] $@
	@$(LD) $(LDFLAGS) $(MY_OBJECTS) -o $@

%.c.o: %.c
	@echo [ARM-CC] $<...
	@$(CC) -c $(CFLAGS) $< -o $@

%.S.o: %.S
	@echo [ARM-ASM] $<...
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@echo [CLEAN] removing object files + $(TARGET) + $(MY_TARGET)...
	@rm -f $(OBJECTS) $(TARGET)
	@rm -f $(MY_OBJECTS) $(MY_TARGET)

%_emu: ADB_OPT = -e
%_emu: ADB_TGT = Android Emulator
%_dev: ADB_OPT = -d
%_dev: ADB_TGT = Android Device

install_%: $(TARGET)
	@echo [INSTALL] moving $(TARGET) to $(ADB_TGT) [$(TARGET_DIR)]...
	@adb $(ADB_OPT) remount
	@adb $(ADB_OPT) push $(TARGET) $(TARGET_DIR)
	@adb $(ADB_OPT) shell chmod 6755 $(TARGET_DIR)/$(TARGET)

user_install_%: $(TARGET)
	@echo [INSTALL:user] moving $(TARGET) to $(ADB_TGT) [$(TARGET_DIR)]...
	@adb $(ADB_OPT) remount
	@adb $(ADB_OPT) push $(TARGET) $(TARGET_DIR)
	@adb $(ADB_OPT) shell chmod 0750 $(TARGET_DIR)/$(TARGET)
	@adb $(ADB_OPT) shell chown compass.system $(TARGET_DIR)/$(TARGET)

run_%: $(TARGET)
	@echo [RUN] calling $(TARGET_DIR)/$(TARGET) on $(ADB_TGT)...
	@adb $(ADB_OPT) shell stop akmd
	@adb $(ADB_OPT) shell start akmd
	@echo 
	@echo "You may have to keep your screen off for this to work..."
	@echo 
	@sleep 2
	@echo "$(TARGET_DIR)/$(TARGET) && exit" | adb $(ADB_OPT) shell

my_install_%: $(MY_TARGET)
	@echo [INSTALL] moving $(MY_TARGET) to $(ADB_TGT) [$(TARGET_DIR)]...
	@adb $(ADB_OPT) remount
	@adb $(ADB_OPT) push $(MY_TARGET) $(TARGET_DIR)
	@adb $(ADB_OPT) shell chmod 6755 $(TARGET_DIR)/$(MY_TARGET)

my_user_install_%: $(MY_TARGET)
	@echo [INSTALL:user] moving $(MY_TARGET) to $(ADB_TGT) [$(TARGET_DIR)]...
	@adb $(ADB_OPT) remount
	@adb $(ADB_OPT) push $(MY_TARGET) $(TARGET_DIR)
	@adb $(ADB_OPT) shell chmod 0750 $(TARGET_DIR)/$(MY_TARGET)
	@adb $(ADB_OPT) shell chown compass.system $(TARGET_DIR)/$(MY_TARGET)

my_run_%: $(MY_TARGET)
	@echo [RUN] calling $(TARGET_DIR)/$(MY_TARGET) on $(ADB_TGT)...
	@adb $(ADB_OPT) shell stop akmd
	@adb $(ADB_OPT) shell start akmd
	@echo 
	@echo "You may have to keep your screen off for this to work..."
	@echo 
	@sleep 2
	@echo "$(TARGET_DIR)/$(MY_TARGET) && exit" | adb $(ADB_OPT) shell
