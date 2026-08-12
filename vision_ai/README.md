# vision_ai - ROS2 Computer Vision Package

ROS2 (Lyrical) computer vision package for Raspberry Pi robotics, providing real-time person detection and face recognition using ONNX 
Runtime.
Includes servo control integration

## Overview

Complete vision pipeline that:
- Captures frames from Raspberry Pi camera (libcamera)
- Detects persons using YOLOv8 neural network
- Recognizes faces using ArcFace embedding model
- Identifies individuals from a person database

## Core Components

- **CamDetectSub** - Main ROS2 node managing camera capture and detection pipeline
- **YoloDetector** - Person detection using YOLOv8 ONNX model
- **ArcFaceModel** - Face embedding extraction
- **CoreDetector** - Orchestrates detection workflow
- **PersonDatabase** - Face recognition and person identification
- **ControlServoNode** - Servo control for pan/tilt

## Build & Installation

```bash
colcon build --packages-select vision_ai
source install/local_setup.bash
```

## Running

```bash
ros2 run vision_ai visionai
```

The node initializes libcamera, loads ONNX models, loads person database, and begins frame capture and processing.

### ROS2 Topics

**Published:**
- `cam/image` - Annotated frames with detections

**Subscribed:**
- `cam/param` - Camera control commands (start/stop)
- `cam/param/servo/angle` - Servo angle commands

## Dependencies

- **ROS2**: rclcpp, std_msgs, sensor_msgs, image_transport, cv_bridge
- **Vision/AI**: ONNX Runtime, OpenCV, libcamera
- **Hardware**: pigpio library
- **Build**: ament_cmake, CMake 3.16+

## Project Structure

```
vision_ai/
├── CMakeLists.txt, package.xml
├── include/                    # Header files
│   ├── defs.h
│   ├── controlservonode.h
│   └── vision_ai/              # Model and detector headers
├── src/                        # Implementation files
├── models/                     # ONNX models (w600k_r50, mobilefacenet, yolov8n_face)
└── face_db/                    # Person face embeddings (one directory per person)
```

## Configuration

### Models
Pre-trained ONNX models in `models/` directory:
- Face recognition : `w600k_r50.onnx` or `mobilefacenet.onnx` for lightweight option
- `yolov8n_face.onnx` - Person detection

### Person Database
Create subdirectories in `face_db/` for each person with face images. The node automatically loads and processes embeddings on startup.

Example:
```
face_db/
├── person1/       # Face images for person1
├── person2/       # Face images for person2
└── person3/       # Face images for person3
```

To add new persons:
1. Create a directory in `face_db/`
2. Add face images (`.jpg`, `.png`)
3. Restart the node to load embeddings
 













