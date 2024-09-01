import cv2
import time
import os

cap = cv2.VideoCapture(0)

# 프레임 초기 인덱스
frame_index = 0

# 현재 시간 기록
start_time = time.time()
save_dir = '/Users/dasfef/Desktop/WORK/webcam/frameCapture'

if not os.path.exists(save_dir):
    os.makedirs(save_dir)

while True:
    ret, frame = cap.read()
    # 현재 시간과 시작 시간을 비교하여 5초마다 이미지 저장
    if time.time() - start_time >= 5:
        start_time = time.time()

        if ret:
            cv2.imwrite(os.path.join(save_dir, 'frame_{}.png'.format(start_time)), frame)
            frame_index += 1

    cv2.imshow('FRAME', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()