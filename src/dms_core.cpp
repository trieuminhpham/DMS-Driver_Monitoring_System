#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int main() {
    // 1. Khoi tao cac mo hinh AI (Haar Cascades)
    CascadeClassifier face_cascade;
    CascadeClassifier eye_cascade;

    // Kiem tra load file XML (Dam bao 2 file nay nam cung thu muc voi file chay)
    if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
        cout << "[-] LOI: Khong tim thay file haarcascade_frontalface_default.xml" << endl;
        return -1;
    }
    if (!eye_cascade.load("haarcascade_eye.xml")) {
        cout << "[-] LOI: Khong tim thay file haarcascade_eye.xml" << endl;
        return -1;
    }

    // 2. Mo ket noi Camera (Su dung V4L2 backend toi uu nhat cho Linux/Pi)
    VideoCapture cap(0, CAP_V4L2);
    if (!cap.isOpened()) {
        cout << "[-] LOI: Khong the mo camera. Vui long kiem tra cap CSI!" << endl;
        return -1;
    }

    // Giam do phan giai xuong 320x240 de giam tai CPU, ngan chan loi Undervoltage tren Pi 4
    cap.set(CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CAP_PROP_FRAME_HEIGHT, 240);

    cout << "[+] Camera san sang. Dang bat dau phan tich hinh anh..." << endl;

    // 3. Bien trang thai de phat hien buon ngu
    int closed_frames = 0;
    const int DROWSY_LIMIT = 6; // Nguong canh bao (Neu nham mat khoang 6 frames lien tuc)
    Mat frame, gray;

    // Tao cua so hien thi (Cua so nay se duoc day qua ThinkBook thong qua X11)
    namedWindow("DMS - Phat Hien Buon Ngu", WINDOW_AUTOSIZE);

    while (true) {
        // Doc tung khung hinh
        cap >> frame;
        if (frame.empty()) {
            cout << "[-] LOI: Khong the doc du lieu tu camera!" << endl;
            break;
        }

        // Chuyen anh sang ban den trang de AI xu ly nhanh hon
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // Nhan dien khuon mat
        vector<Rect> faces;
        face_cascade.detectMultiScale(gray, faces, 1.3, 5);

        bool eye_detected_this_frame = false;

        // Duyet qua cac khuon mat tim duoc
        for (size_t i = 0; i < faces.size(); i++) {
            Rect face = faces[i];
            // Ve khung mau xanh la cay quanh khuon mat
            rectangle(frame, face, Scalar(0, 255, 0), 2); 

            // Toi uu: Chi quet mat o nua tren cua khuon mat de tranh nhan dien nham mieng thanh mat
            Rect roi_eyes(face.x, face.y, face.width, face.height / 2);
            Mat faceROI = gray(roi_eyes);

            vector<Rect> eyes;
            eye_cascade.detectMultiScale(faceROI, eyes, 1.1, 3);

            // Neu tim thay mat
            if (eyes.size() > 0) {
                eye_detected_this_frame = true;
                for (size_t j = 0; j < eyes.size(); j++) {
                    Point eye_center(face.x + eyes[j].x + eyes[j].width/2, face.y + eyes[j].y + eyes[j].height/2);
                    int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
                    // Ve hinh tron mau xanh duong quanh mat
                    circle(frame, eye_center, radius, Scalar(255, 0, 0), 2); 
                }
            }
        }

        // Logic tinh toan trang thai buon ngu
        if (faces.size() > 0) {
            if (!eye_detected_this_frame) {
                closed_frames++; // Khong thay mat -> tang bien dem
            } else {
                closed_frames = 0; // Thay mat -> reset bien dem
            }
        }

        // 4. Hien thi canh bao
        if (closed_frames >= DROWSY_LIMIT) {
            // Ve mot hinh chu nhat mau do o tren cung man hinh
            rectangle(frame, Point(0, 0), Point(320, 35), Scalar(0, 0, 255), -1); 
            // In chu canh bao mau trang
            putText(frame, "CANH BAO BUON NGU!", Point(30, 25), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
        } else {
            // Trang thai an toan
            putText(frame, "Trang thai: Binh thuong", Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 255, 0), 1);
        }

        // Xuat khung hinh ra cua so GUI
        imshow("DMS - Phat Hien Buon Ngu", frame);

        // Doi 30ms cho moi frame va kiem tra phim bam (Bam 'q' hoac ESC de thoat)
        char c = (char)waitKey(30);
        if (c == 27 || c == 'q') {
            cout << "[*] Nhan lenh thoat tu nguoi dung." << endl;
            break;
        }
    }

    // Don dep bo nho va giai phong camera
    cap.release();
    destroyAllWindows();
    return 0;
}
