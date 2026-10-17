CPP_FILES := $(wildcard src/*/*.cpp src/*/*.h tests/*.cpp)
QML_FILES := $(wildcard src/*/*.qml)
QMLFORMAT := /usr/lib/qt6/bin/qmlformat

.PHONY: build configure test run format lint clean release

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

format:
	clang-format -i $(CPP_FILES)
	$(QMLFORMAT) -i $(QML_FILES)

lint: build
	run-clang-tidy -p build -quiet '$(CURDIR)/(src|tests)/'
	cmake --build --preset debug --target all_qmllint

clean:
	rm -rf build

release:
	cmake --preset release
	cmake --build --preset release
