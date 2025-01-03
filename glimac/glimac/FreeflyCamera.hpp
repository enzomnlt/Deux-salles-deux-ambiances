#pragma once

#include "glm.hpp"

namespace glimac
{

    class FreeflyCamera
    {

    public:
        FreeflyCamera()
            : m_Position(glm::vec3(0.f, 0.f, 0.f)), m_fPhi(glm::pi<float>()), m_fTheta(0.f)
        {
            computeDirectionVectors();
        }

        void moveLeft(float t)
        {
            glm::vec3 newPosition = m_Position + t * m_LeftVector;
            if (isInBounds(newPosition))
            {
                m_Position = newPosition;
            }
        }

        void moveFront(float t)
        {
            glm::vec3 newPosition = m_Position + t * m_FrontVector;
            if (isInBounds(newPosition))
            {
                m_Position = newPosition;
            }
        }

        void rotateLeft(float degrees)
        {
            m_fPhi += glm::radians(-degrees);
            computeDirectionVectors();
        }

        void rotateUp(float degrees)
        {
            m_fTheta += glm::radians(-degrees);
            computeDirectionVectors();
        }

        glm::mat4 getViewMatrix() const
        {
            return glm::lookAt(m_Position, m_Position + m_FrontVector, m_UpVector);
        }

        glm::vec3 getPosition() const
        {
            return m_Position;
        }

        void setCameraPositionY(float y)
        {
            m_Position.y = y;
        }

    private:
        glm::vec3 m_Position;
        float m_fPhi;
        float m_fTheta;
        glm::vec3 m_FrontVector;
        glm::vec3 m_LeftVector;
        glm::vec3 m_UpVector;

        void computeDirectionVectors()
        {
            m_FrontVector = glm::vec3(
                glm::cos(m_fTheta) * glm::sin(m_fPhi),
                glm::sin(m_fTheta),
                glm::cos(m_fTheta) * glm::cos(m_fPhi));

            m_LeftVector = glm::vec3(
                glm::sin(m_fPhi + glm::half_pi<float>()),
                0,
                glm::cos(m_fPhi + glm::half_pi<float>()));

            m_UpVector = glm::cross(m_FrontVector, m_LeftVector);
        }

        bool isInBounds(const glm::vec3 &position)
        {
            if (position.x < -11.8f || position.x > 11.8f || position.z < -37.8f || position.z > 3.8f)
            {
                return false;
            }
            if (position.z < -15.8f && position.z > -18.1 && (position.x < -1.8f || position.x > 1.8f))
            {
                return false;
            }
            return true;
        }
    };
}