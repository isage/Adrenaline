all:
	make -C cef
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

cmake:
	cmake -S cef -B cef/build -DCMAKE_BUILD_TYPE=Release -DUSE_PSP_PACKER=1
	cmake --build cef/build -j
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build -j

cmake-debug:
	cmake -S cef -B cef/build -DCMAKE_BUILD_TYPE=Release -DUSE_PSP_PACKER=1 -DDEBUG=2 -DSYSTEMCTRL_DEBUG=3
	cmake --build cef/build -j
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build -j

clean:
	rm -rf build cef/build
	make -C cef clean