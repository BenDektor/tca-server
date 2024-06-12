#include <iostream>
#include <libcamera/libcamera.h>

namespace lc = libcamera;

int main()
{
    lc::CameraManager manager;
    manager.start();

    auto cameras = manager.cameras();
    if (cameras.empty()) {
        std::cerr << "No camera found" << std::endl;
        return 1;
    }

    std::cout << "Available cameras:" << std::endl;
    for (const auto &camera : cameras) {
        std::cout << "ID: "<<camera->id() << std::endl;
    }

    manager.stop();

    return 0;
}
