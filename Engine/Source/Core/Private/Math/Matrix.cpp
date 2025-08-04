
#include "Math/Matrix.h"

namespace CE
{
    Matrix4x4 Matrix4x4::PerspectiveProjection(float aspect, float fieldOfView, float n, float f)
    {
		float tanHalfFOV = tan(Math::ToRadians(fieldOfView / 2.0f));

        return Matrix4x4({
			{ 1 / (aspect * tanHalfFOV), 0, 0, 0 },
			{ 0, -1 / tanHalfFOV, 0, 0 },
			{ 0, 0, f / (f - n), -f * n / (f - n) },
			{ 0, 0, 1, 0 }
		});
    }

	Matrix4x4 Matrix4x4::OrthographicProjection(float l, float r, float t, float b, float nearPlane, float farPlane)
	{
		return Matrix4x4({
			{ 2 / (r - l), 0, 0, -(r + l) / (r - l) },
			{ 0, 2 / (b - t), 0, -(b + t) / (b - t) },
			{ 0, 0, 1 / (farPlane - nearPlane), -nearPlane / (farPlane - nearPlane) },
			{ 0, 0, 0, 1 }
		});
	}

	Matrix4x4 Matrix4x4::Angle(f32 degrees)
	{
		return Quat::EulerDegrees(0, 0, degrees).ToMatrix();
	}

	Matrix4x4 Matrix4x4::RotationDegrees(const Vec3& eulerDegrees)
	{
		return Quat::EulerDegrees(eulerDegrees).ToMatrix();
	}

	Matrix4x4 Matrix4x4::RotationRadians(const Vec3& eulerRadians)
	{
		return Quat::EulerRadians(eulerRadians).ToMatrix();
	}

	Matrix4x4 Matrix4x4::operator+(const Matrix4x4& rhs) const
	{
		Matrix4x4 result{};
		
		for (int i = 0; i < 4; i++)
		{
			result.rows[i] = rows[i] + rhs.rows[i];
		}

		return result;
	}

	Matrix4x4 Matrix4x4::operator-(const Matrix4x4& rhs) const
	{
		Matrix4x4 result{};

		for (int i = 0; i < 4; i++)
		{
			result.rows[i] = rows[i] - rhs.rows[i];
		}

		return result;
	}

	Matrix4x4 Matrix4x4::operator*(f32 rhs) const
	{
		ZoneScoped;

		Matrix4x4 result{};

		for (int i = 0; i < 4; i++)
		{
			result.rows[i] *= rhs;
		}

		return result;
	}

	Matrix4x4 Matrix4x4::operator/(f32 rhs) const
	{
		ZoneScoped;

		Matrix4x4 result{};

		for (int i = 0; i < 4; i++)
		{
			result.rows[i] /= rhs;
		}

		return result;
	}

	Matrix4x4 Matrix4x4::Multiply(const Matrix4x4& lhs, const Matrix4x4& rhs)
	{
		ZoneScoped;

		Matrix4x4 result{};

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				result.rows[i][j] = 0;

				for (int k = 0; k < 4; k++) 
				{
					result.rows[i][j] += lhs.rows[i][k] * rhs.rows[k][j];
				}
			}
		}

		return result;
	}

    Vec4 Matrix4x4::Multiply(const Matrix4x4& lhs, const Vec4& rhs)
    {
		ZoneScoped;

        Vec4 result{};
        
        for (int i = 0; i < 4; i++)
        {
            f32 value = 0;
            
            for (int j = 0; j < 4; j++)
            {
                value += lhs.rows[i][j] * rhs[j];
            }
            
            result[i] = value;
        }
        
        return result;
    }

	Matrix4x4 Matrix4x4::GetTranspose(const Matrix4x4& mat)
	{
		ZoneScoped;

		Matrix4x4 result{};

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				result.rows[i][j] = mat.rows[j][i];
			}
		}

		return result;
	}

	Quat Matrix4x4::ToQuat() const
	{
		float trace = rows[0][0] + rows[1][1] + rows[2][2];
		Quat q;

		if (trace > 0.0f)
		{
			float s = std::sqrt(trace + 1.0f) * 2.0f; // S = 4 * qw
			q.w = 0.25f * s;
			q.x = (rows[2][1] - rows[1][2]) / s;
			q.y = (rows[0][2] - rows[2][0]) / s;
			q.z = (rows[1][0] - rows[0][1]) / s;
		}
		else
		{
			if (rows[0][0] > rows[1][1] && rows[0][0] > rows[2][2])
			{
				float s = std::sqrt(1.0f + rows[0][0] - rows[1][1] - rows[2][2]) * 2.0f;
				q.w = (rows[2][1] - rows[1][2]) / s;
				q.x = 0.25f * s;
				q.y = (rows[0][1] + rows[1][0]) / s;
				q.z = (rows[0][2] + rows[2][0]) / s;
			}
			else if (rows[1][1] > rows[2][2])
			{
				float s = std::sqrt(1.0f + rows[1][1] - rows[0][0] - rows[2][2]) * 2.0f;
				q.w = (rows[0][2] - rows[2][0]) / s;
				q.x = (rows[0][1] + rows[1][0]) / s;
				q.y = 0.25f * s;
				q.z = (rows[1][2] + rows[2][1]) / s;
			}
			else
			{
				float s = std::sqrt(1.0f + rows[2][2] - rows[0][0] - rows[1][1]) * 2.0f;
				q.w = (rows[1][0] - rows[0][1]) / s;
				q.x = (rows[0][2] + rows[2][0]) / s;
				q.y = (rows[1][2] + rows[2][1]) / s;
				q.z = 0.25f * s;
			}
		}

		return q.GetNormalized();
	}

	void Matrix4x4::GetCofactor(const Matrix4x4& mat, Matrix4x4& cofactor, s32 p, s32 q, s32 n)
	{
		ZoneScoped;

		s32 i = 0, j = 0;

		for (int row = 0; row < n; row++) 
		{
			for (int col = 0; col < n; col++) 
			{
				if (row != p && col != q) 
				{
					cofactor[i][j++] = mat[row][col];

					// Row is filled, so increase row index and
					// reset col index
					if (j == n - 1) 
					{
						j = 0;
						i++;
					}
				}
			}
		}
	}

	void Matrix4x4::Decompose(Vec3& outTranslation, Quat& outRotation, Vec3& outScale) const
	{
		// 1. Extract translation (4th column)
		outTranslation = Vec3(rows[0][3], rows[1][3], rows[2][3]);

		// 2. Extract basis vectors (scaled axes)
		Vec3 right = Vec3(rows[0][0], rows[0][1], rows[0][2]);
		Vec3 up = Vec3(rows[1][0], rows[1][1], rows[1][2]);
		Vec3 forward = Vec3(rows[2][0], rows[2][1], rows[2][2]);

		// 3. Compute scale from axis lengths
		outScale.x = right.GetMagnitude();
		outScale.y = up.GetMagnitude();
		outScale.z = forward.GetMagnitude();

		// 4. Normalize to remove scale from rotation
		if (outScale.x == 0 || outScale.y == 0 || outScale.z == 0)
		{
			outRotation = Quat(0, 0, 0, 1); // identity
			return;
		}

		right /= outScale.x;
		up /= outScale.y;
		forward /= outScale.z;

		// 5. Reconstruct normalized rotation matrix from Matrix4x4
		float m00 = right.x, m01 = right.y, m02 = right.z;
		float m10 = up.x, m11 = up.y, m12 = up.z;
		float m20 = forward.x, m21 = forward.y, m22 = forward.z;

		float trace = m00 + m11 + m22;
		Quat q;

		if (trace > 0.0f)
		{
			float s = std::sqrt(trace + 1.0f) * 2.0f;
			q.w = 0.25f * s;
			q.x = (m21 - m12) / s;
			q.y = (m02 - m20) / s;
			q.z = (m10 - m01) / s;
		}
		else if (m00 > m11 && m00 > m22)
		{
			float s = std::sqrt(1.0f + m00 - m11 - m22) * 2.0f;
			q.w = (m21 - m12) / s;
			q.x = 0.25f * s;
			q.y = (m01 + m10) / s;
			q.z = (m02 + m20) / s;
		}
		else if (m11 > m22)
		{
			float s = std::sqrt(1.0f + m11 - m00 - m22) * 2.0f;
			q.w = (m02 - m20) / s;
			q.x = (m01 + m10) / s;
			q.y = 0.25f * s;
			q.z = (m12 + m21) / s;
		}
		else
		{
			float s = std::sqrt(1.0f + m22 - m00 - m11) * 2.0f;
			q.w = (m10 - m01) / s;
			q.x = (m02 + m20) / s;
			q.y = (m12 + m21) / s;
			q.z = 0.25f * s;
		}

		outRotation = q.GetNormalized();
	}

	Quat Matrix4x4::GetRotation() const
	{
		Vec3 pos;
		Quat rot;
		Vec3 scale;
		Decompose(pos, rot, scale);
		return rot;
	}

	int Matrix4x4::GetDeterminant(const Matrix4x4& mat, s32 n)
	{
		ZoneScoped;

		int determinant = 0;

		if (n == 1)
			return mat[0][0];

		Matrix4x4 temp{};

		int sign = 1;

		// Iterate for each element of first row
		for (s32 f = 0; f < n; f++) 
		{
			// Getting Cofactor of A[0][f]
			GetCofactor(mat, temp, 0, f, n);
			determinant += sign * mat[0][f] * GetDeterminant(temp, n - 1);

			// terms are to be added with alternate sign
			sign = -sign;
		}

		return determinant;
	}

	Matrix4x4 Matrix4x4::GetAdjoint(const Matrix4x4& mat)
	{
		ZoneScoped;

		Matrix4x4 adj{};

		// temp is used to store cofactors of mat[][]
		Matrix4x4 temp{};

		int sign = 1;

		for (int i = 0; i < 4; i++) 
		{
			for (int j = 0; j < 4; j++) 
			{
				// Get cofactor of A[i][j]
				GetCofactor(mat, temp, i, j, 4);

				// sign of adj[j][i] positive if sum of row
				// and column indexes is even.
				sign = ((i + j) % 2 == 0) ? 1 : -1;

				// Interchanging rows and columns to get the
				// transpose of the cofactor matrix
				adj[j][i] = (sign) * (GetDeterminant(temp, 4 - 1));
			}
		}

		return adj;
	}

	Matrix4x4 Matrix4x4::GetInverse(const Matrix4x4& m)
	{
		ZoneScoped;

		float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
		float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
		float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
		float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

		float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
		float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
		float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
		float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

		float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
		float idet = 1.0f / det;

		Matrix4x4 ret;

		ret[0][0] = t11 * idet;
		ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
		ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
		ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

		ret[1][0] = t12 * idet;
		ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
		ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
		ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

		ret[2][0] = t13 * idet;
		ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
		ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
		ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

		ret[3][0] = t14 * idet;
		ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
		ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
		ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

		return ret;
	}


} // namespace CE

