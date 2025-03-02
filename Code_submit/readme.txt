File Receiver_destination_drone dùng để nạp vào vi điều khiển esp32 đặt trên drone để nhận tín hiệu điều khiển và gửi trả về các thông số của góc và tình trạng pin của drone
File Controller_ver2_rainmaker dùng để nạp vào vi điều khiển esp32 s3 là bộ phát điều khiển truyền tín hiệu điều khiển đến drone và nhận lại các thông số do drone gửi về và gửi lên esp rainmaker
File ESP_NOW_Broadcast_Master dùng để nạp vào esp32 CAM dùng để cấu hình cho esp32 CAM và thiết lập giao thức esp NOW để gửi ảnh qua cho esp32 thứ 2 đặt trên drone 
File ESP_NOW_Broadcast_Slave dùng để nạp vào esp32 thứ 2 đặt trên drone dùng để nhận dữ liệu hình ảnh mà esp32 CAM chụp gửi tới, sau đó thông qua module simA7600C sử dụng giao thức PPPOS gửi ảnh lên trên mqtt broker bằng dữ liệu di động mạng 4G
File demo.py dùng để giải mã nhận hình ảnh mà esp32 thứ 2 gửi lên mqtt và lưu hình ảnh về máy tính dùng để xử lý ảnh phát hiện người
File best_train_800x600.pt file model xử lý phân tích hình ảnh gửi về từ mqtt 