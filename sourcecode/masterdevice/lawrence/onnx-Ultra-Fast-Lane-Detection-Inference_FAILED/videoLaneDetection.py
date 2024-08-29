import cv2
import time
from ultrafastLaneDetector import UltrafastLaneDetector, ModelType, YOLOv5

if __name__ == "__main__":
	lanemodel_path = "models/lane.onnx"
	detectmodel_path = "models/yolov5n_lite.onnx"
	model_type = ModelType.CULANE

	# Initialize video
	cap = cv2.VideoCapture("sample.mp4")

	# Initialize lane detection model
	lane_detector = UltrafastLaneDetector(lanemodel_path, model_type)
	object_detector = YOLOv5.YOLOV5(detectmodel_path)

	cv2.namedWindow("Detection", cv2.WINDOW_NORMAL)

	while cap.isOpened():
		frame_time = time.time()

		try:
			# Read frame from the video
			ret, frame = cap.read()
		except:
			continue

		if ret:
			# Detect the lanes
			output_img = lane_detector.detect_lanes(frame)
			objoutput, output_img2 = object_detector.inference(output_img)

			objbox = YOLOv5.filter_box(objoutput, 0.5, 0.5)
			YOLOv5.draw(output_img2, objbox)

			# Calculate FPS
			end_time = time.time()  # Record the time at which frame processing ended
			fps = 1 / (end_time - frame_time)  # Calculate FPS

			# Display FPS on output_img2
			cv2.putText(output_img2, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

			cv2.imshow("Detected lanes", output_img2)

		else:
			break

		# Press key q to stop
		if cv2.waitKey(1) == ord('q'):
			break

	cap.release()
	cv2.destroyAllWindows()
