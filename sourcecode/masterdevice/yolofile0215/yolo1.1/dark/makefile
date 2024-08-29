all:
	rm -rf /tmp/core*
	rm -rf  main
	g++ -std=c++11 -O3 dark.cpp -o dark `pkg-config --libs --static opencv4` `pkg-config --cflags opencv4`
