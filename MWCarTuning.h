#define TUNED_VALUE(value) value = std::lerp(a.value, b.value, f);
#define TUNED_AXLEPAIR(value) value.Front = std::lerp(a.value.Front, b.value.Front, f); value.Rear = std::lerp(a.value.Rear, b.value.Rear, f);
#define TOML_VALUE(category, value) value = config[category][#value].value_or(-0.011f); if (value == 0.011f) { MessageBoxA(nullptr, std::format("Failed to find value for {}", #value).c_str(), "nya?!~", MB_ICONERROR); }
#define TOML_AXLEPAIR(category, value) value.Front = config[category][#value][0].value_or(-0.011f); value.Rear = config[category][#value][1].value_or(-0.011f); if (value.Front == 0.011f) { MessageBoxA(nullptr, std::format("Failed to find value for {}", #value).c_str(), "nya?!~", MB_ICONERROR); }
#define TOML_VECTOR(category, value) ReadTOMLVector(config, value, category, #value);
#define TOML_ARRAY(category, value) ReadTOMLArray(config, value, category, #value);

struct MWCarDataBase {
	static void TUNED_VECTOR(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& out, float f) {
		// don't care about lengths on none or full tune
		if (f <= 0.0) {
			out = a;
			return;
		}
		if (f >= 1.0) {
			out = b;
			return;
		}

		for (int i = 0; i < a.size() && i < b.size(); i++) {
			out.push_back(std::lerp(a[i], b[i], f));
		}
	}

	template<size_t size>
	static void ReadTOMLArray(const toml::table& config, float (&out)[size], const char* category, const char* name) {
		for (int i = 0; i < size; i++) {
			out[i] = config[category][name][i].value_or(-0.011f);
			if (out[i] == 0.011f) {
				MessageBoxA(nullptr, std::format("Failed to find value for {}[{}]", name, i).c_str(), "nya?!~", MB_ICONERROR);
			}
		}
	}

	static void ReadTOMLVector(const toml::table& config, std::vector<float>& out, const char* category, const char* name) {
		out.clear();
		for (int i = 0; i < 1024; i++) {
			float f = config[category][name][i].value_or(-0.011f);
			if (f == -0.011f) {
				if (i == 0 && strcmp(name, "SPEED_LIMITER")) { // speed limiter is optional
					MessageBoxA(nullptr, std::format("Failed to find value for {}", name).c_str(), "nya?!~", MB_ICONERROR);
				}
				break;
			}
			out.push_back(f);
		}
	}

	class Brakes {
	public:
		AxlePair BRAKE_LOCK;
		AxlePair BRAKES;
		float EBRAKE;

		Brakes() {}
		Brakes(const toml::table& config, const char* category) {
			TOML_AXLEPAIR(category, BRAKE_LOCK);
			TOML_AXLEPAIR(category, BRAKES);
			TOML_VALUE(category, EBRAKE);
		}
		Brakes(const Brakes& a, const Brakes& b, float f) {
			TUNED_AXLEPAIR(BRAKE_LOCK);
			TUNED_AXLEPAIR(BRAKES);
			TUNED_VALUE(EBRAKE);
		}
		Brakes(const std::vector<Brakes>& vec, float f) {
			if (vec.empty()) __debugbreak();

			if (vec.size() == 1) *this = vec[0];
			else {
				*this = Brakes(vec[0], vec[vec.size()-1], f);
			}
		}
	};

	struct Chassis {
		float AERO_CG;
		float AERO_COEFFICIENT;
		float DRAG_COEFFICIENT;
		float FRONT_AXLE;
		float FRONT_WEIGHT_BIAS;
		float RENDER_MOTION;
		AxlePair RIDE_HEIGHT;
		float ROLL_CENTER;
		float SHOCK_BLOWOUT;
		AxlePair SHOCK_DIGRESSION;
		AxlePair SHOCK_EXT_STIFFNESS;
		AxlePair SHOCK_STIFFNESS;
		AxlePair SHOCK_VALVING;
		AxlePair SPRING_PROGRESSION;
		AxlePair SPRING_STIFFNESS;
		AxlePair SWAYBAR_STIFFNESS;
		AxlePair TRACK_WIDTH;
		AxlePair TRAVEL;
		float WHEEL_BASE;

		Chassis() {}
		Chassis(const toml::table& config, const char* category) {
			TOML_VALUE(category, AERO_CG);
			TOML_VALUE(category, AERO_COEFFICIENT);
			TOML_VALUE(category, DRAG_COEFFICIENT);
			TOML_VALUE(category, FRONT_AXLE);
			TOML_VALUE(category, FRONT_WEIGHT_BIAS);
			TOML_VALUE(category, RENDER_MOTION);
			TOML_AXLEPAIR(category, RIDE_HEIGHT);
			TOML_VALUE(category, ROLL_CENTER);
			TOML_VALUE(category, SHOCK_BLOWOUT);
			TOML_AXLEPAIR(category, SHOCK_DIGRESSION);
			TOML_AXLEPAIR(category, SHOCK_EXT_STIFFNESS);
			TOML_AXLEPAIR(category, SHOCK_STIFFNESS);
			TOML_AXLEPAIR(category, SHOCK_VALVING);
			TOML_AXLEPAIR(category, SPRING_PROGRESSION);
			TOML_AXLEPAIR(category, SPRING_STIFFNESS);
			TOML_AXLEPAIR(category, SWAYBAR_STIFFNESS);
			TOML_AXLEPAIR(category, TRACK_WIDTH);
			TOML_AXLEPAIR(category, TRAVEL);
			TOML_VALUE(category, WHEEL_BASE);

			// nasty hack to make World cars not have broken suspensions, this is inaccurate but it'll do for now
			if (SWAYBAR_STIFFNESS.Front < 100.0 && SWAYBAR_STIFFNESS.Rear < 100.0) {
				SPRING_PROGRESSION.Front *= 0.1;
				SPRING_PROGRESSION.Rear *= 0.1;
				SPRING_STIFFNESS.Front *= 10;
				SPRING_STIFFNESS.Rear *= 10;
				SWAYBAR_STIFFNESS.Front *= 10;
				SWAYBAR_STIFFNESS.Rear *= 10;
			}
		}
		Chassis(const Chassis& a, const Chassis& b, float f) {
			TUNED_VALUE(AERO_CG);
			TUNED_VALUE(AERO_COEFFICIENT);
			TUNED_VALUE(DRAG_COEFFICIENT);
			TUNED_VALUE(FRONT_AXLE);
			TUNED_VALUE(FRONT_WEIGHT_BIAS);
			TUNED_VALUE(RENDER_MOTION);
			TUNED_AXLEPAIR(RIDE_HEIGHT);
			TUNED_VALUE(ROLL_CENTER);
			TUNED_VALUE(SHOCK_BLOWOUT);
			TUNED_AXLEPAIR(SHOCK_DIGRESSION);
			TUNED_AXLEPAIR(SHOCK_EXT_STIFFNESS);
			TUNED_AXLEPAIR(SHOCK_STIFFNESS);
			TUNED_AXLEPAIR(SHOCK_VALVING);
			TUNED_AXLEPAIR(SPRING_PROGRESSION);
			TUNED_AXLEPAIR(SPRING_STIFFNESS);
			TUNED_AXLEPAIR(SWAYBAR_STIFFNESS);
			TUNED_AXLEPAIR(TRACK_WIDTH);
			TUNED_AXLEPAIR(TRAVEL);
			TUNED_VALUE(WHEEL_BASE);
		}
		Chassis(const std::vector<Chassis>& vec, float f) {
			if (vec.empty()) __debugbreak();

			if (vec.size() == 1) *this = vec[0];
			else {
				*this = Chassis(vec[0], vec[vec.size()-1], f);

				// this is how world calculates it
				//else if (vec.size() == 4) {
				//	float h = f * 100;
				//	float a = f * 100;
				//	float t = f * 100;
				//
				//	auto sNode = vec[0];
				//	auto hNode = vec[1];
				//	auto aNode = vec[2];
				//	auto tNode = vec[3];
				//	ROLL_CENTER = (1.5 * (h*hNode.ROLL_CENTER + a*aNode.ROLL_CENTER + t*tNode.ROLL_CENTER) + sNode.ROLL_CENTER * (150 - 0.5 * (h + a + t))) / (h + a + t + 150);
				//}
			}
		}
	};

	struct Engine {
		std::vector<float> ENGINE_BRAKING;
		float FLYWHEEL_MASS;
		float IDLE;
		float MAX_RPM;
		float RED_LINE;
		float SPEED_LIMITER[2];
		std::vector<float> TORQUE;

		Engine() {}
		Engine(const toml::table& config, const char* category) {
			TOML_VECTOR(category, ENGINE_BRAKING);
			TOML_VALUE(category, FLYWHEEL_MASS);
			TOML_VALUE(category, IDLE);
			TOML_VALUE(category, MAX_RPM);
			TOML_VALUE(category, RED_LINE);
			TOML_ARRAY(category, SPEED_LIMITER);
			TOML_VECTOR(category, TORQUE);
		}
		Engine(const Engine& a, const Engine& b, float f) {
			TUNED_VECTOR(a.ENGINE_BRAKING, b.ENGINE_BRAKING, ENGINE_BRAKING, f);
			TUNED_VALUE(FLYWHEEL_MASS);
			TUNED_VALUE(IDLE);
			TUNED_VALUE(MAX_RPM);
			TUNED_VALUE(RED_LINE);
			for (int i = 0; i < 2; i++) {
				TUNED_VALUE(SPEED_LIMITER[i]);
			}
			TUNED_VECTOR(a.TORQUE, b.TORQUE, TORQUE, f);
		}
		Engine(const std::vector<Engine>& vec, float f) {
			if (vec.empty()) __debugbreak();

			if (vec.size() == 1) *this = vec[0];
			else {
				*this = Engine(vec[0], vec[vec.size()-1], f);
			}
		}
	};

	struct Induction {
		float HIGH_BOOST;
		float LOW_BOOST;
		float PSI;
		float SPOOL;
		float SPOOL_TIME_DOWN;
		float SPOOL_TIME_UP;
		float VACUUM;

		Induction() {
			HIGH_BOOST = 0.0f;
			LOW_BOOST = 0.0f;
			PSI = 0.0f;
			SPOOL = 0.0f;
			SPOOL_TIME_DOWN = 0.0f;
			SPOOL_TIME_UP = 0.0f;
			VACUUM = 0.0f;
		}
		Induction(const toml::table& config, const char* category) {
			TOML_VALUE(category, HIGH_BOOST);
			TOML_VALUE(category, LOW_BOOST);
			TOML_VALUE(category, PSI);
			TOML_VALUE(category, SPOOL);
			TOML_VALUE(category, SPOOL_TIME_DOWN);
			TOML_VALUE(category, SPOOL_TIME_UP);
			TOML_VALUE(category, VACUUM);
		}
		Induction(const Induction& a, const Induction& b, float f) {
			TUNED_VALUE(HIGH_BOOST);
			TUNED_VALUE(LOW_BOOST);
			TUNED_VALUE(PSI);
			TUNED_VALUE(SPOOL);
			TUNED_VALUE(SPOOL_TIME_DOWN);
			TUNED_VALUE(SPOOL_TIME_UP);
			TUNED_VALUE(VACUUM);
		}
		Induction(const std::vector<Induction>& vec, float f) {
			if (!vec.empty()) {
				// if the car has none by default, leave it like that if it has zero upgrades
				if (f <= 0.0 || vec.size() == 1) {
					*this = Induction(vec[0]);
				}
				else {
					// some cars have default, _base and _top
					if (vec.size() == 3) {
						*this = Induction(vec[1], vec[2], f);
					}
					// other cars have only _base and _top
					else if (vec.size() == 2) {
						*this = Induction(vec[0], vec[1], f);
					}
					else {
						if (vec[0].PSI > 0.0) {
							*this = Induction(vec[0], vec[vec.size()-1], f);
						}
						else {
							*this = Induction(vec[1], vec[vec.size()-1], f);
						}
					}
				}
			}
			else {
				Induction();
			}
		}
	};

	struct Nos {
		float FLOW_RATE;
		float NOS_CAPACITY;
		float NOS_DISENGAGE;
		float RECHARGE_MAX;
		float RECHARGE_MAX_SPEED;
		float RECHARGE_MIN;
		float RECHARGE_MIN_SPEED;
		float TORQUE_BOOST;

		Nos() {
			FLOW_RATE = 0.0f;
			NOS_CAPACITY = 0.0f;
			NOS_DISENGAGE = 0.0f;
			RECHARGE_MAX = 0.0f;
			RECHARGE_MAX_SPEED = 0.0f;
			RECHARGE_MIN = 0.0f;
			RECHARGE_MIN_SPEED = 0.0f;
			TORQUE_BOOST = 0.0f;
		}
		Nos(const toml::table& config, const char* category) {
			TOML_VALUE(category, FLOW_RATE);
			TOML_VALUE(category, NOS_CAPACITY);
			TOML_VALUE(category, NOS_DISENGAGE);
			TOML_VALUE(category, RECHARGE_MAX);
			TOML_VALUE(category, RECHARGE_MAX_SPEED);
			TOML_VALUE(category, RECHARGE_MIN);
			TOML_VALUE(category, RECHARGE_MIN_SPEED);
			TOML_VALUE(category, TORQUE_BOOST);
		}
		Nos(const Nos& a, const Nos& b, float f) {
			TUNED_VALUE(FLOW_RATE);
			TUNED_VALUE(NOS_CAPACITY);
			TUNED_VALUE(NOS_DISENGAGE);
			TUNED_VALUE(RECHARGE_MAX);
			TUNED_VALUE(RECHARGE_MAX_SPEED);
			TUNED_VALUE(RECHARGE_MIN);
			TUNED_VALUE(RECHARGE_MIN_SPEED);
			TUNED_VALUE(TORQUE_BOOST);
		}
		Nos(const std::vector<Nos>& vec, float f) {
			if (!vec.empty()) {
				// if the car has none by default, leave it like that if it has zero upgrades
				if (f <= 0.0 || vec.size() == 1) {
					*this = Nos(vec[0]);
				}
				else {
					// some cars have default, _base and _top
					if (vec.size() == 3) {
						*this = Nos(vec[1], vec[2], f);
					}
					// other cars have only _base and _top
					else if (vec.size() == 2) {
						*this = Nos(vec[0], vec[1], f);
					}
					else {
						if (vec[0].FLOW_RATE > 0.0) {
							*this = Nos(vec[0], vec[vec.size()-1], f);
						}
						else {
							*this = Nos(vec[1], vec[vec.size()-1], f);
						}
					}
				}
			}
			else {
				Induction();
			}
		}
	};

	struct Tires {
		AxlePair ASPECT_RATIO;
		AxlePair DYNAMIC_GRIP;
		AxlePair GRIP_SCALE;
		AxlePair RIM_SIZE;
		AxlePair SECTION_WIDTH;
		AxlePair STATIC_GRIP;
		float STEERING;
		std::vector<float> YAW_CONTROL;
		float YAW_SPEED;

		Tires() {}
		Tires(const toml::table& config, const char* category) {
			TOML_AXLEPAIR(category, ASPECT_RATIO);
			TOML_AXLEPAIR(category, DYNAMIC_GRIP);
			TOML_AXLEPAIR(category, GRIP_SCALE);
			TOML_AXLEPAIR(category, RIM_SIZE);
			TOML_AXLEPAIR(category, SECTION_WIDTH);
			TOML_AXLEPAIR(category, STATIC_GRIP);
			TOML_VALUE(category, STEERING);
			TOML_VECTOR(category, YAW_CONTROL);
			TOML_VALUE(category, YAW_SPEED);
		}
		Tires(const Tires& a, const Tires& b, float f) {
			TUNED_AXLEPAIR(ASPECT_RATIO);
			TUNED_AXLEPAIR(DYNAMIC_GRIP);
			TUNED_AXLEPAIR(GRIP_SCALE);
			TUNED_AXLEPAIR(RIM_SIZE);
			TUNED_AXLEPAIR(SECTION_WIDTH);
			TUNED_AXLEPAIR(STATIC_GRIP);
			TUNED_VALUE(STEERING);
			TUNED_VECTOR(a.YAW_CONTROL, b.YAW_CONTROL, YAW_CONTROL, f);
			TUNED_VALUE(YAW_SPEED);
		}
		Tires(const std::vector<Tires>& vec, float f) {
			if (vec.empty()) __debugbreak();

			if (vec.size() == 1) *this = vec[0];
			else {
				*this = Tires(vec[0], vec[vec.size()-1], f);
			}
		}
	};

	struct Transmission {
		float CLUTCH_SLIP;
		float DIFFERENTIAL[3];
		float FINAL_GEAR;
		float GEAR_EFFICIENCY[9];
		std::vector<float> GEAR_RATIO;
		float OPTIMAL_SHIFT;
		float SHIFT_SPEED;
		float TORQUE_CONVERTER;
		float TORQUE_SPLIT;

		Transmission() {}
		Transmission(const toml::table& config, const char* category) {
			TOML_VALUE(category, CLUTCH_SLIP);
			TOML_ARRAY(category, DIFFERENTIAL);
			TOML_VALUE(category, FINAL_GEAR);
			TOML_ARRAY(category, GEAR_EFFICIENCY);
			TOML_VECTOR(category, GEAR_RATIO);
			TOML_VALUE(category, OPTIMAL_SHIFT);
			TOML_VALUE(category, SHIFT_SPEED);
			TOML_VALUE(category, TORQUE_CONVERTER);
			TOML_VALUE(category, TORQUE_SPLIT)

			// todo? carbon cars require this
			while (GEAR_RATIO[GEAR_RATIO.size()-1] <= 0.05) {
				GEAR_RATIO.pop_back();
				if (GEAR_RATIO.empty()) {
					MessageBoxA(nullptr, "ERROR: GEAR_RATIO is empty!", "nya?!~", MB_ICONERROR);
					__debugbreak();
				}
			}
		}
		Transmission(const Transmission& a, const Transmission& b, float f) {
			TUNED_VALUE(CLUTCH_SLIP);
			for (int i = 0; i < 3; i++) {
				TUNED_VALUE(DIFFERENTIAL[i]);
			}
			TUNED_VALUE(FINAL_GEAR);
			for (int i = 0; i < 9; i++) {
				TUNED_VALUE(GEAR_EFFICIENCY[i]);
			}
			TUNED_VECTOR(a.GEAR_RATIO, b.GEAR_RATIO, GEAR_RATIO, f);
			TUNED_VALUE(OPTIMAL_SHIFT);
			TUNED_VALUE(SHIFT_SPEED);
			TUNED_VALUE(TORQUE_CONVERTER);
			TUNED_VALUE(TORQUE_SPLIT);
		}
		Transmission(const std::vector<Transmission>& vec, float f) {
			if (vec.empty()) __debugbreak();

			if (vec.size() == 1) *this = vec[0];
			else {
				*this = Transmission(vec[0], vec[vec.size()-1], f);
			}
		}
	};

	// pvehicle
	std::string CollectionName;
	int brakes_upgrades; // 0 = no upgrades, 1 = ultimate only, 2 = super pro, ultimate, 3 = pro, super pro, ultimate, 4 = race, pro, super pro, ultimate
	int chassis_upgrades;
	int engine_upgrades;
	int induction_upgrades;
	float MASS;
	int nos_upgrades;
	float TENSOR_SCALE[3];
	int tires_upgrades;
	int transmission_upgrades;
	float HandlingRating[2];

	// frontend
	int Cost;
};

struct MWCarData : public MWCarDataBase {
	std::vector<Brakes> aBrakes;
	std::vector<Chassis> aChassis;
	std::vector<Engine> aEngine;
	std::vector<Induction> aInduction;
	std::vector<Nos> aNos;
	std::vector<Tires> aTires;
	std::vector<Transmission> aTransmission;

	template<typename T>
	static void CreateTunedVector(const toml::table& config, std::vector<T>& data, const char* category, int maxCount) {
		if (!config[category].is_table()) {
			MessageBoxA(nullptr, std::format("Failed to find {}!", category).c_str(), "nya?!~", MB_ICONERROR);
			__debugbreak();
		}
		
		data.clear();
		for (int i = 0; i < maxCount; i++) {
			if (i == 0) {
				data.push_back(T(config, category));
			}
			else {
				auto catWithId = std::format("{}_{}", category, i);
				if (!config[catWithId].is_table()) continue;
				data.push_back(T(config, std::format("{}_{}", category, i).c_str()));
			}
		}
	}

	MWCarData(const toml::table& config, const std::string& name) {
		CollectionName = name;
		std::transform(CollectionName.begin(), CollectionName.end(), CollectionName.begin(), [](unsigned char c){ return std::tolower(c); });

		if (config["pvehicle"]["MASS"].is_value()) {
			TOML_VALUE("pvehicle", MASS);
			TOML_ARRAY("pvehicle", TENSOR_SCALE);
		}
		else {
			TOML_VALUE("chassis", MASS);
			TOML_ARRAY("chassis", TENSOR_SCALE);
		}

		TOML_VALUE("pvehicle", brakes_upgrades);
		TOML_VALUE("pvehicle", chassis_upgrades);
		TOML_VALUE("pvehicle", engine_upgrades);
		TOML_VALUE("pvehicle", induction_upgrades);
		TOML_VALUE("pvehicle", nos_upgrades);
		TOML_VALUE("pvehicle", tires_upgrades);
		TOML_VALUE("pvehicle", transmission_upgrades);
		TOML_ARRAY("pvehicle", HandlingRating);

		if (config["frontend"]["Cost"].is_value()) {
			TOML_VALUE("frontend", Cost);
		}
		else {
			Cost = 0;
		}

		CreateTunedVector(config, aBrakes, "brakes", 4);
		CreateTunedVector(config, aChassis, "chassis", 4);
		CreateTunedVector(config, aEngine, "engine", 4);
		CreateTunedVector(config, aInduction, "induction", 4);
		CreateTunedVector(config, aNos, "nos", 4);
		CreateTunedVector(config, aTires, "tires", 4);
		CreateTunedVector(config, aTransmission, "transmission", 4);

		bool validNos = false;
		for (auto& nos : aNos) {
			if (nos.FLOW_RATE > 0.0) validNos = true;
		}
		if (!validNos) nos_upgrades = 0;

		bool validTurbo = false;
		for (auto& turbo : aInduction) {
			if (turbo.PSI > 0.0) validTurbo = true;
		}
		if (!validTurbo) induction_upgrades = 0;
	}
};
std::vector<MWCarData> aCarTunings;

MWCarData* LoadCarTuningFromFile(std::string carName) {
	if (carName.ends_with(".conf")) {
		for (int i = 0; i < 5; i++) {
			carName.pop_back();
		}
	}

	auto fileName = std::format("plugins/CarDataDump/{}.conf", carName);
	if (!std::filesystem::exists(fileName)) return nullptr;

	auto config = toml::parse_file(fileName);
	aCarTunings.emplace_back(config, carName);
	return &aCarTunings[aCarTunings.size()-1];
}

int GetCarTuning(const std::string& model) {
	for (auto& tuning : aCarTunings) {
		if (tuning.CollectionName == model) return &tuning - &aCarTunings[0];
	}
	if (auto tuning = LoadCarTuningFromFile(model)) {
		return aCarTunings.size()-1;
	}
	WriteLog(std::format("Failed to find tunings for {}", model));
	return -1;
}

#ifdef MWHANDLING_ASSETTO
UMath::Vector3 GetWheelBaseXZ(Car* car, int wheel) {
	auto acTire = &car->tyres[GetMWWheelID(wheel)];
	UMath::Vector3 v;
	acTire->hub->getBasePosition(&v);
	return v;
}

// wheels in AC are always at 0 Y, moved by graphicsoffset for center of mass reasons
float GetWheelBaseY(MWCarDataBase::Chassis* tuning, Car* car, int wheel) {
	auto acTire = &car->tyres[GetMWWheelID(wheel)];
	UMath::Vector3 v;
	acTire->hub->getBasePosition(&v);
	v.y += -acTire->data.radius;
	v.y += INCH2METERS(tuning->RIDE_HEIGHT.At(wheel / 2u));
	v.y += fTireOffset;
	return v.y;
}
#endif

class MWCarDataTuned : public MWCarDataBase, public MWCarDataBase::Brakes, public MWCarDataBase::Chassis, public MWCarDataBase::Engine, public MWCarDataBase::Induction, public MWCarDataBase::Nos, public MWCarDataBase::Tires, public MWCarDataBase::Transmission {
public:
	struct Junkman {
		bool brakes = false;
		bool chassis = false;
		bool engine = false;
		bool induction = false;
		bool nos = false;
		bool tires = false;
		bool transmission = false;

		bool Get(int part) {
			return (&brakes)[part];
		}

		void Set(int part, int i) {
			(&brakes)[part] = i;
		}

		void SetAll(bool b) {
			brakes = b;
			chassis = b;
			engine = b;
			induction = b;
			nos = b;
			tires = b;
			transmission = b;
		}
	};

	MWCarDataTuned(MWCarData& data, float brakes, float chassis, float engine, float induction, float nos, float tires, float transmission, Junkman junkman)
			: MWCarDataBase::Brakes(data.aBrakes, std::clamp(brakes, 0.0f, 1.0f)),
			  MWCarDataBase::Chassis(data.aChassis, std::clamp(chassis, 0.0f, 1.0f)),
			  MWCarDataBase::Engine(data.aEngine, std::clamp(engine, 0.0f, 1.0f)),
			  MWCarDataBase::Induction(data.aInduction, std::clamp(induction, 0.0f, 1.0f)),
			  MWCarDataBase::Nos(data.aNos, std::clamp(nos, 0.0f, 1.0f)),
			  MWCarDataBase::Tires(data.aTires, std::clamp(tires, 0.0f, 1.0f)),
			  MWCarDataBase::Transmission(data.aTransmission, std::clamp(transmission, 0.0f, 1.0f)),
			  MWCarDataBase(data) // car name, tensor scale, cost, etc
	{
		if (junkman.brakes) {
			BRAKES.Front *= 1.2;
			BRAKES.Rear *= 1.2;
			EBRAKE *= 1.1;
		}
		if (junkman.chassis) {
			DRAG_COEFFICIENT *= 0.9;
			ROLL_CENTER *= 1.05;
			SPRING_STIFFNESS.Front *= 1.05;
			SPRING_STIFFNESS.Rear *= 1.05;
			FRONT_WEIGHT_BIAS *= 1.01;
			AERO_COEFFICIENT *= 1.05;
			SPRING_PROGRESSION.Front *= 1.05;
			SPRING_PROGRESSION.Rear *= 1.05;
		}
		if (junkman.engine) {
			for (auto& f : TORQUE) {
				f *= 1.1;
			}
			for (auto& f : ENGINE_BRAKING) {
				f *= 1.1;
			}
		}
		if (junkman.induction) {
			HIGH_BOOST *= 1.15;
			LOW_BOOST *= 1.15;
		}
		if (junkman.nos) {
			NOS_CAPACITY *= 1.1;
			TORQUE_BOOST *= 1.02;
		}
		if (junkman.tires) {
			STATIC_GRIP.Front *= 1.1;
			STATIC_GRIP.Rear *= 1.1;
			DYNAMIC_GRIP.Front *= 1.1;
			DYNAMIC_GRIP.Rear *= 1.1;
			GRIP_SCALE.Front *= 1.02;
			GRIP_SCALE.Rear *= 1.02;
			for (auto& f : YAW_CONTROL) {
				f *= 1.1;
			}
			STEERING *= 1.05;
		}
		if (junkman.transmission) {
			for (auto& f : GEAR_EFFICIENCY) {
				f *= 1.1;
			}
		}
	}

	MWCarDataTuned(const std::string& model, float brakes, float chassis, float engine, float induction, float nos, float tires, float transmission, Junkman junkman) {
		WriteLog(std::format("GetLerpedCarTuning {} {:.2f} {:.2f} {:.2f} {:.2f} {:.2f} {:.2f} {:.2f}", model, brakes, chassis, engine, induction, nos, tires, transmission));

		auto id = GetCarTuning(model);
		if (id < 0) {
			MessageBoxA(nullptr, std::format("Failed to find tunings for {}", model).c_str(), "nya?!~", MB_ICONERROR);
			__debugbreak();
		}
		*this = MWCarDataTuned(aCarTunings[id], brakes, chassis, engine, induction, nos, tires, transmission, junkman);
	}

#ifdef MWHANDLING_ASSETTO
	MWCarDataTuned(const std::string& model, Car* pCar) {
		const char* tuningPath = pCar == pMyPlugin->car ? "plugins/player.tune" : "plugins/ai.tune";
		if (std::filesystem::exists(tuningPath)) {
			float brakes = 0.0;
			float chassis = 0.0;
			float engine = 0.0;
			float induction = 0.0;
			float nos = 0.0;
			float tires = 0.0;
			float transmission = 0.0;
			Junkman junkman;

			auto file = std::ifstream(tuningPath, std::ios::in | std::ios::binary);
			file.read((char*)&brakes, sizeof(float));
			file.read((char*)&chassis, sizeof(float));
			file.read((char*)&engine, sizeof(float));
			file.read((char*)&induction, sizeof(float));
			file.read((char*)&nos, sizeof(float));
			file.read((char*)&tires, sizeof(float));
			file.read((char*)&transmission, sizeof(float));
			file.read((char*)&junkman, sizeof(Junkman));

			*this = MWCarDataTuned(model, brakes, chassis, engine, induction, nos, tires, transmission, junkman);
		}
		else {
			Junkman junkman;
			junkman.SetAll(bUpgradeJunkman);
			float f = fUpgradeLevel;
			*this = MWCarDataTuned(model, f, f, f, f, f, f, f, junkman);
		}

		if (bMWWheelPositions) return;
		if (!pCar) return;

		// derived AC properties
		auto front = GetWheelBaseXZ(pCar, 0);
		auto rear = GetWheelBaseXZ(pCar, 2);

		// front_axle = front wheel position
		FRONT_AXLE = front.z;

		// front_axle - wheelbase = rear wheel position
		WHEEL_BASE = front.z - rear.z;

		// distance between wheel centers
		TRACK_WIDTH.Front = std::abs(front.x) * 2;
		TRACK_WIDTH.Rear = std::abs(rear.x) * 2;

		// plus wheel size
		TRACK_WIDTH.Front += SECTION_WIDTH.Front * 0.001;
		TRACK_WIDTH.Rear += SECTION_WIDTH.Rear * 0.001;

		// todo this is inaccurate but required for some cars not to flip over really easily in AC
		RIDE_HEIGHT.Front = 0.0;
		RIDE_HEIGHT.Rear = 0.0;
	}
#endif
};

#ifdef MWHANDLING_ASSETTO
Physics::Tunings PlayerCarTunings = {};
Physics::Tunings* GetVehicleMWTunings(Car* veh) {
	if (veh == pMyPlugin->car) return &PlayerCarTunings;
	return nullptr;
}
#endif

#undef TUNED_VALUE
#undef TUNED_AXLEPAIR
#undef TOML_VALUE
#undef TOML_AXLEPAIR
#undef TOML_VECTOR
#undef TOML_ARRAY