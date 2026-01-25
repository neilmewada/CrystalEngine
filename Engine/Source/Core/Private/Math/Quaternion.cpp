
#include "Math/Vector.h"
#include "Math/Quaternion.h"


namespace CE
{

	Quat::Quat() : w(1), x(0), y(0), z(0)
	{
	}

	Quat::Quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z)
	{
	}

	Quat::Quat(const Vec4& vec4) : w(vec4.w), x(vec4.x), y(vec4.y), z(vec4.z)
	{
	}

	Quat::Quat(const Quat& q)
	{
		w = q.w;
		x = q.x;
		y = q.y;
		z = q.z;
	}

	Quat::~Quat()
	{
	}

	Quat& Quat::operator=(const Quat& q)
	{
		if (this == &q)
			return *this;
		w = q.w;
		x = q.x;
		y = q.y;
		z = q.z;
		return *this;
	}

	bool Quat::operator==(const Quat& rhs) const
	{
		return w == rhs.w && x == rhs.x && y == rhs.y && z == rhs.z;
	}

	bool Quat::operator!=(const Quat& rhs) const
	{
		return !(*this == rhs);
	}

	Quat& Quat::operator*=(const Quat& rhs)
	{
		Quat q;

		q.w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
		q.x = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
		q.y = w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x;
		q.z = w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w;

		*this = q;
		return *this;
	}

	Quat Quat::operator*(const Quat& rhs) const
	{
		Quat q;

		q.w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
		q.x = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
		q.y = w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x;
		q.z = w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w;

		return q;
	}

	Quat Quat::operator*(float rhs) const
	{
		return Quat(w * rhs, x * rhs, y * rhs, z * rhs);
	}

	Quat Quat::operator+(const Quat& rhs) const
	{
		return Quat(w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Quat Quat::operator-(const Quat& rhs) const
	{
		return Quat(w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z);
	}

	Vec4 Quat::operator*(const Vec4& rhs) const
	{
		return Quat::ToMatrix(*this) * rhs;
	}

	Vec3 Quat::operator*(const Vec3& rhs) const
	{
		return Quat::ToMatrix(*this) * Vec4(rhs, 1.0f);
	}

	Quat Quat::GetNormalized()
	{
		float mag = sqrtf(w * w + x * x + y * y + z * z);
		if (mag == 0.0f)
			return Quat(0, 0, 0, 1); // Identity quaternion fallback

		return Quat(w / mag, x / mag, y / mag, z / mag);
	}

	void Quat::Normalize()
	{
		float mag = sqrtf(w * w + x * x + y * y + z * z);

		w /= mag;
		x /= mag;
		y /= mag;
		z /= mag;
	}

	Quat Quat::Conjugate()
	{
		return Quat(w, -x, -y, -z);
	}

	void Quat::SetEulerDegrees(float x, float y, float z)
	{
		x *= 0.0174532925f; // To radians!
		y *= 0.0174532925f; // To radians!
		z *= 0.0174532925f; // To radians!

		x *= 0.5f;
		y *= 0.5f;
		z *= 0.5f;
		
		float sinx = sinf(x);
		float siny = sinf(y);
		float sinz = sinf(z);
		float cosx = cosf(x);
		float cosy = cosf(y);
		float cosz = cosf(z);

		w = cosx * cosy * cosz + sinx * siny * sinz;
		this->x = sinx * cosy * cosz + cosx * siny * sinz;
		this->y = cosx * siny * cosz - sinx * cosy * sinz;
		this->z = cosx * cosy * sinz - sinx * siny * cosz;
	}

	float Quat::Dot(const Quat& b) const
	{
		return (w * b.w + x * b.x * y * b.y + z * b.z);
	}

	Matrix4x4 Quat::ToMatrix() const
	{
		return ToMatrix(*this);
	}

	Quat Quat::FromToRotation(const Vec3& from, const Vec3& to)
	{
		const Vec3 unitFrom = from.GetNormalized();
		const Vec3 unitTo = to.GetNormalized();
		const float dot = Vec3::Dot(unitFrom, unitTo);

		Vec3 cross = Vec3::Cross(unitFrom, unitTo);
		float w = sqrt((1 + dot) * 0.5);
		float k = sqrt((1 - dot) * 0.5);
		float scale = (w > 0) ? (1.0 / w) : 0;

		if (dot >= 1.0f)
		{
			return Quat(); // identity
		}
		else if (dot <= -1.0f)
		{
			// If the two vectors are pointing in opposite directions then we
			// need to supply a quaternion corresponding to a rotation of
			// PI-radians about an axis orthogonal to the fromDirection.
			Vec3 axis = Vec3::Cross(unitFrom, Vec3(1, 0, 0));
			if (axis.GetSqrMagnitude() < 1e-6)
			{
				// Bad luck. The x-axis and fromDirection are linearly
				// dependent (colinear). We'll take the axis as the vector
				// orthogonal to both the y-axis and fromDirection instead.
				// The y-axis and fromDirection will clearly not be linearly
				// dependent.
				axis = Vec3::Cross(unitFrom, Vec3(0, 1, 0));
			}

			// Note that we need to normalize the axis as the cross product of
			// two unit vectors is not nececessarily a unit vector.
			return Quat::AngleAxis(180, axis.GetNormalized()); // 180 Degrees = PI radians
		}
		else
		{
			// Scalar component.
			const float s = sqrt(unitFrom.GetSqrMagnitude() * unitTo.GetSqrMagnitude())
				+ Vec3::Dot(unitFrom, unitTo);

			// Vector component.
			Vec3 v = Vec3::Cross(unitFrom, unitTo);
			v.x *= -1;
			v.y *= -1;

			// Return the normalized quaternion rotation.
			return Quat(Vec4(v, s)).GetNormalized();
		}
	}

	Quat Quat::LookRotation(const Vec3& lookAt)
	{
		return Quat::FromToRotation(Vec3(0, 0, 1), lookAt);
	}

	// TODO: Modify LookRotation entries after modifying the original function
	Quat Quat::LookRotation(const Vec3& lookAt, const Vec3& upDirection)
	{
		// Calculate the unit quaternion that rotates Vector3::FORWARD to face
		// in the specified forward direction.
		const Quat q1 = Quat::LookRotation(lookAt);

		// We can't preserve the upwards direction if the forward and upwards
		// vectors are linearly dependent (colinear).
		if (Vec3::Cross(lookAt, upDirection).GetSqrMagnitude() < 1e-6)
		{
			return q1;
		}

		// Determine the upwards direction obtained after applying q1.
		const Vec3 newUp = q1 * Vec3(0, 1, 0);

		// Calculate the unit quaternion rotation that rotates the newUp
		// direction to look in the specified upward direction.
		const Quat q2 = Quat::FromToRotation(newUp, upDirection);

		// Return the combined rotation so that we first rotate to look in the
		// forward direction and then rotate to align Vector3::UPWARD with the
		// specified upward direction. There is no need to normalize the result
		// as both q1 and q2 are unit quaternions.
		return q2 * q1;
	}

	Quat Quat::LookRotation2(const Vec3& forward, const Vec3& up)
	{
		Vec3 right = Vec3::Cross(up, forward);
		Quat result;
		result.w = sqrtf(1.0f + right.x + up.y + forward.z) * 0.5f;
		float w4_recip = 1.0f / (4.0f * result.w);
		result.x = (forward.y - up.z) * w4_recip;
		result.y = (right.z - forward.x) * w4_recip;
		result.z = (up.x - right.y) * w4_recip;
		return result;
	}

	Quat Quat::Slerp(const Quat& from, const Quat& to, float t)
	{
		float cosTheta = Quat::Dot(from, to);
		Quat temp(to);

		if (cosTheta < 0.0f) {
			cosTheta *= -1.0f;
			temp = temp * -1.0f;
		}

		float theta = acosf(cosTheta);
		float sinTheta = 1.0f / sinf(theta);

		return sinTheta * (
			((Quat)(from * sinf(theta * (1.0f - t)))) +
			((Quat)(temp * sinf(t * theta)))
			);
	}

	Quat Quat::Lerp(const Quat& from, const Quat& to, float t)
	{
		Quat src = from * (1.0f - t);
		Quat dst = to * t;

		Quat q = src + dst;
		return q;
	}

	float Quat::Angle(const Quat& a, const Quat& b)
	{
		float degrees = acosf((b * a.GetInversed()).w) * 2.0f * 57.2957795f;
		if (degrees > 180.0f)
			return 360.0f - degrees;
		return degrees;
	}

	float Quat::Dot(const Quat& a, const Quat& b)
	{
		return a.Dot(b);
	}

	Quat Quat::AngleAxis(float angle, const Vec4& axis)
	{
		Vec4 vn = axis.GetNormalized();

		angle *= 0.0174532925f; // To radians!
		angle *= 0.5f;
		float sinAngle = sin(angle);

		return Quat(cos(angle), vn.x * sinAngle, vn.y * sinAngle, vn.z * sinAngle);
	}

	Quat Quat::GetInversed() const
	{
		const float sqr = GetSqrMagnitude();

		const float invSqr = 1.0f / sqr;

		return Quat(w * invSqr, -x * invSqr, -y * invSqr, -z * invSqr);
	}

	Quat Quat::EulerDegrees(float X, float Y, float Z)
	{
		X *= 0.0174532925f; // To radians!
		Y *= 0.0174532925f; // To radians!
		Z *= 0.0174532925f; // To radians!

		return EulerRadians(X, Y, Z);
	}

	Quat Quat::EulerRadians(float X, float Y, float Z)
	{
		// NEW
		f32 cy = cos(Z * 0.5);
		f32 sy = sin(Z * 0.5);
		f32 cp = cos(Y * 0.5);
		f32 sp = sin(Y * 0.5);
		f32 cr = cos(X * 0.5);
		f32 sr = sin(X * 0.5);

		f32 qw = cr * cp * cy + sr * sp * sy;
		f32 qx = -sr * cp * cy + cr * sp * sy;
		f32 qy = cr * sp * cy + sr * cp * sy;
		f32 qz = cr * cp * sy - sr * sp * cy;

		//return Quat(qw, qx, qy, qz);

		// OLD
		X *= 0.5f;
		Y *= 0.5f;
		Z *= 0.5f;

		float sinx = sinf(X);
		float siny = sinf(Y);
		float sinz = sinf(Z);
		float cosx = cosf(X);
		float cosy = cosf(Y);
		float cosz = cosf(Z);

		Quat q;

		q.w = cosx * cosy * cosz + sinx * siny * sinz;
		q.x = sinx * cosy * cosz + cosx * siny * sinz;
		q.y = cosx * siny * cosz - sinx * cosy * sinz;
		q.z = cosx * cosy * sinz - sinx * siny * cosz;

		return q;
	}

	Matrix4x4 Quat::ToMatrix(const Quat& q)
	{
		ZoneScoped;

		float sqw = q.w * q.w;
		float sqx = q.x * q.x;
		float sqy = q.y * q.y;
		float sqz = q.z * q.z;
		float invs = 1.0f / (sqx + sqy + sqz + sqw);

		Matrix4x4 matrix = Matrix4x4::Identity();

		f32* mat = &matrix.rows[0][0];

		mat[0] = (sqx - sqy - sqz + sqw) * invs;
		mat[5] = (-sqx + sqy - sqz + sqw) * invs;
		mat[10] = (-sqx - sqy + sqz + sqw) * invs;

		float tmp1 = q.x * q.y;
		float tmp2 = q.z * q.w;
		mat[4] = 2.0 * (tmp1 + tmp2) * invs;
		mat[1] = 2.0 * (tmp1 - tmp2) * invs;

		tmp1 = q.x * q.z;
		tmp2 = q.y * q.w;
		mat[8] = 2.0 * (tmp1 - tmp2) * invs;
		mat[2] = 2.0 * (tmp1 + tmp2) * invs;

		tmp1 = q.y * q.z;
		tmp2 = q.x * q.w;
		mat[9] = 2.0 * (tmp1 + tmp2) * invs;
		mat[6] = 2.0 * (tmp1 - tmp2) * invs;

		return matrix;
	}

	Vec3 Quat::ToEulerRadians() const
	{
		//const Quat& q = *this;

		double sqw = w * w;
		double sqx = x * x;
		double sqy = y * y;
		double sqz = z * z;
		double unit = sqx + sqy + sqz + sqw;
		double test = x * y + z * w;
		Vec3 euler;

		if (test > 0.499 * unit) { // Singularity at north pole
			euler.y = 2 * atan2(x, w);
			euler.x = M_PI / 2;
			euler.z = 0;
			return euler;
		}
		if (test < -0.499 * unit) { // Singularity at south pole
			euler.y = -2 * atan2(x, w);
			euler.x = -M_PI / 2;
			euler.z = 0;
			return euler;
		}
		euler.y = atan2(2 * y * w + 2 * x * z, sqw - sqx - sqy + sqz); // Yaw
		euler.x = asin(-2 * (x * z - y * w) / unit); // Pitch
		euler.z = atan2(2 * x * w + 2 * y * z, sqw + sqx - sqy - sqz); // Roll

		return euler;
	}

	Vec3 Quat::ToEulerDegrees() const
	{
		return ToEulerRadians() * RAD_TO_DEG;
	}

	Vec4 operator*(const Vec4& v, const Quat& m)
	{
		return Quat::ToMatrix(m) * v;
	}

	Vec3 operator*(const Vec3& v, const Quat& m)
	{
		return Quat::ToMatrix(m) * Vec4(v, 1.0f);
	}

	Quat operator*(float f, const Quat& m)
	{
		return Quat(m.w * f, m.x * f, m.y * f, m.z * f);
	}

} // namespace CE
