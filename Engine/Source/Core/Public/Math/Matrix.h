#pragma once

#include "Math/Math.h"
#include "Math/Vector.h"

namespace CE
{
    
    class CORE_API Matrix4x4
    {
    public:

        Matrix4x4()
        {
            memset(rows, 0, sizeof(rows));
        }

        Matrix4x4(Vec4 rows[4])
        {
            for (int i = 0; i < 4; i++)
                this->rows[i] = rows[i];
        }

        Matrix4x4(std::initializer_list<Vec4> rows)
        {
            for (int i = 0; i < 4; i++)
                this->rows[i] = *(rows.begin() + i);
        }

		Matrix4x4(std::initializer_list<f32> cells)
		{
			memset(rows, 0, sizeof(rows));

			for (int i = 0; i < cells.size(); i++)
			{
				int row = cells.size() / 4;
				int col = cells.size() % 4;

				rows[row][col] = *(cells.begin() + i);
			}
		}

        Matrix4x4(const Matrix4x4& copy)
        {
            memcpy(rows, copy.rows, sizeof(rows));
        }

		static Matrix4x4 PerspectiveProjection(float aspect, float fieldOfView, float nearPlane, float farPlane);

		static Matrix4x4 OrthographicProjection(float left, float right, float top, float bottom, float nearPlane, float farPlane);

		inline static Matrix4x4 OrthographicProjection(float aspect, float nearPlane, float farPlane)
		{
			return OrthographicProjection(-aspect, aspect, -1, 1, nearPlane, farPlane);
		}

        inline static Matrix4x4 Translation(const Vec3& translation)
        {
            return Matrix4x4({
                { 1, 0, 0, translation.x },
                { 0, 1, 0, translation.y },
                { 0, 0, 1, translation.z },
                { 0, 0, 0, 1             }
            });
        }

        //! @brief 2D angle in Z axis in degrees.
        static Matrix4x4 Angle(f32 degrees);

        //! @brief 3D euler angles in degrees 
        static Matrix4x4 RotationDegrees(const Vec3& eulerDegrees);

        //! @brief 3D euler angles in radians
        static Matrix4x4 RotationRadians(const Vec3& eulerRadians);

        inline static Matrix4x4 Scale(const Vec2& scale)
        {
            return Matrix4x4({
                { scale.x,  0,       0,     0 },
                { 0,        scale.y, 0,     0 },
                { 0,        0,     1,     0 },
                { 0,        0,     0,     1 }
                });
        }

        inline static Matrix4x4 Scale(const Vec3& scale)
        {
            return Matrix4x4({
                { scale.x,  0,       0,       0 },
                { 0,        scale.y, 0,       0 },
                { 0,        0,       scale.z, 0 },
                { 0,        0,       0,       1 }
            });
        }

        CE_INLINE static Matrix4x4 Zero()
        {
            static Vec4 rows[4] = {
                Vec4{ 0, 0, 0, 0 },
                Vec4{ 0, 0, 0, 0 },
                Vec4{ 0, 0, 0, 0 },
                Vec4{ 0, 0, 0, 0 }
            };

            return Matrix4x4(rows);
        }

        CE_INLINE static Matrix4x4 Identity()
        {
            static Vec4 rows[4] = {
                Vec4{ 1, 0, 0, 0 },
                Vec4{ 0, 1, 0, 0 },
                Vec4{ 0, 0, 1, 0 },
                Vec4{ 0, 0, 0, 1 }
            };

            return Matrix4x4(rows);
        }

        CE_INLINE Vec4& operator[](int index)
        {
            return rows[index];
        }

        CE_INLINE const Vec4& operator[](int index) const
        {
            return rows[index];
        }

        Matrix4x4 operator+(const Matrix4x4& rhs) const;
        Matrix4x4 operator-(const Matrix4x4& rhs) const;

        CE_INLINE Matrix4x4 operator+=(const Matrix4x4& rhs)
        {
            *this = *this + rhs;
            return *this;
        }

        CE_INLINE Matrix4x4 operator-=(const Matrix4x4& rhs)
        {
            *this = *this - rhs;
            return *this;
        }

        Matrix4x4 operator*(f32 rhs) const;
        Matrix4x4 operator/(f32 rhs) const;

        CE_INLINE Matrix4x4 operator*=(f32 rhs)
        {
            *this = *this * rhs;
            return *this;
        }

        CE_INLINE Matrix4x4 operator/=(f32 rhs)
        {
            *this = *this / rhs;
            return *this;
        }

        CE_INLINE Matrix4x4 operator*(const Matrix4x4& rhs) const
        {
            return Multiply(*this, rhs);
        }
        
        CE_INLINE Vec4 operator*(const Vec4& rhs) const
        {
            return Multiply(*this, rhs);
        }

		CE_INLINE Vec4 operator*(const Vec3& rhs) const
		{
			return Multiply(*this, Vec4(rhs, 1.0f));
		}

        CE_INLINE Matrix4x4 operator*=(const Matrix4x4& rhs)
        {
            *this = *this * rhs;
            return *this;
        }

        CE_INLINE bool operator==(const Matrix4x4& rhs) const
        {
            return rows[0] == rhs.rows[0] && rows[1] == rhs.rows[1] && rows[2] == rhs.rows[2] && rows[3] == rhs.rows[3];
        }

        CE_INLINE bool operator!=(const Matrix4x4& rhs) const
        {
            return !(*this == rhs);
        }

        static Matrix4x4 Multiply(const Matrix4x4& lhs, const Matrix4x4& rhs);
        static Vec4 Multiply(const Matrix4x4& lhs, const Vec4& rhs);

        static Matrix4x4 GetTranspose(const Matrix4x4& mat);

        CE_INLINE Matrix4x4 GetTranspose() const
        {
            return GetTranspose(*this);
        }

        CE_INLINE void Transpose()
        {
            *this = GetTranspose(*this);
        }

        CE_INLINE Matrix4x4 GetInverse() const
        {
            return GetInverse(*this);
        }

        CE_INLINE void Invert()
        {
            *this = GetInverse(*this);
        }

        CE_INLINE String ToString() const
        {
            return String::Format("[{} {} {} {}]\n[{} {} {} {}]\n[{} {} {} {}]\n[{} {} {} {}]",
                rows[0][0], rows[0][1], rows[0][2], rows[0][3],
                rows[1][0], rows[1][1], rows[1][2], rows[1][3],
                rows[2][0], rows[2][1], rows[2][2], rows[2][3],
                rows[3][0], rows[3][1], rows[3][2], rows[3][3]);
        }

        Quat ToQuat() const;

        void Decompose(Vec3& outTranslation, Quat& outRotation, Vec3& outScale) const;

        Quat GetRotation() const;

    private:
        /// Function to get determinant of mat[p][q]
        static void GetCofactor(const Matrix4x4& mat, Matrix4x4& cofactor, s32 p, s32 q, s32 n);

        static int GetDeterminant(const Matrix4x4& mat, s32 n);

        static Matrix4x4 GetAdjoint(const Matrix4x4& mat);

        static Matrix4x4 GetInverse(const Matrix4x4& mat);
        
    public:

        Vec4 rows[4];
    };

} // namespace CE

/// fmt user-defined Formatter for CE::Matrix4x4
template <> struct fmt::formatter<CE::Matrix4x4> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        // Return an iterator past the end of the parsed range:
        return ctx.end();
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const CE::Matrix4x4& mat, FormatContext& ctx) const -> decltype(ctx.out()) {
        // ctx.out() is an output iterator to write to.
        return fmt::format_to(ctx.out(), "{}", mat.ToString());
    }
};
