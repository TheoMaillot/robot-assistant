BUILD_DIR ?= build
TARGET    ?= companion_device
ONNX_LIB_DIR ?= $(CURDIR)/third_party/onnxruntime-linux/lib

.PHONY: all build run clean

all: build

build:
	@mkdir -p $(BUILD_DIR)
	@cmake -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR) -j$$(nproc)

run: build
	@LD_LIBRARY_PATH=$(ONNX_LIB_DIR):$$LD_LIBRARY_PATH ./$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)