class AverageBase {
public:
	AverageBase(int size, int slots)
			: nSlots(slots),
			  nSamples(0),
			  nCurrentSlot(0) {}

	unsigned char GetNumSamples() {
		return nSamples;
	}

protected:
	unsigned char nSlots;
	unsigned char nSamples;
	unsigned char nCurrentSlot;
};

class Average : public AverageBase {
public:
	Average()
			: AverageBase(4, 0),
			  fAverage(0.0f),
			  pData(NULL),
			  fTotal(0.0f) {}

	Average(int slots)
			: AverageBase(4, slots),
			  fAverage(0.0f),
			  pData(NULL),
			  fTotal(0.0f) {
		Init(slots);
	}

	void Init(int slots) {
		if (pData && pData != SmallDataBuffer) {
			delete[] pData;
			pData = nullptr;
		}
		nSlots = slots;
		pData = SmallDataBuffer;
		if (slots > 5) {
			pData = new float[nSlots];
		}
		memset(pData, 0, 4 * slots);
	}

	void DeInit() {
		if (pData && pData != SmallDataBuffer) {
			delete[] pData;
			pData = nullptr;
		}
	}

	float GetValue() {
		return fAverage;
	}

	float GetTotal() {
		return fTotal;
	}

protected:
	float fTotal;
	float fAverage;
	float *pData;

private:
	float SmallDataBuffer[5];
};

class AverageWindow : public Average {
public:
	AverageWindow(float f_timewindow, float f_frequency)
			: Average(f_timewindow * f_frequency + 0.5f),
			  fTimeWindow(f_timewindow),
			  iOldestValue(0),
			  AllocSize(4 * nSlots) {
		pTimeData = new float[nSlots];
		memset(pTimeData, 0, AllocSize);
	}

	void DeInit() {
		if (pTimeData) {
			delete[] pTimeData;
		}
		Average::DeInit();
	}

	float GetOldestValue() {
		return pData[iOldestValue];
	}

	float GetOldestTimeValue() {
		return pTimeData[iOldestValue];
	}

	void Record(const float fValue, const float fTimeNow) {
		if (pData[nCurrentSlot] == 0.0 && pTimeData[nCurrentSlot] == 0.0) {
			nSamples++;
		}
		else {
			fTotal -= pData[nCurrentSlot];
		}
		fTotal += fValue;
		pData[nCurrentSlot] = fValue;
		pTimeData[nCurrentSlot] = fTimeNow;

		while (fTimeNow - pTimeData[iOldestValue] > fTimeWindow) {
			if (pTimeData[iOldestValue] > 0.0) {
				fTotal -= pData[iOldestValue];
				pData[iOldestValue] = 0.0;
				pTimeData[iOldestValue] = 0.0;
				nSamples--;
			}
			if (++iOldestValue >= nSlots) {
				iOldestValue = 0;
			}
		}

		auto avg = fTotal;
		if (nSamples) {
			avg = fTotal / (double)nSamples;
		}
		fAverage = avg;
		if (++nCurrentSlot >= nSlots) {
			nCurrentSlot = 0;
		}
	}
	void Reset(float fValue) {
		for (int i = 0; i < nSlots; i++) {
			pData[i] = fValue;
			pTimeData[i] = 0;
		}
		fAverage = 0;
		nSamples = 0;
		iOldestValue = 0;
		nCurrentSlot = 0;
		fTotal = nSlots * fValue;
	}

	float fTimeWindow;
	int iOldestValue;
	float *pTimeData;
	unsigned int AllocSize;
};

class Graph {
public:
	Graph(UMath::Vector2 *points, int num_points) {
		Points = points;
		NumPoints = num_points;
	}

	float GetValue(float x) {
		// todo this is entirely guessed, it wasn't in decomp and its pseudocode confusing as hell
		auto curve = Points[0].x;
		for (int i = 0; i < NumPoints; i++) {
			if (x < Points[i].x) break;

			if (i >= NumPoints-1) return curve = Points[i].y;

			float delta = x;
			delta -= Points[i].x;
			delta /= (Points[i+1].x - Points[i].x);
			curve = std::lerp(Points[i].y, Points[i+1].y, delta);
		}
		return curve;
	}

private:
	UMath::Vector2 *Points;
	int NumPoints;
};

template <typename T> struct GraphEntry {
	T x;
	T y;
};

template <typename T> class tGraph {
public:
	tGraph(GraphEntry<T> *data, int num) {
		GraphData = data;
		NumEntries = num;
	}
	void Blend(T *dest, T *a, T *b, const T blend_a);

	// Credits: Brawltendo
	void GetValue(T *pValue, T x) {
		if (NumEntries > 1) {
			if (x <= GraphData[0].x) {
				memcpy(pValue, &GraphData[0].y, sizeof(float));
			} else if (x >= GraphData[NumEntries - 1].x) {
				memcpy(pValue, &GraphData[NumEntries - 1].y, sizeof(float));
			} else {
				for (int i = 0; i < NumEntries - 1; ++i) {
					if (x >= GraphData[i].x && x < GraphData[i + 1].x) {
						const T blend = (x - GraphData[i].x) / (GraphData[i + 1].x - GraphData[i].x);
						Blend(pValue, &GraphData[i + 1].y, &GraphData[i].y, blend);
						return;
					}
				}
			}
		} else if (NumEntries > 0) {
			memcpy(pValue, &GraphData[0].y, sizeof(float));
		}
	}

	float GetValue(T x) {
		float ret;
		GetValue(&ret, x);
		return ret;
	}

private:
	GraphEntry<T> *GraphData;
	int NumEntries;
};

template <> void tGraph<float>::Blend(float *dest, float *a, float *b, const float blend_a) {
	*dest = *a * blend_a + *b * (1.0f - blend_a);
}

class TableBase {
public:
	int NumEntries;
	float MinArg;
	float MaxArg;
	float IndexMultiplier;

	TableBase() {}

	TableBase(int num, float min, float max) {
		NumEntries = num;
		MinArg = min;
		MaxArg = max;
		IndexMultiplier = (NumEntries - 1) / (MaxArg - MinArg);
	}
};

class Table : public TableBase {
public:
	const float* pTable;

	Table(const float *table, int num, float min, float max) : TableBase(num, min, max), pTable(table) {}

	Table(const float* table, int numEntries, float minArg, float maxArg, float indexMultiplier) {
		pTable = table;
		NumEntries = numEntries;
		MinArg = minArg;
		MaxArg = maxArg;
		IndexMultiplier = indexMultiplier;
	}

	// Credits: Brawltendo
	float GetValue(float input) {
		const int entries = NumEntries;
		const float normarg = IndexMultiplier * (input - MinArg);
		const int index = (int)normarg;

		if (index < 0 || normarg < 0.0f)
			return pTable[0];
		if (index >= (entries - 1))
			return pTable[entries - 1];

		float ind = index;
		if (ind > normarg)
			ind -= 1.0f;

		float delta = normarg - ind;
		return (1.0f - delta) * pTable[index] + delta * pTable[index + 1];
	}
};

class AxlePair {
public:
	float Front;
	float Rear;

	float At(int index) const {
		return (&Front)[index];
	}
};

namespace Physics {
	class Tunings {
	public:
		enum Path {
			STEERING = 0,
			HANDLING = 1,
			BRAKES = 2,
			RIDEHEIGHT = 3,
			AERODYNAMICS = 4,
			NOS = 5,
			INDUCTION = 6,
			MAX_TUNINGS = 7,
		};

		float Value[MAX_TUNINGS];
	};
}

class WCollisionTri {
public:
	UMath::Vector3 fPt0;
	UMath::Vector3 fPt1;
	unsigned int fFlags;
	UMath::Vector3 fPt2;
};

class WWorldPos {
public:
	WCollisionTri fFace;
	float fYOffset;
	int fSurface;

	WWorldPos() {
		memset(this,0,sizeof(*this));
		fYOffset = 0.025;
	}

	void SetTolerance(float liftAmount) {
		fYOffset = liftAmount;
	}

	bool Update(UMath::Vector3* pos, UMath::Vector4* dest) {
		auto origin = *pos;
		origin.y += fYOffset;
		auto dir = NyaVec3(0,-1,0);

		tLineOfSightIn prop;
		prop.bIgnoreBackfaces = false;
		prop.bGetClosestHit = true;
		prop.fMaxDistance = 10000;
		tLineOfSightOut out;
		if (CheckLineOfSight(&prop, pGameFlow->pHost->pUnkForLOS, &origin, &dir, &out)) {
			if (out.vHitNormal.y < 0) {
				out.vHitNormal *= -1;
			}
			dest->x = out.vHitNormal.x;
			dest->y = out.vHitNormal.y;
			dest->z = out.vHitNormal.z;
			dest->w = -(out.fHitDistance - fYOffset); // todo is this correct?
			fSurface = out.nSurfaceId;
			return true;
		}
		return false;
	}
};

enum SteeringType {
	kGamePad = 0,
	kWheelSpeedSensitive = 1,
	kWheelSpeedInsensitive = 2,
};

enum GearID {
	G_REVERSE = 0,
	G_NEUTRAL = 1,
	G_FIRST = 2,
	G_SECOND = 3,
	G_THIRD = 4,
	G_FOURTH = 5,
	G_FIFTH = 6,
	G_SIXTH = 7,
	G_SEVENTH = 8,
	G_EIGHTH = 9,
	G_MAX = 10,
};

enum ShiftStatus {
	SHIFT_STATUS_NONE = 0,
	SHIFT_STATUS_NORMAL = 1,
	SHIFT_STATUS_GOOD = 2,
	SHIFT_STATUS_PERFECT = 3,
	SHIFT_STATUS_MISSED = 4,
};

enum ShiftPotential {
	SHIFT_POTENTIAL_NONE = 0,
	SHIFT_POTENTIAL_DOWN = 1,
	SHIFT_POTENTIAL_UP = 2,
	SHIFT_POTENTIAL_GOOD = 3,
	SHIFT_POTENTIAL_PERFECT = 4,
	SHIFT_POTENTIAL_MISS = 5,
};

enum DriverStyle {
	STYLE_RACING = 0,
	STYLE_DRAG = 1,
};

enum DriverClass {
	DRIVER_HUMAN = 0,
	DRIVER_TRAFFIC = 1,
	DRIVER_COP = 2,
	DRIVER_RACER = 3,
	DRIVER_NONE = 4,
	DRIVER_NIS = 5,
	DRIVER_REMOTE = 6,
};

void ConvertWorldToLocal(Car* pCar, UMath::Vector3 &val, bool translate) {
	UMath::Vector4 invorient;

	if (translate) {
		UMath::Sub(val, pCar->GetMatrix()->p, val);
	}
	UMath::Transpose(*pCar->GetQuaternion(), invorient);
	UMath::Rotate(&val, &invorient, &val);
}

namespace Sim {
	float GetTime() {
		return pPlayerHost->nRaceTime / 1000.0;
	}
}

class SimSurface {
public:
	float LATERAL_GRIP = 1.0;
	float DRIVE_GRIP = 1.0;
	float ROLLING_RESISTANCE = 1.0;
};

SimSurface* GetSimSurface(int surfaceType) {
	static SimSurface tmp;
	tmp.LATERAL_GRIP = tmp.DRIVE_GRIP = tmp.ROLLING_RESISTANCE = UMath::Clamp(1.0f / pEnvironment->aSurfaces[surfaceType].fBodyFriction, 0.5f, 1.0f);
	return &tmp;
}