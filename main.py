from roboflow import Roboflow
import supervision as sv
import cv2
import serial
import time
import threading
import sys
import os

# 시리얼 포트 설정 (아두이노 IDE에서 확인 가능)
my_serial = serial.Serial("COM7", baudrate=9600, timeout=1.0)
time.sleep(2.0)

def detect_thread(): #arduino_thread():
    rf = Roboflow(api_key="XvjqjWhZPd4E4wkAEvSL")
    project = rf.workspace().project("fire-detection-for-khkt")
    model = project.version(3).model

    # 동영상 파일 열기
    video_path = "./fire1.mp4"
    cap = cv2.VideoCapture(video_path)

    label_annotator = sv.LabelAnnotator()
    bounding_box_annotator = sv.BoxAnnotator()
    # 새로운 크기 지정
    new_size = (640, 480)

    while True:
        # 비디오에서 프레임 읽기
        ret, frame = cap.read()
        if not ret:
            break

        # 프레임 크기 조정
        frame = cv2.resize(frame, new_size)

        # 모델로 프레임 예측
        result = model.predict(frame, confidence=30, overlap=30).json()
        labels = [item["class"] for item in result["predictions"]]
        detections = sv.Detections.from_roboflow(result)

        if(is_button_clicked != True & len(labels) != 0):
            send_command("Detect=True")
        else:
            send_command("Detect=False")            
        
        # 수정된 부분: box_annotator -> bounding_box_annotator
        annotated_frame = bounding_box_annotator.annotate(
            scene=frame, detections=detections)
        annotated_frame = label_annotator.annotate(
            scene=annotated_frame, detections=detections, labels=labels)

        # 영상 표시
        cv2.imshow("Detection", annotated_frame)

        # 'q' 키를 누르면 종료
        if cv2.waitKey(30) & 0xFF == ord('q'):
            sys.exit()

    # 비디오 파일과 윈도우 해제
    cap.release()
    cv2.destroyAllWindows()

def send_command(command):
    my_serial.write(command.encode('utf-8'))
    time.sleep(1.0)  # 아두이노가 처리할 시간을 주기 위해 잠시 대기

def send_buzzer(freq):
    sendData = f"BUZZER={freq}\n"
    my_serial.write( sendData.encode() )

serial_receive_data = ""
def serial_read_thread():
    global serial_receive_data, is_button_clicked
    is_button_clicked = False
    while True:
        read_data = my_serial.readline()
        serial_receive_data = read_data.decode()
        
        if("BUTTON1=CLICK" in serial_receive_data):
            is_button_clicked = True
        elif("BUTTON2=CLICK" in serial_receive_data):
            is_button_clicked = False
        
if __name__ == "__main__":
    detect_t = threading.Thread(target=detect_thread)
    thread_sirial_read = threading.Thread(target=serial_read_thread)
    

    detect_t.start()
    print("detect thread start")
    thread_sirial_read.start()
    print("serial thread start")

    detect_t.join()
    thread_sirial_read.join()