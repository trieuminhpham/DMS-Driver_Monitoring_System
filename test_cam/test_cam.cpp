#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    cout << "Dang khoi tao camera /dev/video0..." << endl;
    
    // Goi camera thong qua chuan V4L2
    VideoCapture cap(0, CAP_V4L2);
    
    if (!cap.isOpened()) {
        cerr << "Loi: Khong the mo camera!" << endl;
        return -1;
    }

    // Set do phan giai vua phai de test
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    Mat frame;
    // Doc bo vai frame dau de camera tu dong can bang sang (Auto Exposure)
    for(int i = 0; i < 5; i++) {
        cap >> frame;
    }

    if (frame.empty()) {
        cerr << "Loi: Khong chup duoc anh!" << endl;
        return -1;
    }

    // Luu anh ra the nho
    imwrite("ketqua_test.jpg", frame);
    cout << "Tuyet voi! Da luu anh thanh cong vao file ketqua_test.jpg" << endl;

    return 0;
}
