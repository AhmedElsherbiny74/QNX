# === Project Info ===
PLATFORM ?= x86_64
BUILD_PROFILE ?= debug
CONFIG_NAME ?= $(PLATFORM)-$(BUILD_PROFILE)
OUTPUT_DIR = build/$(CONFIG_NAME)

# Executable names
APP_EXE = $(OUTPUT_DIR)/app
SENSOR_EXE = $(OUTPUT_DIR)/sensor

# === Tools ===
CC = qcc -Vgcc_nto$(PLATFORM)
CXX = q++ -Vgcc_nto$(PLATFORM)_cxx
LD = $(CXX)

# === Build flags ===
CCFLAGS_release += -O2
CCFLAGS_debug += -g -O0 -fno-builtin
CCFLAGS_coverage += -g -O0 -ftest-coverage -fprofile-arcs
LDFLAGS_coverage += -ftest-coverage -fprofile-arcs
CCFLAGS_profile += -g -O0 -finstrument-functions
LIBS_profile += -lprofilingS

CCFLAGS_all += -Wall -fmessage-length=0
CCFLAGS_all += $(CCFLAGS_$(BUILD_PROFILE))
LDFLAGS_all += $(LDFLAGS_$(BUILD_PROFILE))
LIBS_all += $(LIBS_$(BUILD_PROFILE))

# === Source Files ===
APP_SRC = src/app.cpp
SENSOR_SRC = src/sensor.cpp

APP_OBJ = $(OUTPUT_DIR)/src/app.o
SENSOR_OBJ = $(OUTPUT_DIR)/src/sensor.o

# === Compilation Rules ===
$(OUTPUT_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c -o $@ $(CCFLAGS_all) $<

# === Linking Rules ===
$(APP_EXE): $(APP_OBJ)
	$(LD) -o $@ $^ $(LDFLAGS_all) $(LIBS_all)

$(SENSOR_EXE): $(SENSOR_OBJ)
	$(LD) -o $@ $^ $(LDFLAGS_all) $(LIBS_all)

# === Default Targets ===
all: $(APP_EXE) $(SENSOR_EXE)

clean:
	rm -rf $(OUTPUT_DIR)

rebuild: clean all
