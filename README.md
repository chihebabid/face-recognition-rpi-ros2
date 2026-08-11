# face-recognition-rpi-ros2
Real-time face detection on Raspberry Pi using YOLOv8n with a Qt-based desktop monitoring and servo control application

## Architecture

- **Raspberry Pi** — ROS 2 C++ application for camera capture, YOLOv8n inference, face detection, and servo control.
- **PC** — Qt/C++ application for real-time visualization and remote servo control.

```text
┌──────────────────────┐
│    Raspberry Pi      │
│                      │
│ Camera → YOLOv8n     │
│          ↓           │
│    Face Detection    │
│          ↓           │
│    Servo Control     │
└──────────┬───────────┘
           │ ROS 2 / Network
           ▼
┌──────────────────────┐
│         PC           │
│                      │
│     Qt Application   │
│  Visualization + GUI │
└──────────────────────┘
```

## Technologies

- C++ / Modern C++
- ROS 2
- OpenCV
- YOLOv8n / ONNX Runtime
- Qt
- Raspberry Pi

## Project Structure

```text
├── vision_ai/       # Raspberry Pi ROS 2 application
├── qt_control/         # PC Qt application
└── LICENSE     # MIT License
```

## License

This project is licensed under the [MIT License](LICENSE).
