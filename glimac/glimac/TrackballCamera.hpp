#pragma once

#include "glm.hpp"

namespace glimac {

class TrackballCamera {

public:
    TrackballCamera(): m_fDistance(5.f), m_fAngleX(0.f), m_fAngleY(0.f) {}

    void moveFront(float delta) {
        m_fDistance += delta;
    }

    void rotateLeft(float degrees) {
        m_fAngleY += degrees;
    }

    void rotateUp(float degrees) {
        m_fAngleX += degrees;
    }

    glm::mat4 getViewMatrix() const {
        glm::mat4 viewMatrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -m_fDistance));
        viewMatrix = glm::rotate(viewMatrix, glm::radians(m_fAngleX), glm::vec3(1.f, 0.f, 0.f));
        viewMatrix = glm::rotate(viewMatrix, glm::radians(m_fAngleY), glm::vec3(0.f, 1.f, 0.f));
        return viewMatrix;
    }

private:
    float m_fDistance; 
    float m_fAngleX;
    float m_fAngleY;
};

}