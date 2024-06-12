#include <iostream>
#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>

using namespace libcamera;

int main()
{
    CameraManager manager;
    manager.start();

    /* Acquire a camera */
    std::shared_ptr<Camera> camera = manager.get("camera_name"); // Replace "camera_name" with the name of your camera

    if (!camera) {
        std::cerr << "Failed to find camera" << std::endl;
        return 1;
    }

    if (camera->acquire()) {
        std::cerr << "Failed to acquire camera" << std::endl;
        return 1;
    }

    /* Configure camera */
    CameraConfiguration config;
    if (camera->configure(&config)) {
        std::cerr << "Failed to configure camera" << std::endl;
        return 1;
    }

    /* Start the camera */
    if (camera->start()) {
        std::cerr << "Failed to start camera" << std::endl;
        return 1;
    }

    /* Capture an image */
    std::shared_ptr<FrameBuffer> frameBuffer = camera->allocateFrameBuffer();
    if (!frameBuffer) {
        std::cerr << "Failed to allocate frame buffer" << std::endl;
        return 1;
    }

    if (camera->queueRequest(frameBuffer)) {
        std::cerr << "Failed to queue request" << std::endl;
        return 1;
    }

    Request *request = camera->capture();
    if (!request) {
        std::cerr << "Failed to capture image" << std::endl;
        return 1;
    }

    /* Wait for the request to complete */
    request->wait();
    if (request->status() == Request::RequestComplete) {
        std::cout << "Image captured successfully" << std::endl;

        /* Save the captured image */
        frameBuffer->save("test.jpg"); // Save the image as test.jpg
    } else {
        std::cerr << "Failed to capture image" << std::endl;
    }

    /* Stop and release the camera */
    camera->stop();
    camera->release();

    return 0;
}
