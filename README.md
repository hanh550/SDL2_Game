# SDL2_Game
Sinh viên:Ngô Hoàng Anh                     Sinh ngày:15/11/2006
Ngành học:Công Nghệ Thông Tin               Lớp: IT3
Tên game : Shooter game
Lối chơi, logic của game: Game được lấy cảm hứng từ trò chơi bắn gà cổ điển, người chơi sẽ điều khiền 1(hoặc 2) phi thuyền (tùy theo chế độ) để tiêu diệt kẻ địch và né đạn của đối phương. Trò chơi được thiết kế với nhiều tính năng khá hấp dẫn như sau:
   + 1 player: điều khiền phi thuyền(player 1) bằng phím A D để di chuyển và phím space để ra đạn.
+ 2 players:  2 người chơi điều khiển 2 phi thuyền, người chơi 1 sẽ điều khiển như trên và người chơi 2 sẽ điều khiển bằng mũi tên trái phải và ra đạn bằng enter .
+ mục tiêu: người chơi cần tiêu diệt kẻ địch xuất hiện trên màn hình, né tránh đạnn của chúng để dành được mức điểm 500 . khi điểm số đạt đến 500, boss sẽ xuất hiện, hãy tiêu diệt boss để chiến thắng.
+ điểm mới của game so với game bắn gà cổ điển: có chế độ 2 players, có độ khó cao hơn, có khá nhiều loại quái mới… 
- logic game: 
+ người chơi chỉ được di chuyển trong phạm vi màn hình(từ 0 cho đến SCREEN_WIDTH) với tốc độ là PLAYER_SPEED.
+ đạn được điều chỉnh với giới hạn nhất định(bulletCooldown) và được di chuyển với vận tốc (BULLET_SPEED)
+ quản lí kẻ địch: số lượng kẻ địch xuất hiện sẽ được tính theo công thức baseEnemis + (score/100) * 2
+ boss sẽ xuất hiện khi đạt 500 điểm, di chuyển qua lại và di chuyển sang trái phải, bắn viên đạn với xác xuất cao hơn kẻ địch thường gấp 3 lần
+ va chạm tính điểm: kẻ địch sẽ có health là 1 và boss là 50, khi va chạm health lập tức -1, nếu phi thuyền va chạm với kẻ địch hoặc đạn, dẫn đến game over
- logic phụ: sử dụng hình ảnh(tải ảnh và up lên) bằng SDL2 và SDL_image
+ tải font chữ để hiện thị điểm số, game over…
+ hiệu ứng vụ nổ (explosion) xuất hiện khi kẻ địch hoặc phi thuyền bị phá hủy, với thời gian tồn tại 20 frame
- độ khó:
+ boss có lượng máu cáo(50 heatlh)
+ số lượng kẻ địch tăng lên dần dần

 
Đồ họa, âm thanh: 
- Hình ảnh: game sử dụng tổng cộng 16 texture khác nhau: nền menu chicken, nền game, phi thuyền, kẻ địch, boss, đạn …..
+ Đồ họa thiên về chủ đề không gian với phong cách đơn giản nhưng rõ ràng
- Âm thanh: có hiệu ứng bắn nổ, nhạc nền ..
- hoạt ảnh: có hiệu ứng nổ khi bắn. di chuyển, đạn, phi thuyền được tạo bằng frame tạo cảm giác mượt mà 

Cấu trúc của project game: Game được chia làm 3 phần chính như sau:
 - phần mở đầu:
+ chức năng: chọn chế độ chơi(1 player hoặc 2 players)
+ hướng dẫn cách chơi
+ thoát game
- phần chính của game:
+ bảng chơi: màn hình, phi thuyền, boss….
+hiển thị điểm số
+ quản lí kẻ địch, càng chơi lâu càng nhiều địch 
- lối chơi: 
+ người chơi di chuyển và tiêu diệt kẻ địch
+ kẻ địch thường xuyên di chuyển, boss sẽ xuất hiện
+ game sẽ kết thúc khi (trường hợp 1) thắng boss và dành chiến thắng, (trường hợp 2) bị tiêu diệt và game over.
- phần cuối của game: hoạt ảnh kết thúc hoặc thua cuộc, hiển thị số điểm cuối cùng đạt được, nút chức năng(play again? Hoặc exit)
Các chức năng đã cài được cho game: 
- phần mở đầu:
+ 1 player: điều khiển 1 phi thuyền
+ 2 players: điều khiển 2 phi thuyền 
+ hướng dẫn: hiển thị văn bản, có back to menu để quay trở lại màn hình chính
+thoát game: exit
- phần chơi:
+ di chuyền bằng A/D hoặc trái/phải bắn bằng space và enter 
+ hiệu ứng nổ khi kẻ địch bị bắn
+quản lí kẻ địch: kẻ địch sẽ xuất hiện từ trên xuống, nhiều hơn khi điểm cao hơn 
+ boss xuất hiện khi đạt 500 scores , bắn 3 đạn liên tục
- phần kết: 

+thắng: you win, xuất hiện khi boss bị tiêu diệt
+ thua: xuất hiện khi bạn bị tiêu diệt(game over)
- thoát game: nút exit để đóng game hoàn toàn 
Các công cụ hỗ trợ bài làm :
-Về phần code : Chatgpt, Copilot trong VSCode, Google, grook3….
      -Chỉnh sửa ảnh đơn giản : Canva, remove.bg để xóa nền, google, textstudio.com để lấy ảnh chữ.
      -Chỉnh sửa âm thanh : mp3cut.net và audio-joiner.com để cắt và ghép file âm thanh.
      - Nguồn tư liệu : Google, youtube…. 
      -Một số nguồn cơ bản : 
      + trên kênh youtube của 1 vài anh chị khóa trước
      + …..
Nguồn link sửa đổi file âm thanh:
https://online-audio-converter.com/vi/
https://convertio.co/vn/
link hỗ trợ sửa đổi file hình ảnh:
https://convertio.co/vn/jpg-png/
