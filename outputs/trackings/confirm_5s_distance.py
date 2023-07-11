import cv2

video = cv2.VideoCapture("output_1_0.mp4")
count = 0
while True:
    ret, frame = video.read()
    if ret < 0:
        break
    if (count % 5 == 0):
        cv2.imshow("frame", frame)
        cv2.waitKey(0)
cv2.destroyAllWindows()
