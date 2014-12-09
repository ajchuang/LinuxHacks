CC := arm-none-linux-gnueabi-gcc
LD := arm-none-linux-gnueabi-gcc
CFLAGS := -g -Wall -Werror -static
INCLUDE = -I. -I../../flo-kernel/usr/include -I../../flo-kernel/include -I../../flo-kernel/arch/arm/include
LDFLAGS := -static $(INCLUDE)
TARGET_DIR := /data/misc
TARGET := gpsd

OBJECTS := gpsd.o

all: gpsd 

: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f gpsd 
	rm -f gpsd.o

.PHONY: clean

install: $(TARGET)
	@echo [INSTALL] moving $(TARGET) to [$(TARGET_DIR)]...
	@adb remount
	@adb push $(TARGET) $(TARGET_DIR)
	@adb shell chmod 6755 $(TARGET_DIR)/$(TARGET)

user_install: $(TARGET)
	@echo [INSTALL:user] moving $(TARGET) to [$(TARGET_DIR)]...
	@adb remount
	@adb push $(TARGET) $(TARGET_DIR)
	@adb shell chmod 0750 $(TARGET_DIR)/$(TARGET)
	@adb shell chown compass.system $(TARGET_DIR)/$(TARGET)

run: $(TARGET)
	@echo [RUN] calling $(TARGET_DIR)/$(TARGET)
	@adb shell stop akmd
	@adb shell start akmd
	@echo 
	@echo "You may have to keep your screen off for this to work..."
	@echo 
	@sleep 2
	@echo "$(TARGET_DIR)/$(TARGET) && exit" | adb shell
