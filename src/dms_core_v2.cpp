#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>

using namespace std;
using namespace cv;

// --- SHARED MEMORY ---
Mat shared_frame;
mutex mtx;
condition_variable cv_frame;
atomic<bool> is_running(true);
bool new_frame_ready = false;

// --- HAM VE KHUNG HUB---
void drawHUD(Mat& img, Rect rect, Scalar color, int thickness = 2) {
    int x = rect.x, y = rect.y, w = rect.width, h = rect.height;
    int len = 20; 
    
    line(img, Point(x, y), Point(x + len, y), color, thickness);
    line(img, Point(x, y), Point(x, y + len), color, thickness);
    
    line(img, Point(x + w, y), Point(x + w - len, y), color, thickness);
    line(img, Point(x + w, y), Point(x + w, y + len), color, thickness);
    
    line(img, Point(x, y + h), Point(x + len, y + h), color, thickness);
    line(img, Point(x, y + h), Point(x, y + h - len), color, thickness);
    
    line(img, Point(x + w, y + h), Point(x + w - len, y + h), color, thickness);
    line(img, Point(x + w, y + h), Point(x + w, y + h - len), color, thickness);
}

// ---------------------------------------------------------
// THREAD 1: CAMERA
// ---------------------------------------------------------
void cameraThreadFunc() {
    VideoCapture cap(0, CAP_V4L2);
    if (!cap.isOpened()) {
        cout << "[-] ERROR: Cannot open V4L2 camera!" << endl;
        is_running = false;
        return;
    }

    cap.set(CAP_PROP_FRAME_WIDTH, 320);
    cap.set(CAP_PROP_FRAME_HEIGHT, 240);
    cap.set(CAP_PROP_FPS, 30);

    Mat local_frame;
    while (is_running) {
        cap >> local_frame;
        if (local_frame.empty()) continue;

        {
            lock_guard<mutex> lock(mtx);
            local_frame.copyTo(shared_frame);
            new_frame_ready = true;
        }
        cv_frame.notify_one();
    }
    cap.release();
}

// ---------------------------------------------------------
// MAIN THREAD: AI & UI HIEN THI
// ---------------------------------------------------------
int main() {
    cout << "[+] DMS System initializing..." << endl;

    CascadeClassifier face_cascade, eye_cascade;
    if (!face_cascade.load("models/haarcascade_frontalface_default.xml") || 
        !eye_cascade.load("models/haarcascade_eye.xml")) {
        cout << "[-] ERROR: Haar Cascade XML files not found!" << endl;
        return -1;
    }

    thread cam_thread(cameraThreadFunc);

    int closed_frames = 0;
    const int DROWSY_LIMIT = 10; 
    Mat frame, gray, display_frame;

    int64 prev_tick = getTickCount();
    double fps = 0.0;

    namedWindow("Driver Monitoring System", WINDOW_AUTOSIZE);

    while (is_running) {
        unique_lock<mutex> lock(mtx);
        cv_frame.wait(lock, []{ return new_frame_ready || !is_running; });
        if (!is_running) break;

        shared_frame.copyTo(frame);
        new_frame_ready = false;
        lock.unlock();

        // 1. TINH FPS
        int64 current_tick = getTickCount();
        double current_fps = getTickFrequency() / (current_tick - prev_tick);
        fps = (fps * 0.9) + (current_fps * 0.1); 
        prev_tick = current_tick;

        // 2. xu LY AI O DO PHAN GIAI THAP(320x240)
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        vector<Rect> faces;
        face_cascade.detectMultiScale(gray, faces, 1.3, 5);
        bool eye_detected_this_frame = false;

        // 3. UPSCALE LEN 640x480 DE VE UI
        resize(frame, display_frame, Size(640, 480));
        float scale = 2.0f; 

        for (size_t i = 0; i < faces.size(); i++) {
            Rect face = faces[i];
            
            Rect display_face(face.x * scale, face.y * scale, face.width * scale, face.height * scale);
            drawHUD(display_frame, display_face, Scalar(0, 255, 255), 2); 

            Rect roi_eyes(face.x, face.y, face.width, face.height / 2);
            Mat faceROI = gray(roi_eyes);
            vector<Rect> eyes;
            eye_cascade.detectMultiScale(faceROI, eyes, 1.1, 3);

            if (eyes.size() > 0) {
                eye_detected_this_frame = true;
                for (size_t j = 0; j < eyes.size(); j++) {
                    Point eye_center((face.x + eyes[j].x + eyes[j].width/2) * scale, 
                                     (face.y + eyes[j].y + eyes[j].height/2) * scale);
                    int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25 * scale);
                    
                    circle(display_frame, eye_center, radius, Scalar(0, 255, 0), 2); 
                    circle(display_frame, eye_center, 2, Scalar(0, 0, 255), -1); 
                }
            }
        }

        // Logic danh gia
        if (faces.size() > 0) {
            if (!eye_detected_this_frame) closed_frames++;
            else closed_frames = 0;
        }

        // --- VE OVERLAY BANG THONG SO---
        Mat overlay;
        display_frame.copyTo(overlay);
        rectangle(overlay, Point(10, 10), Point(260, 140), Scalar(0, 0, 0), -1);
        addWeighted(overlay, 0.6, display_frame, 0.4, 0, display_frame); 

        putText(display_frame, "FPS: " + to_string((int)fps), Point(20, 35), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
        
        string status_text = (faces.empty()) ? "STATUS: N/A" : "STATUS: MONITORING";
        Scalar status_color = (faces.empty()) ? Scalar(150, 150, 150) : Scalar(255, 255, 0);
        putText(display_frame, status_text, Point(20, 65), FONT_HERSHEY_SIMPLEX, 0.6, status_color, 2);

        string eye_text = "EYES: " + string(eye_detected_this_frame ? "OPEN" : "CLOSED");
        Scalar eye_color = eye_detected_this_frame ? Scalar(0, 255, 0) : Scalar(0, 0, 255);
        if(!faces.empty()) putText(display_frame, eye_text, Point(20, 95), FONT_HERSHEY_SIMPLEX, 0.6, eye_color, 2);

        putText(display_frame, "DROWSINESS:", Point(20, 125), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
        int bar_width = 100;
        int fill_width = min(bar_width, (int)((float)closed_frames / DROWSY_LIMIT * bar_width));
        rectangle(display_frame, Point(130, 112), Point(130 + bar_width, 128), Scalar(255, 255, 255), 1);
        if(fill_width > 0) {
            rectangle(display_frame, Point(130, 112), Point(130 + fill_width, 128), Scalar(0, 165, 255), -1);
        }

        // --- HIEN THI CANH BAO ---
        if (closed_frames >= DROWSY_LIMIT) {
            // Nhay khung vien do toan man hinh
            rectangle(display_frame, Point(0, 0), Point(640, 480), Scalar(0, 0, 255), 8); 
            
            // Overlay do cho canh bao
            Mat warning_overlay;
            display_frame.copyTo(warning_overlay);
            rectangle(warning_overlay, Point(0, 400), Point(640, 480), Scalar(0, 0, 200), -1);
            addWeighted(warning_overlay, 0.7, display_frame, 0.3, 0, display_frame);

            putText(display_frame, "WARNING: DROWSINESS DETECTED!", Point(45, 450), 
                    FONT_HERSHEY_TRIPLEX, 0.9, Scalar(255, 255, 255), 2);
        }

        imshow("Driver Monitoring System", display_frame);

        char c = (char)waitKey(1);
        if (c == 27 || c == 'q') {
            is_running = false;
            cv_frame.notify_all(); 
            break;
        }
    }

    cam_thread.join();
    destroyAllWindows();
    cout << "[*] System exited safely." << endl;
    return 0;
}