BOARD_IP   ?= 192.168.42.1
BOARD_USER ?= root
BOARD_DIR  ?= /root/app
LIB_DIR	   ?= /root/app/lib
BUILD_DIR  ?= build_duo
TOOLCHAIN  ?= duo-toolchain.cmake

.PHONY: all config build deploy run clean all-in-one

all-in-one: build deploy run

config:
	cmake -B $(BUILD_DIR) \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DWHISPER_BUILD_EXAMPLES=OFF \
		-DGGML_RVV=OFF \
		-DGGML_OPENMP=OFF

build:
	@if [ ! -d "$(BUILD_DIR)" ]; then $(MAKE) config; fi
	cmake --build $(BUILD_DIR) -j$$(nproc)

deploy:
	@echo "--- Transfert de l'exécutable ---"
	scp -O $(BUILD_DIR)/robot_main $(BOARD_USER)@$(BOARD_IP):$(BOARD_DIR)/
	@echo "--- Configuration des permissions ---"
	ssh $(BOARD_USER)@$(BOARD_IP) "chmod +x $(BOARD_DIR)/robot_main"

run:
	ssh -t $(BOARD_USER)@$(BOARD_IP) "cd $(BOARD_DIR) && LD_LIBRARY_PATH=$(LIB_DIR) ./robot_main"

clean:
	rm -rf $(BUILD_DIR)