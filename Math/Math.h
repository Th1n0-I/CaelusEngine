//
// Created by volts on 2026-08-24.
//

#ifndef CAELUSENGINE_MATH_H
#define CAELUSENGINE_MATH_H
#include <cmath>


namespace Caelus::Math {
    struct Vector4 {
        float x, y, z, w;

        Vector4 operator+(const Vector4& b) const {return {.x = x + b.x, .y = y + b.y, .z = z + b.z, .w = w + b.w};}
        Vector4 operator-(const Vector4& b) const {return {.x = x - b.x, .y = y - b.y, .z = z - b.z, .w = w - b.w};}
        Vector4 operator*(const float& b) const {return {.x = x * b, .y = y * b, .z = z * b, .w = w * b};}
        Vector4 operator*(const Vector4& b) const {return {.x = x * b.x, .y = y * b.y, .z = z * b.z, .w = w * b.w};}
        Vector4 operator/(const float& b) const {return {.x = x / b, .y = y / b, .z = z / b, .w = w / b};}
        Vector4 operator/(const Vector4& b) const {return{.x = x / b.x, .y = y / b.y, .z = z / b.z, .w = w / b.w};}
        [[nodiscard]] float length() const {return x + y + z + w;}
    };

    static Vector4 normalize(const Vector4& a) { return a / a.length(); }

    static float dot (const Vector4& a, const Vector4& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    struct Vector3 {
        float x, y, z;

        Vector3 operator+(const Vector3& b) const {return {.x = x + b.x, .y = y + b.y, .z = z + b.z};}
        Vector3 operator-(const Vector3& b) const {return {.x = x - b.x, .y = y - b.y, .z = z - b.z};}
        Vector3 operator*(const float& b) const {return {.x = x * b, .y = y * b, .z = z * b};}
        Vector3 operator*(const Vector3& b) const {return {.x = x * b.x, .y = y * b.y, .z = z * b.z};}
        Vector3 operator/(const float& b) const {return {.x = x / b, .y = y / b, .z = z / b};}
        Vector3 operator/(const Vector3& b) const {return{.x = x / b.x, .y = y / b.y, .z = z / b.z};}
        [[nodiscard]] float length() const {return x + y + z;}
    };

    static float dot (const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3 normalize (const Vector3& a) {
        return { a / a.length() };
    }

    struct Matrix4x4 {
        // [COLUMN][ROW]
        float m[4][4];

        Vector4 operator*(const Vector4& v) const {
            const auto v1 = Vector4{.x = v.x * m[0][0], .y = v.x * m[0][1], .z = v.x * m[0][2], .w = v.x * m[0][3]};
            const auto v2 = Vector4{.x = v.y * m[1][0], .y = v.y * m[1][1], .z = v.y * m[1][2], .w = v.y * m[1][3]};
            const auto v3 = Vector4{.x = v.z * m[2][0], .y = v.z * m[2][1], .z = v.z * m[2][2], .w = v.z * m[2][3]};
            const auto v4 = Vector4{.x = v.w * m[3][0], .y = v.w * m[3][1], .z = v.w * m[3][2], .w = v.w * m[3][3]};
            return v1 + v2 + v3 + v4;
        }

        Matrix4x4 operator*( const Matrix4x4& M) const {
            Matrix4x4 result{};
            Vector4 col1 = *this * Vector4{.x = M.m[0][0], .y = M.m[0][1], .z = M.m[0][2], .w = M.m[0][3]};
            Vector4 col2 = *this * Vector4{.x = M.m[1][0], .y = M.m[1][1], .z = M.m[1][2], .w = M.m[1][3]};
            Vector4 col3 = *this * Vector4{.x = M.m[2][0], .y = M.m[2][1], .z = M.m[2][2], .w = M.m[2][3]};
            Vector4 col4 = *this * Vector4{.x = M.m[3][0], .y = M.m[3][1], .z = M.m[3][2], .w = M.m[3][3]};
            return {
                col1.x,col1.y,col1.z,col1.w,
                col2.x,col2.y,col2.z,col2.w,
                col3.x,col3.y,col3.z,col3.w,
                col4.x,col4.y,col4.z,col4.w
            };
        }
    };

    static Matrix4x4 identity() {
        Matrix4x4 r{};
        r.m[0][0] = 1.0f; r.m[1][1] = 1.0f; r.m[2][2] = 1.0f; r.m[3][3] = 1.0f;
        return r;
    }

    static Matrix4x4 scale(const Vector4& scale) {
        Matrix4x4 r = identity();
        r.m[0][0] = scale.x;
        r.m[1][1] = scale.y;
        r.m[2][2] = scale.z;
        return r;
    }

    static Matrix4x4 scale(const Vector3& scale) {
        Matrix4x4 r = identity();
        r.m[0][0] = scale.x;
        r.m[1][1] = scale.y;
        r.m[2][2] = scale.z;
        return r;
    }

    static Matrix4x4 rotateX(const float theta) {
        Matrix4x4 r = identity();
        r.m[1][1] = cosf(theta);
        r.m[1][2] = -sinf(theta);
        r.m[2][1] = sinf(theta);
        r.m[2][2] = cosf(theta);
        return r;
    }

    static Matrix4x4 rotateY(const float theta) {
        Matrix4x4 r = identity();
        r.m[0][0] = cosf(theta);
        r.m[0][2] = sinf(theta);
        r.m[2][0] = -sinf(theta);
        r.m[2][2] = cosf(theta);
        return r;
    }

    static Matrix4x4 rotateZ(const float theta) {
        Matrix4x4 r = identity();
        r.m[0][0] = cosf(theta);
        r.m[0][1] = -sinf(theta);
        r.m[1][0] = sinf(theta);
        r.m[1][1] = cosf(theta);
        return r;
    }

    static Matrix4x4 translate(const Vector4& translate) {
        Matrix4x4 r = identity();
        r.m[0][3] = translate.x;
        r.m[1][3] = translate.y;
        r.m[2][3] = translate.z;
        return r;
    }

    static Matrix4x4 translate(const Vector3& translate) {
        Matrix4x4 r = identity();
        r.m[0][3] = translate.x;
        r.m[1][3] = translate.y;
        r.m[2][3] = translate.z;
        return r;
    }

} // Caelus::Math


#endif //CAELUSENGINE_MATH_H
