#include "Camera.hpp"
#include <iostream>

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction)
        {
        case gps::MOVE_FORWARD:
            this->cameraPosition += this->cameraFrontDirection * speed;
            break;
        case gps::MOVE_BACKWARD:
            this->cameraPosition -= this->cameraFrontDirection * speed;
            break;
        case gps::MOVE_RIGHT:
            this->cameraPosition += this->cameraRightDirection * speed;
            break;
        case gps::MOVE_LEFT:
            this->cameraPosition -= this->cameraRightDirection * speed;
            break;
        default:
            break;
        }
    }



    //update the camera internal parameters following a camera rotate event 
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(front);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }
}
