BUILD_DIR := bin
TARGET := assignment1

.PHONY: configure build run clean clean-build clean-deps re

configure:
	cmake -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Debug

build: configure
	cmake --build $(BUILD_DIR) -v

run: build
	./$(BUILD_DIR)/$(TARGET)

# keeps _deps, removes only compiled build output where possible
clean-build:
	cmake --build $(BUILD_DIR) --target clean || true

# deletes everything, including fetched SFML/imgui deps
clean:
	rm -rf $(BUILD_DIR)

# explicit deps wipe only when FetchContent gets corrupted/stale
clean-deps:
	rm -rf $(BUILD_DIR)/_deps

re: clean-build build