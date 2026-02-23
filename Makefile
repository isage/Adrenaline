all:
	make -C cef
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

cmake:
	cmake -S cef -B cef/build
	cmake --build cef/build
	cmake -S . -B build -DCEF_CMAKE=1 -DCMAKE_BUILD_TYPE=Release
	cmake --build build

clean:
	make -C cef clean
	rm -rf build