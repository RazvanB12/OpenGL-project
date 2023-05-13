#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        //cross product ca sa obtinem normala dintre fata si sus - vezi in poza  
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }
    
    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }

    glm::vec3 Camera::getCameraTarget() {
        return this->cameraTarget;
    }

    glm::vec3 Camera::getCameraFrontDirection() {
        return this->cameraFrontDirection;
    }

    glm::vec3 Camera::getCameraUpDirection() {
        return this->cameraUpDirection;
    }

    glm::vec3 Camera::getCameraRightDirection() {
        return this->cameraRightDirection;
    }

    //resets camera to origin
    void Camera::resetCamera() {
        this->cameraPosition = glm::vec3(0.0f, 1.0f, 3.0f);
        this->cameraTarget = glm::vec3(0.0f, 1.0f, -10.0f);
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) 
        {
        //din poza
        case MOVE_FORWARD:
            cameraPosition += cameraFrontDirection * speed;
            cameraTarget += cameraFrontDirection * speed;
            break;

        case MOVE_BACKWARD:
            cameraPosition -= cameraFrontDirection * speed;
            cameraTarget -= cameraFrontDirection * speed;
            break;

        case MOVE_UP:
            cameraPosition += cameraUpDirection * speed;
            cameraTarget += cameraUpDirection * speed;
            break;

        case MOVE_DOWN:
            cameraPosition -= cameraUpDirection * speed;
            cameraTarget -= cameraUpDirection * speed;
            break;

        case MOVE_RIGHT:
            cameraPosition += cameraRightDirection * speed;
            cameraTarget += cameraRightDirection * speed;
            break;

        case MOVE_LEFT:
            cameraPosition -= cameraRightDirection * speed;
            cameraTarget -= cameraRightDirection * speed;
            break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        this->cameraTarget.x = this->cameraPosition.x + sin(yaw) * cos(pitch);
        this->cameraTarget.y = this->cameraPosition.y + sin(pitch);
        this->cameraTarget.z = this->cameraPosition.z - cos(yaw) * cos(pitch);

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
    }

    void Camera::changeDirectionView(float angle) {
        this->cameraTarget.x = this->cameraPosition.x + sin(angle);
        this->cameraTarget.y = this->cameraPosition.y;
        this->cameraTarget.z = this->cameraPosition.z - cos(angle);
    }
}