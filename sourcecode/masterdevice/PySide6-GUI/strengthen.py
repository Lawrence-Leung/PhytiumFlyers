import cv2	
def method_two():
	img = cv2.imread('/home/jimkwokying/projectTest/masterdevice/import_img/long1.jpg')
	(b, g, r) = cv2.split(img)
	equal_b = cv2.equalizeHist(b)
	equal_g = cv2.equalizeHist(g)
	equal_r = cv2.equalizeHist(r)
	dst = cv2.merge((equal_b, equal_g, equal_r))
	cv2.imshow('img', img)
	cv2.imshow('dst', dst)
	cv2.imwrite('/home/jimkwokying/projectTest/masterdevice/import_img/long11.jpg',dst)
	cv2.waitKey(10000)
	cv2.destroyAllWindows()
	
if __name__ == "__main__":
	method_two()