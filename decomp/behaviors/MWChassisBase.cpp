void ChassisMW::Destroy() {
	CHASSIS_FUNCTION_LOG("Destroy");

	GetOwner()->Remove<IChassis>(this);

	delete mMWAttributes;

	WriteLog("SuspensionRacer::Destroy finished");
}

void ChassisMW::Create(Car* car) {
	CHASSIS_FUNCTION_LOG("Create");

	mChassisType = "Chassis";

	pCar = car;

	GetOwner()->Add<IChassis>(this);

	mJumpTime = 0.0f;
	mJumpAlititude = 0.0f;
	mTireHeat = 0.0f;

	mMWAttributes = new MWCarTuning;
	GetLerpedCarTuning(*mMWAttributes, GetVehicle()->GetVehicleName());

	GetOwner()->QueryInterface(&mRBComplex);
	GetOwner()->QueryInterface(&mRB);
	GetOwner()->QueryInterface(&mInput);
	GetOwner()->QueryInterface(&mEngine);
	GetOwner()->QueryInterface(&mTransmission);
	GetOwner()->QueryInterface(&mEngineDamage);
	GetOwner()->QueryInterface(&mSpikeDamage);
}

Meters ChassisMW::GuessCompression(unsigned int id, Newtons downforce) {
	float compression = 0.0f;
	if (downforce < 0.0f) {
		unsigned int axle = id / 2;
		float spring_weight = LBIN2NM(mMWAttributes->SPRING_STIFFNESS.At(axle));
		downforce *= 0.25f;
		compression = -downforce / spring_weight;
	}
	return compression;
}

float ChassisMW::GetRenderMotion() {
	return mMWAttributes->RENDER_MOTION;
}

Meters ChassisMW::GetRideHeight(unsigned int idx) {
	return INCH2METERS(mMWAttributes->RIDE_HEIGHT.At(idx / 2));
}

float ChassisMW::CalculateUndersteerFactor() {
	float magnitude = 0.0f;
	float slip_avg = (GetWheelSkid(0) + GetWheelSkid(1)) / 2.0f;
	float steer = (GetWheelSteer(0) + GetWheelSteer(1)) / 2.0f;
	float speed = mRB->GetSpeed();
	if ((GetVehicle()->GetSpeed() > 0.0f) && (speed > 1.0f) && (steer * slip_avg < 0.0f)) {
		magnitude = UMath::Abs(slip_avg) / speed;
	}
	return UMath::Min(magnitude, 1.0f);
}

Mps ChassisMW::ComputeMaxSlip(const ChassisMW::State &state) {
	float ramp = UMath::Ramp(state.speed, 10.0f, 71.0f);
	float result = ramp + 0.5f;
	if (state.gear == G_REVERSE)
		result = 71.0f;
	return result;
}

void ChassisMW::DoTireHeat(const ChassisMW::State &state) {
	if (state.flags & 1) {
		for (unsigned int i = 0; i < GetNumWheels(); ++i) {
			if (GetWheelSlip(i) > 0.5f) {
				this->mTireHeat += state.time / 3.0f;
				this->mTireHeat = UMath::Min(this->mTireHeat, 1.0f);
				return;
			}
		}
	} else {
		if (this->mTireHeat > 0.0f) {
			this->mTireHeat -= state.time / 6.0f;
			this->mTireHeat = UMath::Max(this->mTireHeat, 0.0f);
		}
	}
}

float ChassisMW::CalculateOversteerFactor() {
	float speed = mRB->GetSpeed();
	float magnitude = 0.0f;
	if ((GetVehicle()->GetSpeed() > 0.0f) && (speed > 1.0f)) {
		magnitude = UMath::Abs((GetWheelSkid(3) + GetWheelSkid(2)) * 0.5f) / speed;
	}
	return UMath::Min(magnitude, 1.0f);
}

void ChassisMW::OnTaskSimulate(float dT) {}

float GripVsSpeed[] = {0.833f, 0.958f, 1.008f, 1.0167f, 1.033f, 1.033f, 1.033f, 1.0167f, 1.0f, 1.0f};
Table GripRangeTable(GripVsSpeed, 10, 0.0f, 1.0f);

// Credits: Brawltendo
float ChassisMW::ComputeLateralGripScale(const ChassisMW::State &state) {
	// lateral grip is tripled when in a drag race
	if (state.driver_style == STYLE_DRAG) {
		return 3.0f;
	}

	float ratio = UMath::Ramp(state.speed, 0.0f, MPH2MPS(85.0f));
	return GripRangeTable.GetValue(ratio) * 1.2f;
}

float TractionVsSpeed[] = {0.90899998f, 1.045f, 1.09f, 1.09f, 1.09f, 1.09f, 1.09f, 1.045f, 1.0f, 1.0f};
Table TractionRangeTable(TractionVsSpeed, 10, 0.0f, 1.0f);
static const float Traction_RangeMaxSpeedMPH = 85.0f;
static const float Tweak_GlobalTractionScale = 1.1f;
static const float Tweak_ReverseTractionScale = 2.0f;

// Credits: Brawltendo
float ChassisMW::ComputeTractionScale(const ChassisMW::State &state) {
	float result = 1.0f;

	if (state.driver_style == STYLE_DRAG) {
		result = Tweak_GlobalTractionScale;
	} else {
		float ratio = UMath::Ramp(state.speed, 0.0f, MPH2MPS(Traction_RangeMaxSpeedMPH));
		result = TractionRangeTable.GetValue(ratio) * Tweak_GlobalTractionScale;
	}

	if (state.gear == G_REVERSE) {
		result = Tweak_ReverseTractionScale;
	}

	return result;
}

ChassisMW::SleepState ChassisMW::DoSleep(const ChassisMW::State &state) {
	if (state.flags & 1) {
		return SS_NONE;
	}
	if (state.speed < 0.5f) {
		if ((GetNumWheelsOnGround() == GetNumWheels()) && (state.brake_input + state.ebrake_input > 0.0f) && (state.gas_input == 0.0f)) {
			if ((UMath::Length(state.angular_vel) < 0.25f) && (!mRBComplex->HasHadCollision())) {
				if (state.speed < FLOAT_EPSILON) {
					mRBComplex->Damp(1.0f);
				} else {
					mRBComplex->Damp(1.0f - state.speed);
				}
				for (unsigned int i = 0; i < GetNumWheels(); ++i) {
					SetWheelAngularVelocity(i, 0.0f);
				}
				return SS_ALL;
			}
		}
	}
	if (state.speed < 1.0f) {
		if ((UMath::Length(state.angular_vel) < 0.25f) && (state.gas_input <= 0.0f)) {
			UMath::Vector3 v = state.local_vel;
			UMath::Vector3 w = state.local_angular_vel;
			UMath::Vector3 f = *mRBComplex->GetForce();
			UMath::Vector3 t = *mRBComplex->GetTorque();
			mRB->ConvertWorldToLocal(&f, false);
			mRB->ConvertWorldToLocal(&t, false);

			v.x *= state.speed;
			w.y *= state.speed;
			f.x = -f.x * (1.0f - state.speed);
			t.y = -t.y * (1.0f - state.speed);

			UMath::Rotate(v, state.matrix, v);
			UMath::Rotate(w, state.matrix, w);
			UMath::Rotate(t, state.matrix, t);
			UMath::Rotate(f, state.matrix, f);

			mRB->Resolve(&f, &t);
			mRB->SetLinearVelocity(&v);
			mRB->SetAngularVelocity(&w);
			return SS_LATERAL;
		}
	}
	return SS_NONE;
}

void ChassisMW::OnBehaviorChange() {
	GetOwner()->QueryInterface(&mTransmission);
	GetOwner()->QueryInterface(&mEngine);
	GetOwner()->QueryInterface(&mEngineDamage);
	GetOwner()->QueryInterface(&mInput);
	GetOwner()->QueryInterface(&mRBComplex);
	GetOwner()->QueryInterface(&mRB);
	GetOwner()->QueryInterface(&mSpikeDamage);
}

// Credits: Brawltendo
void ChassisMW::ComputeAckerman(const float steering, const ChassisMW::State &state, UMath::Vector4 *left, UMath::Vector4 *right) {
	int going_right = true;
	float wheelbase = mMWAttributes->WHEEL_BASE;
	float wheeltrack = mMWAttributes->TRACK_WIDTH.Front;
	float steer_inside = ANGLE2RAD(steering);

	// clamp steering angle <= 180 degrees
	if (steer_inside > (float)M_PI)
		steer_inside -= (float)(std::numbers::pi*2);

	// negative steering angle indicates a left turn
	if (steer_inside < 0.0f) {
		going_right = false;
		steer_inside = -steer_inside;
	}

	// Ackermann steering geometry causes the outside wheel to have a smaller turning angle than the inside wheel
	// this is determined by the distance of the wheel to the center of the rear axle
	// this equation is a modified version of 1/tan(L/(R+T/2)), where L is the wheelbase, R is the steering radius, and T is the track width
	float steer_outside = (wheelbase * steer_inside) / (wheeltrack * steer_inside + wheelbase);
	float steer0, steer1; // 0 - right, 1 - left
	if (going_right) {
		steer0 = steer_inside;
		steer1 = steer_outside;
	} else {
		steer1 = -steer_inside;
		steer0 = -steer_outside;
	}

	float ca, sa;
	// calculate forward vector for front wheels
	UMath::Vector3 r;
	ca = cosf(steer0);
	sa = sinf(steer0);
	r.z = ca;
	r.x = sa;
	r.y = 0.0f;
	UMath::Rotate(r, state.matrix, r);
	*right = UMath::Vector4Make(r, steer0);

	UMath::Vector3 l;
	ca = cosf(steer1);
	sa = sinf(steer1);
	l.z = ca;
	l.x = sa;
	l.y = 0.0f;
	UMath::Rotate(l, state.matrix, l);
	*left = UMath::Vector4Make(l, steer1);
}

void ChassisMW::SetCOG(float extra_bias, float extra_ride) {
	float front_z = mMWAttributes->FRONT_AXLE;
	float rear_z = front_z - mMWAttributes->WHEEL_BASE;
	
	UMath::Vector3 dim;
	mRB->GetDimension(&dim);

	float fwbias = (mMWAttributes->FRONT_WEIGHT_BIAS + extra_bias) * 0.01f;
	if (GetNumWheelsOnGround() == 0) {
		fwbias = 0.5f;
	}
	float cg_z = (front_z - rear_z) * fwbias + rear_z;
	float cg_y = INCH2METERS(mMWAttributes->ROLL_CENTER) - (dim.y + UMath::Max(INCH2METERS(mMWAttributes->RIDE_HEIGHT.At(0) + extra_ride),
																			  INCH2METERS(mMWAttributes->RIDE_HEIGHT.At(1) + extra_ride)));
	UMath::Vector3 cog(0.0f, cg_y, cg_z);
	mRBComplex->SetCenterOfGravity(&cog);
}

void ChassisMW::ComputeState(float dT, ChassisMW::State &state) {
	state.time = dT;
	state.flags = 0;
	state.inertia = *mRBComplex->GetInertiaTensor();
	mRB->GetDimension(&state.dimension);

	mRB->GetMatrix4(&state.matrix);
	state.matrix.p = UMath::Vector4Make(*mRB->GetPosition(), 1.0f);

	state.local_vel = *mRB->GetLinearVelocity();
	state.linear_vel = state.local_vel;
	mRB->ConvertWorldToLocal(&state.local_vel, false);
	state.angular_vel = *mRB->GetAngularVelocity();
	state.local_angular_vel = state.angular_vel;
	mRB->ConvertWorldToLocal(&state.local_angular_vel, false);

	state.speed = UMath::Length(state.linear_vel);
	if (state.local_vel.z < 1.0f) {
		state.slipangle = 0.0f;
	} else {
		state.slipangle = UMath::Atan2a(state.local_vel.x, state.local_vel.z);
	}

	state.gas_input = UMath::Clamp(mInput->GetControlGas(), 0.0f, 1.0f);
	state.brake_input = UMath::Clamp(mInput->GetControlBrake(), 0.0f, 1.0f);
	state.ebrake_input = mInput->GetControlHandBrake();
	state.steer_input = UMath::Clamp(mInput->GetControlSteering(), -1.0f, 1.0f);

	state.cog = *mRBComplex->GetCenterOfGravity();
	state.ground_effect = GetNumWheelsOnGround() * 0.25f;
	state.mass = mRB->GetMass();
	state.driver_style = GetVehicle()->GetDriverStyle();
	state.driver_class = GetVehicle()->GetDriverClass();

	if (GetVehicle()->IsStaging()) {
		state.flags |= State::IS_STAGING;
	}

	if (mEngine) {
		state.nos_boost = mEngine->GetNOSBoost();
	} else {
		state.nos_boost = 1.0f;
	}
	if (mTransmission) {
		state.gear = mTransmission->GetGear();
	} else {
		state.gear = G_NEUTRAL;
	}
	//if (mDragTransmission) {
	//	state.shift_boost = mDragTransmission->GetShiftBoost();
	//} else {
		state.shift_boost = 1.0f;
	//}

	if (mEngineDamage && mEngineDamage->IsBlown() || GetVehicle()->IsDestroyed()) {
		state.brake_input = 1.0f;
		state.gas_input = 0.0f;
		state.ebrake_input = 1.0f;
	}

	UMath::Rotate(state.cog, state.matrix, state.world_cog);

	state.blown_tires = 0;
	if (mSpikeDamage) {
		unsigned int num_wheels = GetNumWheels();
		for (unsigned int i = 0; i < num_wheels; ++i) {
			if (mSpikeDamage->GetTireDamage(i) == TIRE_DAMAGE_BLOWN) {
				state.blown_tires |= (1 << i);
			}
		}
	}

	if (GetVehicle()->IsDestroyed()) {
		state.flags |= State::IS_DESTROYED;
	}
}

static float AeroDropOff = 0.5f;
static float AeroDropOffMin = 0.4f;
static float OffThrottleDragFactor = 2.0f;
static float OffThrottleDragCenterHeight = -0.1f;
static const float Tweak_TuningAero_Drag = 0.25f;
static const float Tweak_TuningAero_DownForce = 0.25f;
static const float Tweak_PlaneDynamics = 0.0f;

// Credits: Brawltendo
void ChassisMW::DoAerodynamics(const ChassisMW::State &state, float drag_pct, float aero_pct, float aero_front_z, float aero_rear_z,
							 const Physics::Tunings *tunings) {
	if (drag_pct > 0.0f) {
		const float dragcoef_spec = mMWAttributes->DRAG_COEFFICIENT;
		// drag increases relative to the car's speed
		// letting off the throttle will increase drag by OffThrottleDragFactor
		float drag = state.speed * drag_pct * dragcoef_spec;
		drag += drag * (OffThrottleDragFactor - 1.0f) * (1.0f - state.gas_input);
		if (tunings) {
			drag += drag * Tweak_TuningAero_Drag * tunings->Value[Physics::Tunings::AERODYNAMICS];
		}

		UMath::Vector3 drag_vector(state.linear_vel);
		drag_vector *= -drag;
		UMath::Vector3 drag_center(state.cog);

		// manipulate drag height based on off-throttle amount when 2 or more wheels are grounded
		if (state.ground_effect >= 0.5f)
			drag_center.y += OffThrottleDragCenterHeight * (1.0f - state.gas_input);

		UMath::RotateTranslate(drag_center, state.matrix, drag_center);
		mRB->ResolveForce(&drag_vector, &drag_center);
	}

	if (aero_pct > 0.0f) {
		// scale downforce by the gradient when less than 2 wheels are grounded
		float upness = UMath::Max(state.GetUpVector().y, 0.0f);
		if (state.ground_effect >= 0.5f)
			upness = 1.0f;

		// in reverse, the car's forward vector is used as the movement direction
		UMath::Vector3 movement_dir(state.GetForwardVector());
		if (state.speed > 0.0001f) {
			movement_dir = state.linear_vel;
			movement_dir *= 1.0f / state.speed;
		}

		float forwardness = UMath::Max(UMath::Dot(movement_dir, state.GetForwardVector()), 0.0f);
		forwardness = UMath::Max(AeroDropOffMin, UMath::Pow(forwardness, AeroDropOff));
		float downforce = aero_pct * upness * forwardness * Physics::Info::AerodynamicDownforce(mMWAttributes, state.speed);
		// lower downforce when car is in air
		if (state.ground_effect == 0.0f) {
			downforce *= 0.8f;
		}
		if (tunings) {
			downforce += downforce * Tweak_TuningAero_DownForce * tunings->Value[Physics::Tunings::AERODYNAMICS];
		}

		if (downforce > 0.0f) {
			UMath::Vector3 aero_center(state.cog.x, state.cog.y, state.cog.z);
			// when at least 1 wheel is grounded, change the downforce forward position using the aero CG and axle positions
			if (state.ground_effect != 0.0f) {
				aero_center.z = (aero_front_z - aero_rear_z) * (mMWAttributes->AERO_CG * 0.01f) + aero_rear_z;
			}

			if (Tweak_PlaneDynamics != 0.0f) {
				// just some random nonsense because the DWARF says there was a block here
				float pitch = UMath::Atan2a(UMath::Abs(state.matrix.z.z), state.matrix.z.x);
				aero_center.z *= pitch;
			}

			UMath::Vector3 force(0.0f, -downforce, 0.0f);
			UMath::RotateTranslate(aero_center, state.matrix, aero_center);
			UMath::Rotate(force, state.matrix, force);
			mRB->ResolveForce(&force, &aero_center);
		}
	}
}

static const int bJumpStabilizer = 1;
UMath::Vector2 JumpStabilizationGraph[] = {UMath::Vector2(0.0f, 0.0f), UMath::Vector2(0.4f, 0.15f), UMath::Vector2(2.0f, 5.0f)};
Graph JumpStabilization(JumpStabilizationGraph, 3);
static const int bActiveStabilizer = 1;
static const float fPitchStabilizerAction = 40.0f;
static const float fRollStabilizerAction = 20.0f;
static const float fStablizationAltitude = 15.0f;
static const float fStabilizerUp = 0.8f;
static const float fStabilizerSpeed = 5.0f;
static const float fStabilizerMaxAngVel = 0.0f;
static const int bDoLandingGravity = 1;
static const float fExtraLandingGravity = 3.0f;
static const float fLandingGravitySpeed = 20.0f;
static const float fLandingGravityUpThreshold = 0.96f;
static const float fLandingGravityMinTime = 1.0f;
static const float fLandingGravityMinAltitude = 2.0f;
static const float fLandingGravityMaxAltitude = 6.0f;

void ChassisMW::DoJumpStabilizer(const ChassisMW::State &state) {
	if (!bJumpStabilizer || !mRBComplex) {
		return;
	}

	int nTouching = GetNumWheelsOnGround();
	bool resolve = false;
	UMath::Vector4 ground_normal = *mRBComplex->GetGroundNormal();
	float altitude = -ground_normal.w;
	float ground_dot = UMath::Dot(state.GetUpVector(), UMath::Vector4To3(ground_normal));

	UMath::Vector3 damping_torque = {};
	UMath::Vector3 damping_force = {};

	if (!nTouching) {
		mJumpTime += state.time;
		mJumpAlititude = UMath::Max(mJumpAlititude, altitude);

		if (bDoLandingGravity) {
			float accel = fExtraLandingGravity;
			// apply more downforce when the car has been airborne for a long time
			if (mJumpTime > fLandingGravityMinTime && ground_dot > fLandingGravityUpThreshold && mJumpAlititude > fLandingGravityMinAltitude &&
				state.linear_vel.y < 0.0f && altitude < fLandingGravityMaxAltitude) {
				float alt_ratio = 1.0f - UMath::Ramp(altitude, 0.0f, fLandingGravityMaxAltitude);
				float speed_ratio = UMath::Ramp(state.speed, 0.0f, fLandingGravitySpeed);
				accel += alt_ratio * 10.0f * speed_ratio;
			}

			UMath::Vector3 df_extra(0.0f, -state.mass * accel, 0.0f);
			UMath::Add(damping_force, df_extra, damping_force);
			resolve = true;
		}
	} else {
		mJumpTime = 0.0f;
		mJumpAlititude = 0.0f;
	}

	if (bJumpStabilizer && nTouching < 2 && state.GetUpVector().y > fStabilizerUp && !mRBComplex->IsInGroundContact()) {
		float speed_ramp = UMath::Ramp(state.speed, 0.0f, fStabilizerSpeed);
		float avelmag = UMath::Length(state.local_angular_vel);
		float damping = speed_ramp * JumpStabilization.GetValue(avelmag);

		UMath::Vector3 damping_moment;
		UMath::Scale(state.local_angular_vel, state.inertia, damping_moment);
		UMath::Scale(damping_moment, -damping, damping_moment);
		damping_moment.y = 0.0f;
		UMath::Rotate(damping_moment, state.matrix, damping_moment);
		UMath::Add(damping_moment, damping_torque, damping_torque);
		resolve = true;
	}

	if (bActiveStabilizer && nTouching == 0 && ground_normal.y > 0.9f && state.GetUpVector().y > 0.1f && ground_dot > 0.8f &&
		altitude > FLOAT_EPSILON) {
		float altitude_ramp = 1.0f - UMath::Ramp(altitude, 0.0f, fStablizationAltitude);
		float speed_ramp = UMath::Ramp(state.speed, 0.0f, fStabilizerSpeed);

		UMath::Vector3 vMoment;
		UMath::Vector3 vFlatFwd;
		UMath::UnitCross(state.GetRightVector(), UMath::Vector4To3(ground_normal), vFlatFwd);
		float dot = UMath::Dot(vFlatFwd, state.GetForwardVector());
		if (dot < 0.99f) {
			UMath::UnitCross(vFlatFwd, state.GetForwardVector(), vMoment);
			float fMag = state.mass * speed_ramp * altitude_ramp * fPitchStabilizerAction * (dot - 1.0f);
			UMath::ScaleAdd(vMoment, fMag, damping_torque, damping_torque);
			resolve = true;
		}

		UMath::Vector3 vFlatRight;
		UMath::UnitCross(UMath::Vector4To3(ground_normal), state.GetForwardVector(), vFlatRight);
		dot = UMath::Dot(vFlatRight, state.GetRightVector());
		if (dot < 0.99f) {
			UMath::UnitCross(vFlatRight, state.GetRightVector(), vMoment);
			float fMag = state.mass * speed_ramp * fRollStabilizerAction * altitude_ramp * (dot - 1.0f);
			UMath::ScaleAdd(vMoment, fMag, damping_torque, damping_torque);
			resolve = true;
		}
	}

	if (resolve) {
		mRB->Resolve(&damping_force, &damping_torque);
	}
}