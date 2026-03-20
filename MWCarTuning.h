struct MWCarTuning {
	std::string carName;

	// chassis
	AxlePair SHOCK_DIGRESSION;
	AxlePair SPRING_PROGRESSION;
	AxlePair TRAVEL;
	AxlePair RIDE_HEIGHT;
	AxlePair SHOCK_EXT_STIFFNESS;
	AxlePair SHOCK_STIFFNESS;
	AxlePair SPRING_STIFFNESS;
	AxlePair SHOCK_VALVING;
	AxlePair SWAYBAR_STIFFNESS;
	float ROLL_CENTER;
	float SHOCK_BLOWOUT;
	float AERO_CG;
	float RENDER_MOTION;
	float AERO_COEFFICIENT;
	float FRONT_WEIGHT_BIAS;
	float DRAG_COEFFICIENT;

	// brakes
	AxlePair BRAKE_LOCK;
	AxlePair BRAKES;
	float EBRAKE;

	// tires
	std::vector<float> YAW_CONTROL;
	AxlePair GRIP_SCALE;
	AxlePair DYNAMIC_GRIP;
	AxlePair STATIC_GRIP;
	float STEERING;
	float YAW_SPEED;

	// transmission
	std::vector<float> GEAR_RATIO;
	float DIFFERENTIAL[3];
	float GEAR_EFFICIENCY[9];
	float TORQUE_CONVERTER;
	float TORQUE_SPLIT;
	float CLUTCH_SLIP;
	float OPTIMAL_SHIFT;
	float SHIFT_SPEED;
	float FINAL_GEAR;

	// engine
	std::vector<float> TORQUE;
	float SPEED_LIMITER[2];
	std::vector<float> ENGINE_BRAKING;
	float FLYWHEEL_MASS;
	float MAX_RPM;
	float RED_LINE;
	float IDLE;

	// induction
	float LOW_BOOST;
	float SPOOL_TIME_DOWN;
	float VACUUM;
	float SPOOL;
	float SPOOL_TIME_UP;
	float PSI;
	float HIGH_BOOST;

	// nos
	float NOS_DISENGAGE = 2.0;
	float TORQUE_BOOST = 1.0;
	float FLOW_RATE = 3.0;
	float RECHARGE_MIN = 50;
	float NOS_CAPACITY = 5.0;
	float RECHARGE_MAX = 30;
	float RECHARGE_MAX_SPEED = 100;
	float RECHARGE_MIN_SPEED = 50;

	// pvehicle
	float TENSOR_SCALE[3];

	// extra tires stuff for flatout
	AxlePair ASPECT_RATIO = {40, 40};
	AxlePair RIM_SIZE = {18, 19};
	AxlePair SECTION_WIDTH = {245, 265};
	AxlePair TRACK_WIDTH = {1.534, 1.544};
	float WHEEL_BASE = 2.685;
	float FRONT_AXLE = 1.39;

	static void ReadDynamicArray(toml::table& config, std::vector<float>& out, const char* category, const char* name) {
		out.clear();
		for (int i = 0; i < 1024; i++) {
			float f = config[category][name][i].value_or(-0.011f);
			if (f == -0.011f) break;
			out.push_back(f);
		}
	}
};
std::vector<MWCarTuning> aCarTunings;
MWCarTuning* LoadCarTuningFromFile(std::string configCarName) {
	if (configCarName.ends_with(".conf")) {
		for (int i = 0; i < 5; i++) {
			configCarName.pop_back();
		}
	}

	// default to custom setups, then mw, then carbon
	auto fileName = std::format("CarDataDump/{}.conf", configCarName);
	if (!std::filesystem::exists(fileName)) return nullptr;
	auto config = toml::parse_file(fileName);

	MWCarTuning tmp;
	tmp.carName = configCarName;

	// brakes
	tmp.BRAKE_LOCK.Front = config["brakes"]["BRAKE_LOCK"][0].value_or(0.0);
	tmp.BRAKE_LOCK.Rear = config["brakes"]["BRAKE_LOCK"][1].value_or(0.0);
	tmp.BRAKES.Front = config["brakes"]["BRAKES"][0].value_or(0.0);
	tmp.BRAKES.Rear = config["brakes"]["BRAKES"][1].value_or(0.0);
	tmp.EBRAKE = config["brakes"]["EBRAKE"].value_or(0.0);

	// chassis
	tmp.AERO_CG = config["chassis"]["AERO_CG"].value_or(50.0);
	tmp.AERO_COEFFICIENT = config["chassis"]["AERO_COEFFICIENT"].value_or(0.0);
	tmp.DRAG_COEFFICIENT = config["chassis"]["DRAG_COEFFICIENT"].value_or(0.0);
	tmp.FRONT_WEIGHT_BIAS = config["chassis"]["FRONT_WEIGHT_BIAS"].value_or(50.0);
	tmp.RENDER_MOTION = config["chassis"]["RENDER_MOTION"].value_or(1.0);
	tmp.RIDE_HEIGHT.Front = config["chassis"]["RIDE_HEIGHT"][0].value_or(10.0);
	tmp.RIDE_HEIGHT.Rear = config["chassis"]["RIDE_HEIGHT"][1].value_or(10.0);
	tmp.ROLL_CENTER = config["chassis"]["ROLL_CENTER"].value_or(0.0);
	tmp.SHOCK_BLOWOUT = config["chassis"]["SHOCK_BLOWOUT"].value_or(0.0);
	tmp.SHOCK_DIGRESSION.Front = config["chassis"]["SHOCK_DIGRESSION"][0].value_or(0.0);
	tmp.SHOCK_DIGRESSION.Rear = config["chassis"]["SHOCK_DIGRESSION"][1].value_or(0.0);
	tmp.SHOCK_EXT_STIFFNESS.Front = config["chassis"]["SHOCK_EXT_STIFFNESS"][0].value_or(0.0);
	tmp.SHOCK_EXT_STIFFNESS.Rear = config["chassis"]["SHOCK_EXT_STIFFNESS"][1].value_or(0.0);
	tmp.SHOCK_STIFFNESS.Front = config["chassis"]["SHOCK_STIFFNESS"][0].value_or(0.0);
	tmp.SHOCK_STIFFNESS.Rear = config["chassis"]["SHOCK_STIFFNESS"][1].value_or(0.0);
	tmp.SHOCK_VALVING.Front = config["chassis"]["SHOCK_VALVING"][0].value_or(0.0);
	tmp.SHOCK_VALVING.Rear = config["chassis"]["SHOCK_VALVING"][1].value_or(0.0);
	tmp.SPRING_PROGRESSION.Front = config["chassis"]["SPRING_PROGRESSION"][0].value_or(0.0);
	tmp.SPRING_PROGRESSION.Rear = config["chassis"]["SPRING_PROGRESSION"][1].value_or(0.0);
	tmp.SPRING_STIFFNESS.Front = config["chassis"]["SPRING_STIFFNESS"][0].value_or(0.0);
	tmp.SPRING_STIFFNESS.Rear = config["chassis"]["SPRING_STIFFNESS"][1].value_or(0.0);
	tmp.SWAYBAR_STIFFNESS.Front = config["chassis"]["SWAYBAR_STIFFNESS"][0].value_or(0.0);
	tmp.SWAYBAR_STIFFNESS.Rear = config["chassis"]["SWAYBAR_STIFFNESS"][1].value_or(0.0);
	tmp.TRAVEL.Front = config["chassis"]["TRAVEL"][0].value_or(5.0);
	tmp.TRAVEL.Rear = config["chassis"]["TRAVEL"][1].value_or(5.0);

	// tires
	tmp.DYNAMIC_GRIP.Front = config["tires"]["DYNAMIC_GRIP"][0].value_or(1.0);
	tmp.DYNAMIC_GRIP.Rear = config["tires"]["DYNAMIC_GRIP"][1].value_or(1.0);
	tmp.GRIP_SCALE.Front = config["tires"]["GRIP_SCALE"][0].value_or(1.0);
	tmp.GRIP_SCALE.Rear = config["tires"]["GRIP_SCALE"][1].value_or(1.0);
	tmp.STATIC_GRIP.Front = config["tires"]["STATIC_GRIP"][0].value_or(2.0);
	tmp.STATIC_GRIP.Rear = config["tires"]["STATIC_GRIP"][1].value_or(2.0);
	tmp.STEERING = config["tires"]["STEERING"].value_or(1.0);
	tmp.ReadDynamicArray(config, tmp.YAW_CONTROL, "tires", "YAW_CONTROL");
	tmp.YAW_SPEED = config["tires"]["YAW_SPEED"].value_or(1.0);

	// transmission
	tmp.ReadDynamicArray(config, tmp.GEAR_RATIO, "transmission", "GEAR_RATIO");
	for (int i = 0; i < 3; i++) { tmp.DIFFERENTIAL[i] = config["transmission"]["DIFFERENTIAL"][i].value_or(1.0); }
	for (int i = 0; i < 9; i++) { tmp.GEAR_EFFICIENCY[i] = config["transmission"]["GEAR_EFFICIENCY"][i].value_or(1.0); }
	tmp.TORQUE_CONVERTER = config["transmission"]["TORQUE_CONVERTER"].value_or(0.5);
	tmp.TORQUE_SPLIT = config["transmission"]["TORQUE_SPLIT"].value_or(0.5);
	tmp.CLUTCH_SLIP = config["transmission"]["CLUTCH_SLIP"].value_or(0.5);
	tmp.OPTIMAL_SHIFT = config["transmission"]["OPTIMAL_SHIFT"].value_or(0.5);
	tmp.SHIFT_SPEED = config["transmission"]["SHIFT_SPEED"].value_or(0.5);
	tmp.FINAL_GEAR = config["transmission"]["FINAL_GEAR"].value_or(0.5);

	// engine
	tmp.ReadDynamicArray(config, tmp.TORQUE, "engine", "TORQUE");
	tmp.SPEED_LIMITER[0] = config["engine"]["SPEED_LIMITER"][0].value_or(0.0);
	tmp.SPEED_LIMITER[1] = config["engine"]["SPEED_LIMITER"][1].value_or(0.0);
	tmp.ReadDynamicArray(config, tmp.ENGINE_BRAKING, "engine", "ENGINE_BRAKING");
	tmp.FLYWHEEL_MASS = config["engine"]["FLYWHEEL_MASS"].value_or(0.0);
	tmp.MAX_RPM = config["engine"]["MAX_RPM"].value_or(0.0);
	tmp.RED_LINE = config["engine"]["RED_LINE"].value_or(0.0);
	tmp.IDLE = config["engine"]["IDLE"].value_or(0.0);

	// induction
	tmp.LOW_BOOST = config["induction"]["LOW_BOOST"].value_or(0.0);
	tmp.SPOOL_TIME_DOWN = config["induction"]["SPOOL_TIME_DOWN"].value_or(0.0);
	tmp.VACUUM = config["induction"]["VACUUM"].value_or(0.0);
	tmp.SPOOL = config["induction"]["SPOOL"].value_or(0.0);
	tmp.SPOOL_TIME_UP = config["induction"]["SPOOL_TIME_UP"].value_or(0.0);
	tmp.PSI = config["induction"]["PSI"].value_or(0.0);
	tmp.HIGH_BOOST = config["induction"]["HIGH_BOOST"].value_or(0.0);

	// pvehicle
	tmp.TENSOR_SCALE[0] = config["pvehicle"]["TENSOR_SCALE"][0].value_or(-0.011f);
	tmp.TENSOR_SCALE[1] = config["pvehicle"]["TENSOR_SCALE"][1].value_or(3.5);
	tmp.TENSOR_SCALE[2] = config["pvehicle"]["TENSOR_SCALE"][2].value_or(1.0);
	if (tmp.TENSOR_SCALE[0] == -0.011f) {
		tmp.TENSOR_SCALE[0] = 1.0;
		WriteLog(std::format("TENSOR_SCALE missing for {}", configCarName));
	}

	aCarTunings.push_back(tmp);
	return &aCarTunings[aCarTunings.size()-1];
}

int GetCarTuning(const std::string& model) {
	for (auto& tuning : aCarTunings) {
		if (tuning.carName == model) return &tuning - &aCarTunings[0];
	}
	if (auto tuning = LoadCarTuningFromFile(model)) {
		return aCarTunings.size()-1;
	}
	WriteLog(std::format("Failed to find tunings for {}", model));
	return -1;
}

#define TUNED_VALUE(value, delta) tmp.value = std::lerp(base->value, top->value, delta);

void GetLerpedCarTuning(MWCarTuning& tmp, const std::string& model, float brake, float drivetrain, float engine, float induction, float nitro, float suspension, float tire) {
	auto baseId = GetCarTuning(model);
	auto topId = GetCarTuning(model + "_top");
	auto base = baseId >= 0 ? &aCarTunings[baseId] : nullptr;
	auto top = topId >= 0 ? &aCarTunings[topId] : nullptr;
	if (!top && !base) {
		MessageBoxA(nullptr, std::format("Failed to find tunings for {}", model).c_str(), "nya?!~", MB_ICONERROR);
		__debugbreak();
	}
	if (!top) {
		tmp = *base;
		return;
	}
	if (!base) {
		tmp = *top;
		return;
	}

	if (base->YAW_CONTROL.size() != top->YAW_CONTROL.size() || base->GEAR_RATIO.size() != top->GEAR_RATIO.size() || base->TORQUE.size() != top->TORQUE.size() || base->ENGINE_BRAKING.size() != top->ENGINE_BRAKING.size()) {
		WriteLog(std::format("Mismatched tunings for {}", model));
	}

	while (base->YAW_CONTROL.size() < top->YAW_CONTROL.size()) { base->YAW_CONTROL.push_back(0.0); }
	while (base->GEAR_RATIO.size() < top->GEAR_RATIO.size()) { base->GEAR_RATIO.push_back(0.0); }
	while (base->TORQUE.size() < top->TORQUE.size()) { base->TORQUE.push_back(0.0); }
	while (base->ENGINE_BRAKING.size() < top->ENGINE_BRAKING.size()) { base->ENGINE_BRAKING.push_back(0.0); }

	while (top->YAW_CONTROL.size() < base->YAW_CONTROL.size()) { top->YAW_CONTROL.push_back(0.0); }
	while (top->GEAR_RATIO.size() < base->GEAR_RATIO.size()) { top->GEAR_RATIO.push_back(0.0); }
	while (top->TORQUE.size() < base->TORQUE.size()) { top->TORQUE.push_back(0.0); }
	while (top->ENGINE_BRAKING.size() < base->ENGINE_BRAKING.size()) { top->ENGINE_BRAKING.push_back(0.0); }

	WriteLog(std::format("GetLerpedCarTuning {} {:.2f} {:.2f} {:.2f} {:.2f} {:.2f} {:.2f} {:.2f}", model, brake, drivetrain, engine, induction, nitro, suspension, tire));

	tmp = *base;

	brake = UMath::Clamp(brake, 0.0, 1.0);
	drivetrain = UMath::Clamp(drivetrain, 0.0, 1.0);
	engine = UMath::Clamp(engine, 0.0, 1.0);
	induction = UMath::Clamp(induction, 0.0, 1.0);
	nitro = UMath::Clamp(nitro, -1.0, 1.0);
	suspension = UMath::Clamp(suspension, 0.0, 1.0);
	tire = UMath::Clamp(tire, 0.0, 1.0);

	// brakes
	TUNED_VALUE(BRAKE_LOCK.Front, brake);
	TUNED_VALUE(BRAKE_LOCK.Rear, brake);
	TUNED_VALUE(BRAKES.Front, brake);
	TUNED_VALUE(BRAKES.Rear, brake);
	TUNED_VALUE(EBRAKE, brake);

	// chassis
	TUNED_VALUE(AERO_CG, suspension);
	TUNED_VALUE(AERO_COEFFICIENT, suspension);
	TUNED_VALUE(DRAG_COEFFICIENT, suspension);
	TUNED_VALUE(FRONT_WEIGHT_BIAS, suspension);
	TUNED_VALUE(RENDER_MOTION, suspension);
	TUNED_VALUE(RIDE_HEIGHT.Front, suspension);
	TUNED_VALUE(RIDE_HEIGHT.Rear, suspension);
	TUNED_VALUE(ROLL_CENTER, suspension);
	TUNED_VALUE(SHOCK_BLOWOUT, suspension);
	TUNED_VALUE(SHOCK_DIGRESSION.Front, suspension);
	TUNED_VALUE(SHOCK_DIGRESSION.Rear, suspension);
	TUNED_VALUE(SHOCK_EXT_STIFFNESS.Front, suspension);
	TUNED_VALUE(SHOCK_EXT_STIFFNESS.Rear, suspension);
	TUNED_VALUE(SHOCK_STIFFNESS.Front, suspension);
	TUNED_VALUE(SHOCK_STIFFNESS.Rear, suspension);
	TUNED_VALUE(SHOCK_VALVING.Front, suspension);
	TUNED_VALUE(SHOCK_VALVING.Rear, suspension);
	TUNED_VALUE(SPRING_PROGRESSION.Front, suspension);
	TUNED_VALUE(SPRING_PROGRESSION.Rear, suspension);
	TUNED_VALUE(SPRING_STIFFNESS.Front, suspension);
	TUNED_VALUE(SPRING_STIFFNESS.Rear, suspension);
	TUNED_VALUE(SWAYBAR_STIFFNESS.Front, suspension);
	TUNED_VALUE(SWAYBAR_STIFFNESS.Rear, suspension);
	TUNED_VALUE(TRAVEL.Front, suspension);
	TUNED_VALUE(TRAVEL.Rear, suspension);

	// tires
	TUNED_VALUE(DYNAMIC_GRIP.Front, tire);
	TUNED_VALUE(DYNAMIC_GRIP.Rear, tire);
	TUNED_VALUE(GRIP_SCALE.Front, tire);
	TUNED_VALUE(GRIP_SCALE.Rear, tire);
	TUNED_VALUE(STATIC_GRIP.Front, tire);
	TUNED_VALUE(STATIC_GRIP.Rear, tire);
	TUNED_VALUE(STEERING, tire);
	for (int i = 0; i < tmp.YAW_CONTROL.size(); i++) {
		TUNED_VALUE(YAW_CONTROL[i], tire);
	}
	TUNED_VALUE(YAW_SPEED, tire);

	// transmission
	for (int i = 0; i < tmp.GEAR_RATIO.size(); i++) {
		TUNED_VALUE(GEAR_RATIO[i], drivetrain);
	}
	for (int i = 0; i < 3; i++) {
		TUNED_VALUE(DIFFERENTIAL[i], drivetrain);
	}
	for (int i = 0; i < 9; i++) {
		TUNED_VALUE(GEAR_EFFICIENCY[i], drivetrain);
	}
	TUNED_VALUE(TORQUE_CONVERTER, drivetrain);
	TUNED_VALUE(TORQUE_SPLIT, drivetrain);
	TUNED_VALUE(CLUTCH_SLIP, drivetrain);
	TUNED_VALUE(OPTIMAL_SHIFT, drivetrain);
	TUNED_VALUE(SHIFT_SPEED, drivetrain);
	TUNED_VALUE(FINAL_GEAR, drivetrain);

	// engine
	for (int i = 0; i < tmp.TORQUE.size(); i++) {
		TUNED_VALUE(TORQUE[i], engine);
	}
	TUNED_VALUE(SPEED_LIMITER[0], engine);
	TUNED_VALUE(SPEED_LIMITER[1], engine);
	for (int i = 0; i < tmp.ENGINE_BRAKING.size(); i++) {
		TUNED_VALUE(ENGINE_BRAKING[i], engine);
	}
	TUNED_VALUE(FLYWHEEL_MASS, engine);
	TUNED_VALUE(MAX_RPM, engine);
	TUNED_VALUE(RED_LINE, engine);
	TUNED_VALUE(IDLE, engine);

	// nos
	if (nitro < 0.0) {
		tmp.FLOW_RATE = 0.0;
		tmp.NOS_CAPACITY = 40.0;
		tmp.NOS_DISENGAGE = 2.0;
		tmp.RECHARGE_MAX = 0.0;
		tmp.RECHARGE_MAX_SPEED = 0.0;
		tmp.RECHARGE_MIN = 0.0;
		tmp.RECHARGE_MIN_SPEED = 0.0;
		tmp.TORQUE_BOOST = 0.0;
	}
	else {
		tmp.FLOW_RATE = 3.0;
		tmp.NOS_CAPACITY = std::lerp(2.5, 5.0, nitro);
		tmp.NOS_DISENGAGE = 2.0;
		tmp.RECHARGE_MAX = 30;
		tmp.RECHARGE_MAX_SPEED = 100;
		tmp.RECHARGE_MIN = 50;
		tmp.RECHARGE_MIN_SPEED = 50;
		tmp.TORQUE_BOOST = std::lerp(0.75, 1.0, nitro);
	}

	// induction
	TUNED_VALUE(LOW_BOOST, induction);
	TUNED_VALUE(SPOOL_TIME_DOWN, induction);
	TUNED_VALUE(VACUUM, induction);
	TUNED_VALUE(SPOOL, induction);
	TUNED_VALUE(SPOOL_TIME_UP, induction);
	TUNED_VALUE(PSI, induction);
	TUNED_VALUE(HIGH_BOOST, induction);

	while (tmp.GEAR_RATIO[tmp.GEAR_RATIO.size()-1] <= 0.35) { tmp.GEAR_RATIO.pop_back(); }
}

void GetLerpedCarTuning(MWCarTuning& out, const std::string& model) {
	float f = pGameFlow->PreRace.fUpgradeLevel;
	if (pGameFlow->PreRace.nMode == GM_CAREER) {
		f = 1.0;
	}
	return GetLerpedCarTuning(out, model, f, f, f, f, f, f, f);
}

Physics::Tunings* GetVehicleMWTunings(void* veh) {
	return nullptr;
}