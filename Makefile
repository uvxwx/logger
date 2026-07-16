BUILD_DIR := build
DEBUG_BUILD_DIR := build-debug
CMAKE := cmake
CTEST := ctest
CLANG_FORMAT := clang-format
CLANG_TIDY := clang-tidy
CONTAINER_TOOL ?= docker
IMAGE_NAME := logger-project
SOURCE_FILES := $(shell find include src tests \( -name '*.hpp' -o -name '*.cpp' \))

.PHONY: help configure build configure-debug build-debug test format tidy clean container-build docker-build

help:
	@printf '%s\n' \
		'make configure       - configure release build in build' \
		'make build           - configure and build release' \
		'make configure-debug - configure debug build in build-debug' \
		'make build-debug     - configure and build debug' \
		'make test            - build debug and run tests' \
		'make format          - run clang-format on sources' \
		'make tidy            - run clang-tidy using build-debug/compile_commands.json' \
		'make clean           - remove build directories' \
		'make container-build - build OCI image (set CONTAINER_TOOL=podman if needed)' \
		'make docker-build    - alias for container-build'

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Release

build: configure
	$(CMAKE) --build $(BUILD_DIR)

configure-debug:
	$(CMAKE) -S . -B $(DEBUG_BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=Debug

build-debug: configure-debug
	$(CMAKE) --build $(DEBUG_BUILD_DIR)

test: build-debug
	$(CTEST) --test-dir $(DEBUG_BUILD_DIR) --output-on-failure

format:
	$(CLANG_FORMAT) -i $(SOURCE_FILES)

tidy: configure-debug
	$(CLANG_TIDY) -p $(DEBUG_BUILD_DIR) $$(find src tests -name '*.cpp')

clean:
	rm -rf $(BUILD_DIR) $(DEBUG_BUILD_DIR)

container-build:
	$(CONTAINER_TOOL) build -t $(IMAGE_NAME) .

docker-build: container-build
