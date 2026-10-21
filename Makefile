CPP_FILES := $(wildcard src/*/*.cpp src/*/*.h tests/*.cpp)
QML_FILES := $(wildcard src/*/*.qml)

ifeq ($(shell uname),Darwin)
QMLFORMAT := $(shell brew --prefix qtdeclarative)/bin/qmlformat
LLVM_BIN := $(shell brew --prefix llvm)/bin/
else
QMLFORMAT := /usr/lib/qt6/bin/qmlformat
endif

.PHONY: build configure test run format qmlformat lint clean release install

build: build/build.ninja
	cmake --build --preset debug

build/build.ninja:
	cmake --preset debug

configure:
	cmake --preset debug

test: build
	ctest --test-dir build --output-on-failure

run: build
	./build/quicklog

format: qmlformat
	clang-format -i $(CPP_FILES)

qmlformat:
	$(QMLFORMAT) -i $(QML_FILES)

lint: build
	$(LLVM_BIN)run-clang-tidy -clang-tidy-binary $(LLVM_BIN)clang-tidy -p build -quiet '$(CURDIR)/(src|tests)/'
	cmake --build --preset debug --target all_qmllint

clean:
	rm -rf build

release:
	cmake --preset release
	cmake --build --preset release

install: release
	cmake --install build-release
