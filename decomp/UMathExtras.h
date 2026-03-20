#define FLOAT_EPSILON 0.000001f

namespace VehicleSystem {
	float ENABLE_ROLL_STOPS_THRESHOLD = 0.2f;
};

inline bool IsFront(unsigned int i) {
	return i < 2;
}

inline bool IsRear(unsigned int i) {
	return i > 1;
}

inline int bClamp(int a, int MINIMUM, int MAXIMUM) {
	return std::min(std::max(a, MINIMUM), MAXIMUM);
}

inline float bClamp(float a, float MINIMUM, float MAXIMUM) {
	return std::min(MAXIMUM, std::max(a, MINIMUM));
}

namespace UMath {
	class Vector2 {
	public:
		float x, y;
	};

	class Vector3 : public NyaVec3 {
	public:
		using NyaVec3::NyaVec3;

		Vector3(const NyaVec3& v) : NyaVec3(v) {}
	};

	class Vector4 : public NyaVec4 {
	public:
		using NyaVec4::NyaVec4;

		Vector4(const NyaVec4& v) : NyaVec4(v) {}
	};

	class Matrix4 : public NyaMat4x4 {
	public:
		using NyaMat4x4::NyaMat4x4;

		Matrix4(const NyaMat4x4& m) : NyaMat4x4(m) {}
	};

	inline void Transpose(UMath::Matrix4* m, UMath::Matrix4 &result) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result.GetVector(i)[j] = m->GetVector(j)[i];
			}
		}
	}

	inline void Transpose(const UMath::Vector4 &a, UMath::Vector4 &result) {
		result.x = -a.x;
		result.y = -a.y;
		result.z = -a.z;
		result.w = a.w;
	}

	void Rotate(const UMath::Vector3 *v, const UMath::Vector4 *q, UMath::Vector3 *result) {
		double v3; // fp0
		double v4; // fp5
		double v5; // fp10
		double v6; // fp13

		v3 = ((q->x * ((v->z * q->w) + ((q->x * v->y) - (v->x * q->y))))
			  - (q->z * ((v->x * q->w) + ((q->y * v->z) - (q->z * v->y)))));
		v4 = ((q->y * ((v->x * q->w) + ((q->y * v->z) - (q->z * v->y))))
			  - (q->x * ((v->y * q->w) + ((v->x * q->z) - (q->x * v->z)))));
		v5 = ((q->w * ((v->y * q->w) + ((v->x * q->z) - (q->x * v->z))))
			  + (q->y * ((q->x * v->x) + ((q->z * v->z) + (q->y * v->y)))));
		v6 = ((q->w * ((v->z * q->w) + ((q->x * v->y) - (v->x * q->y))))
			  + (q->z * ((q->x * v->x) + ((q->z * v->z) + (q->y * v->y)))));
		result->x = ((((v->x * q->w) + ((q->y * v->z) - (q->z * v->y))) * q->w)
					 + (q->x * ((q->x * v->x) + ((q->z * v->z) + (q->y * v->y)))))
					- ((q->z * ((v->y * q->w) + ((v->x * q->z) - (q->x * v->z))))
					   - (q->y * ((v->z * q->w) + ((q->x * v->y) - (v->x * q->y)))));
		result->y = v5 - v3;
		result->z = v6 - v4;
	}

	inline void Mult(const UMath::Matrix4 m1, const UMath::Matrix4 m2, UMath::Matrix4 &result) {
		for (int j = 0; j < 4; ++j) {
			result.x[j] = m1.x[0] * m2.x[j] + m1.x[1] * m2.y[j] + m1.x[2] * m2.z[j] + m1.x[3] * m2.p[j];
			result.y[j] = m1.y[0] * m2.x[j] + m1.y[1] * m2.y[j] + m1.y[2] * m2.z[j] + m1.y[3] * m2.p[j];
			result.z[j] = m1.z[0] * m2.x[j] + m1.z[1] * m2.y[j] + m1.z[2] * m2.z[j] + m1.z[3] * m2.p[j];
			result.p[j] = m1.p[0] * m2.x[j] + m1.p[1] * m2.y[j] + m1.p[2] * m2.z[j] + m1.p[3] * m2.p[j];
		}
	}

	inline void Mult(const UMath::Vector4* b, const UMath::Vector4* a, UMath::Vector4 &dest) {
		dest.x = a->y * b->z - a->z * b->y + a->w * b->x + a->x * b->w;
		dest.y = a->z * b->x - a->x * b->z + a->w * b->y + a->y * b->w;
		dest.z = a->x * b->y - a->y * b->x + a->w * b->z + a->z * b->w;
		dest.w = a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z;
	}

	inline Vector4 Vector4Make(const Vector3 &c, float w) {
		Vector4 res;
		res.x = c.x;
		res.y = c.y;
		res.z = c.z;
		res.w = w;
		return res;
	}

	inline Vector3 Vector4To3(const Vector4 &c) {
		return {c.x,c.y,c.z};
	}

	float Abs(float f) { return std::abs(f); }
	float Min(float a, float b) { return std::min(a, b); }
	float Max(float a, float b) { return std::max(a, b); }
	float Lerp(float a, float b, float c) { return std::lerp(a, b, c); }
	float Sina(float a) { return std::sin(a * (std::numbers::pi*2)); }
	float Sqrt(float a) { return std::sqrt(a); }
	float Pow(float a, float b) { return std::pow(a, b); }
	float Pow(int a, int b) { return std::pow(a, b); }
	float Atan2a(float a, float b) { return std::atan2(a, b) / (std::numbers::pi*2); }

	inline void Cross(Vector3 a, Vector3 b, Vector3 &r) {
		r.x = a.y * b.z - a.z * b.y;
		r.y = a.z * b.x - a.x * b.z;
		r.z = a.x * b.y - a.y * b.x;
	}

	inline float Atan2d(float o, float a) {
		return ANGLE2DEG(Atan2a(o, a));
	}

	inline void RotateTranslate(Vector3 v, Matrix4 m, Vector3 &result) {
		result.x = ((m.x.x * v.x) + ((m.z.x * v.z) + (m.y.x * v.y))) + m.p.x;
		result.y = ((m.x.y * v.x) + ((m.z.y * v.z) + (m.y.y * v.y))) + m.p.y;
		result.z = ((m.x.z * v.x) + ((m.z.z * v.z) + (m.y.z * v.y))) + m.p.z;
	}

	inline void Unit(Vector3 a, Vector3 &r) {
		auto len = a.length();
		if (len != 0.0) {
			r.x = a.x / len;
			r.y = a.y / len;
			r.z = a.z / len;
		}
		else {
			r = {0,0,0};
		}
	}

	void UnitCross(Vector3 a, Vector3 b, Vector3 &r) {
		r.x = a.y * b.z - a.z * b.y;
		r.y = a.z * b.x - a.x * b.z;
		r.z = a.x * b.y - a.y * b.x;
		Unit(r, r);
	}

	int Clamp(const int a, const int amin, const int amax) {
		return a < amin ? amin : (a > amax ? amax : a);
	}

	float Clamp(const float a, const float amin, const float amax) {
		return a < amin ? amin : (a > amax ? amax : a);
	}

	inline float LengthSquare(const Vector3 &a) {
		return a.x * a.x + a.y * a.y + a.z * a.z;
	}

	inline float Length(const Vector3 &a) {
		return std::sqrt(LengthSquare(a));
	}

	inline float Lengthxz(const Vector3 &a) {
		auto tmp = a;
		tmp.y = 0;
		return tmp.length();
	}

	inline void Scale(const Vector3 &a, const Vector3 &b, Vector3 &r) {
		r.x = a.x * b.x;
		r.y = a.y * b.y;
		r.z = a.z * b.z;
	}

	inline void Scale(const Vector3 &a, const float s, Vector3 &r) {
		r.x = a.x * s;
		r.y = a.y * s;
		r.z = a.z * s;
	}

	inline float Dot(const Vector3 &a, const Vector3 &b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	inline void Rotate(Vector3 a, Matrix4 m, Vector3 &r) {
		r.x = m.x.x * a.x + m.y.x * a.y + m.z.x * a.z;
		r.y = m.x.y * a.x + m.y.y * a.y + m.z.y * a.z;
		r.z = m.x.z * a.x + m.y.z * a.y + m.z.z * a.z;
	}

	inline void Add(const Vector3 &a, const Vector3 &b, Vector3 &r) {
		r.x = a.x + b.x;
		r.y = a.y + b.y;
		r.z = a.z + b.z;
	}

	inline void Sub(const Vector3 &a, const Vector3 &b, Vector3 &r) {
		r.x = a.x - b.x;
		r.y = a.y - b.y;
		r.z = a.z - b.z;
	}

	// NOTE this was wrong in the decomp!
	inline void ScaleAdd(const Vector3 &a, const float s, const Vector3 &b, Vector3 &r) {
		r.x = s * a.x + b.x;
		r.y = s * a.y + b.y;
		r.z = s * a.z + b.z;
	}

	inline float Ramp(const float a, const float amin, const float amax) {
		float arange = amax - amin;
		return arange > FLOAT_EPSILON ? std::max(0.0f, std::min((a - amin) / arange, 1.0f)) : 0.0f;
	}

	// Credits: Brawltendo
	inline float Limit(const float a, const float l) {
		float retval;
		if (!(a * l > 0.f)) {
			retval = a;
		} else {
			if (a > 0.f) {
				retval = Min(a, l);

			} else {
				retval = Max(a, l);
			}
		}
		return retval;
	}
}

UMath::Vector3 CalculateInertiaTensor(UMath::Vector3 tensorScale, float mass, UMath::Vector3 dim) {
	UMath::Vector3 out;
	out.x = ((dim.z * 2.0 * dim.z * 2.0) + (dim.y * 2.0 * dim.y * 2.0)) * 0.083333336 * mass * tensorScale.x;
	out.y = ((dim.z * 2.0 * dim.z * 2.0) + (dim.x * 2.0 * dim.x * 2.0)) * 0.083333336 * mass * tensorScale.y;
	out.z = ((dim.y * 2.0 * dim.y * 2.0) + (dim.x * 2.0 * dim.x * 2.0)) * 0.083333336 * mass * tensorScale.z;
	return out;
}

UMath::Matrix4 GetInverseWorldTensor(UMath::Vector3 inertiaTensor, UMath::Matrix4 orientation) {
	if (inertiaTensor.x > 0.000001) {
		inertiaTensor.x = (1.0 / inertiaTensor.x);
	}
	if (inertiaTensor.y > 0.000001) {
		inertiaTensor.y = (1.0 / inertiaTensor.y);
	}
	if (inertiaTensor.z > 0.000001) {
		inertiaTensor.z = (1.0 / inertiaTensor.z);
	}

	UMath::Matrix4 out = {};
	out.x.x = inertiaTensor.x;
	out.y.y = inertiaTensor.y;
	out.z.z = inertiaTensor.z;
	out.pw = 1.0;

	UMath::Matrix4 v8;
	UMath::Matrix4 v9;
	UMath::Transpose(&orientation, v9);
	UMath::Mult(out, orientation, v8);
	UMath::Mult(v9, v8, out);
	return out;
}