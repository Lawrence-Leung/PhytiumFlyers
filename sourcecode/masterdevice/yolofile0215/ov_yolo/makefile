all:
	rm -rf /tmp/core*
	rm -rf  main
	g++ -g -std=c++11 -O3 main.cpp -o main `pkg-config --libs --static opencv4` `pkg-config --cflags opencv4` -I/opt/intel/openvino_2022/runtime/include/ -L/usr/local/lib -L/opt/intel/openvino_2022/runtime/lib/intel64/ -lopenvino -lopencv_core -lopencv_highgui -lopencv_imgproc 
	#-I/opt/intel/openvino_2022.3.1/runtime/include
	#
	# -L/usr/local/onnxruntime-linux-x64-1.17.0/lib -lopencv4
	#-I/usr/local/onnxruntime-linux-x64-1.17.0/include -L/usr/local/onnxruntime-linux-x64-1.17.0/lib -L/home/jimkwokying/MNN/build -lMNN -lonnxruntime 
 
