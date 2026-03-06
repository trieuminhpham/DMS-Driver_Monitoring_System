# Driver Monitoring System (DMS) - Embedded Linux

This repository contains the core C++ application for a **Driver Monitoring System (DMS)** optimized for **embedded Linux devices (Raspberry Pi)**.  
The system captures video from a camera, processes frames using **OpenCV**, and detects driver drowsiness using **Haar Cascade classifiers**.

The project focuses on **embedded system design, multithreading architecture, and Yocto-based cross compilation**, where AI models are used mainly as a practical tool for face and eye detection.

---

# 🚀 Key Features

### Real-time Face and Eye Detection
The system uses **Haar Cascade classifiers** to detect the driver's **face and eyes** in real time.

### Multi-threaded Processing
The application separates **camera capture** and **AI processing** into two threads using:

- `std::thread`
- `std::mutex`
- `std::condition_variable`

This architecture allows the system to **utilize multiple CPU cores on Raspberry Pi 4** and prevents blocking between video capture and AI processing.

### Drowsiness Detection Logic
Driver drowsiness is determined by counting **consecutive frames where eyes remain closed**.

If the number of closed-eye frames exceeds a threshold (e.g., **10 frames**), the system triggers a **warning alert**.

This threshold helps prevent false alarms caused by **natural human blinking**.

### Embedded Optimization
The project is implemented in **C++14** and built using a **Yocto SDK cross-compilation environment**, targeting **ARM architecture on Raspberry Pi**.

---

# 🧠 System Architecture

The system consists of the following pipeline:

Camera Input  
↓  
Video Capture (V4L2 / OpenCV)  
↓  
Frame Buffer (Shared Memory)  
↓  
Face & Eye Detection (OpenCV Haar Cascade)  
↓  
Drowsiness Decision Logic  
↓  
Driver Warning / Display Output

The application uses a **producer–consumer model**:

- **Thread 1 (Producer)**: captures frames from the camera
- **Thread 2 (Consumer)**: processes frames using AI detection

---

# 🛠 Prerequisites

To build this project, you need a **Linux host machine** with a configured **Yocto SDK toolchain**.

Required tools:

- CMake (version 3.10 or higher)
- Yocto SDK targeting Raspberry Pi
- OpenCV (available inside the Yocto sysroot)
- GCC cross compiler from Yocto SDK

---

# ⚙️ Build Instructions

The project uses an **out-of-source build** approach.

### 1. Source the Yocto SDK environment

```bash
source /opt/poky/4.0.33/environment-setup-cortexa72-poky-linux
```

*(Replace the path with your actual Yocto SDK environment script.)*

### 2. Build the project

```bash
mkdir build
cd build

cmake ..
make -j$(nproc)
```

After compilation, the executable binary will be generated in the `build` directory.

---

# 📦 Deployment & Execution

After building, transfer the binary and models to the Raspberry Pi.

### 1. Create a directory on the Raspberry Pi

```bash
ssh root@<RPI_IP_ADDRESS> "mkdir -p /home/root/dms_app"
```

### 2. Transfer application files

```bash
scp build/dms_core_v2 root@<RPI_IP_ADDRESS>:/home/root/dms_app/
scp -r models/ root@<RPI_IP_ADDRESS>:/home/root/dms_app/
```

### 3. Run the application

```bash
ssh root@<RPI_IP_ADDRESS>

cd /home/root/dms_app
./dms_core_v2
```

---

# ⚠️ Known Limitations

### Haar Cascade Accuracy

The current implementation uses **bounding box object detection**, which has several limitations:

- Difficulty detecting eyes when the driver wears **dark sunglasses**
- Lower accuracy when the driver **looks downward**
- Reduced performance in **low-light conditions**

---

# 🖥 X11 Forwarding (Optional)

If running the application via SSH and you want to display the OpenCV window on your host machine:

```bash
ssh -Y root@<RPI_IP_ADDRESS>
```

Then run the program normally.

---

# 📂 Repository Structure

```
DMS
├── src                # Core C++ source code
├── models             # Haar Cascade models
├── docs               # System architecture and diagrams
├── demo               # Demo videos
├── test_cam           # Camera test programs
├── CMakeLists.txt
└── README.md
```

---
