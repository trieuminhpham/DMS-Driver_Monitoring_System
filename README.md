# Driver Monitoring System (DMS) - Embedded Linux

This repository contains the core C++ application for a Driver Monitoring System (DMS) optimized for embedded Linux devices (e.g., Raspberry Pi). The system utilizes OpenCV and Haar Cascade Classifiers to detect driver drowsiness in real-time.

## 🚀 Key Features

* **Real-time Detection**: Uses `haarcascade` to detect faces and eyes.
* **Multi-threading**: Decouples the camera reading stream from the AI processing stream using `std::thread`, `std::mutex`, and `std::condition_variable` to ensure high performance and prevent CPU bottlenecks.
* **Drowsiness Alert**: Evaluates consecutive frames of closed eyes to trigger warnings.
* **Embedded Optimized**: Written in C++14 and built via Yocto SDK Cross-compilation for maximum performance on ARM architectures.

🛠 Prerequisites

To build this project, you need a Host machine (Linux) with a configured Yocto SDK Toolchain.

    CMake (v3.10 or higher)

    Yocto SDK (Targeting your specific Raspberry Pi architecture)

    OpenCV (Included in the Yocto sysroot)

⚙️ Build Instructions

We use an "out-of-source" build approach to keep the workspace clean.

    Source the Yocto SDK environment:
    ```Bash

    # Note: Replace the path below with your actual Yocto SDK environment setup script path
    source /opt/poky/4.0.33/environment-setup-cortexa72-poky-linux

    Create a build directory and compile:
    ```Bash

    mkdir build && cd build
    cmake ..
    make -j$(nproc)

📦 Deployment & Execution

Once compiled, you need to transfer the binary and the AI models to your Raspberry Pi via SCP.

    Transfer files to the target board:
    ```Bash

    # Create a directory on the Raspberry Pi
    ssh root@<RPI_IP_ADDRESS> "mkdir -p /home/root/dms_app"

    # Copy the binary and models directory
    scp build/dms_core_v2 root@<RPI_IP_ADDRESS>:/home/root/dms_app/
    scp -r models/ root@<RPI_IP_ADDRESS>:/home/root/dms_app/

    Run the application:
    ```Bash

    ssh root@<RPI_IP_ADDRESS>
    cd /home/root/dms_app/
    ./dms_core_v2

⚠️ Known Limitations & Future Improvements

    Haar Cascade Accuracy: The current model uses Bounding Box object detection. It may trigger false positives (e.g., struggling with drivers wearing dark sunglasses, looking down, or in low-light conditions).

    Next Steps: Future iterations will migrate to Facial Landmarks (EAR - Eye Aspect Ratio calculation) using Dlib or MediaPipe to significantly improve accuracy and distinguish between normal blinking and sleeping.

    X11 Forwarding: If running via SSH, you must enable X11 Forwarding (ssh -X) to view the OpenCV imshow UI on your host machine's monitor.
