all:
	rm -rf /tmp/core*
	rm -rf  yolo_test
	g++ -g -std=c++14 -O3 yolo.cpp -o yolo_test `pkg-config --libs --static opencv4` `pkg-config --cflags opencv4` -I/usr/local/onnxruntime-linux-x64-1.17.0/include -L/usr/local/onnxruntime-linux-x64-1.17.0/lib -lonnxruntime

