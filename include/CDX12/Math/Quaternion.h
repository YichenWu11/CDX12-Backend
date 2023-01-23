#pragma once

#include <CDX12/Math/MathHelper.h>

namespace Chen::CDX12::Math {
    class alignas(16) Quaternion {
    public:
        Quaternion() { m_vec = DirectX::XMQuaternionIdentity(); }
        Quaternion(const DirectX::FXMVECTOR& axis, const float angle) { m_vec = DirectX::XMQuaternionRotationAxis(axis, angle); }
        Quaternion(float pitch, float yaw, float roll) { m_vec = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll); }
        explicit Quaternion(const DirectX::XMMATRIX& matrix) { m_vec = XMQuaternionRotationMatrix(matrix); }
        explicit Quaternion(DirectX::FXMVECTOR vec) { m_vec = vec; }

        XM_CALLCONV operator DirectX::XMVECTOR() const { return m_vec; }

        Quaternion XM_CALLCONV operator~(void) const { return Quaternion(DirectX::XMQuaternionConjugate(m_vec)); }
        Quaternion XM_CALLCONV operator-(void) const { return Quaternion(DirectX::XMVectorNegate(m_vec)); }

        Quaternion XM_CALLCONV        operator*(Quaternion rhs) const { return Quaternion(DirectX::XMQuaternionMultiply(rhs, m_vec)); }
        DirectX::XMVECTOR XM_CALLCONV operator*(DirectX::FXMVECTOR rhs) const { return DirectX::XMVector3Rotate(rhs, m_vec); }

        Quaternion& XM_CALLCONV operator=(Quaternion rhs) {
            m_vec = rhs;
            return *this;
        }
        Quaternion& XM_CALLCONV operator*=(Quaternion rhs) {
            *this = *this * rhs;
            return *this;
        }

    private:
        DirectX::XMVECTOR m_vec;
    };
} // namespace Chen::CDX12::Math
