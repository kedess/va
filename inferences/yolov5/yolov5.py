import torch
import socket
from io import BytesIO
from PIL import Image
import json
import time
import uuid
import cv2
from pathlib import Path

from _thread import *
import threading

def threaded(conn, model):
	with conn:
		buffer = BytesIO()
		buffer_len = -1
		while True: 
			data = conn.recv(4096)
			if not data:
				break
			if buffer_len == -1:
				buffer_len = int.from_bytes(data[:8], byteorder='little')
				buffer.write(data[8:])
			else:
				buffer.write(data)
			if buffer.getbuffer().nbytes == buffer_len:
				file_tmp = Path(str(uuid.uuid4()))
				file_tmp.write_bytes(buffer.getbuffer())
				cap=cv2.VideoCapture(file_tmp.name);
				count = 0
				json_data_list = []; 
				while(cap.isOpened()):
					success, frame = cap.read()
					if success:
						ret,buffer = cv2.imencode('.jpg',frame)
						if ret:
							count += 1
							img = Image.open(BytesIO(buffer.tobytes()))
							results = model(img)
							pred = results.pandas().xyxy[0]
							inferences = list()
							for index, row in pred.iterrows():
								dict_object = dict(name=row['name'], cls=row['class'], confidence=row['confidence'], xmin=int(row['xmin']), ymin=int(row['ymin']), xmax=int(row['xmax']), ymax=int(row['ymax']))
								inferences.append(dict_object)
								# print(count, row['name'], row['class'], row['confidence'], int(row['xmin']), int(row['ymin']), int(row['xmax']), int(row['ymax']))
							json_data_list.append(inferences)
					else:
						break
				conn.sendall(bytes(json.dumps(json_data_list), 'utf-8'))
				conn.sendall(bytes('\r\n\r\n', 'utf-8'))
				file_tmp.unlink()
				cap.release()
			
def main():
	# Model
	model = torch.hub.load('ultralytics/yolov5', 'yolov5s')  # or yolov5n - yolov5x6, custom
	
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
	s.bind(('localhost', 3030))
	s.listen()
	with s:
		while True:
			conn, addr = s.accept()
			start_new_thread(threaded, (conn, model))

if __name__ == '__main__':
    main()
