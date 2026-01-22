all:
	make -C cef
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

clean:
	make -C cef clean
	rm -rf build