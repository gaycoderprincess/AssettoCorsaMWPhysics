class ICollisionBody : public IMWInterface {
public:
	ICollisionBody(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "ICollisionBody";

	Car* pCar;
	UMath::Vector3 vTensorScale = {0.0, 0.0, 0.0};
	UMath::Vector3 vCenterOfGravity = {0.0, 0.0, 0.0};

	// todo
	virtual UMath::Vector3* GetForce() {
		static UMath::Vector3 tmp;
		tmp = {0,0,0};
		return &tmp;
	}
	virtual UMath::Vector3* GetTorque() {
		static UMath::Vector3 tmp;
		tmp = {0,0,0};
		return &tmp;
	}

	virtual void SetCenterOfGravity(UMath::Vector3* cog) {
		vCenterOfGravity = *cog;
	}
	virtual UMath::Vector3* GetCenterOfGravity() {
		return &vCenterOfGravity;
	}

	virtual bool IsInGroundContact() {
		/*static UMath::Vector3 normal;
		normal = {0,1,0};

		auto origin = pCar->GetMatrix()->p;
		origin.y += 2;
		auto dir = NyaVec3(0,-1,0);

		tLineOfSightIn prop;
		prop.fMaxDistance = 10000;
		tLineOfSightOut out;
		if (CheckLineOfSight(&prop, pGameFlow->pHost->pUnkForLOS, &origin, &dir, &out)) {
			return out.fHitDistance > 2.5;
		}
		return false;*/
		return true; // todo for jump stabilizer!
	}
	virtual UMath::Vector3* GetGroundNormal() {
		static UMath::Vector3 normal;
		normal = {0,1,0};

		UMath::Vector3 origin;
		pCar->body->getPosition(&origin, 0.0);
		auto dir = NyaVec3(0,-1,0);

		RayCastResult result;
		if (GetTrack()->rayCast((vec3f*)&origin, (vec3f*)&dir, &result, 10000)) {
			normal = result.normal;
		}
		return &normal;
	}
	virtual UMath::Vector3* GetInertiaTensor() {
		if (vTensorScale.x == 0.0f) {
			MWCarTuning tune;
			GetLerpedCarTuning(tune, mCOMObject->Find<IVehicle>()->GetVehicleName());
			vTensorScale.x = tune.TENSOR_SCALE[0];
			vTensorScale.y = tune.TENSOR_SCALE[1];
			vTensorScale.z = tune.TENSOR_SCALE[2];
		}

		UMath::Vector3 dim;
		dim.x = std::max(std::abs(pCar->bounds.min.x), std::abs(pCar->bounds.max.x));
		dim.y = std::max(std::abs(pCar->bounds.min.y), std::abs(pCar->bounds.max.y));
		dim.z = std::max(std::abs(pCar->bounds.min.z), std::abs(pCar->bounds.max.z));

		static UMath::Vector3 tmp;
		tmp = CalculateInertiaTensor(vTensorScale, pCar->body->getMass(), dim);
		return &tmp;
	}
	virtual void Damp(float amount) {
		UMath::Vector3 linearVel;
		UMath::Vector3 angularVel;
		pCar->body->getVelocity(&linearVel);
		pCar->body->getAngularVelocity(&angularVel);

		float scale = 1.0f - amount;
		UMath::Scale(linearVel, scale, linearVel);
		UMath::Scale(angularVel, scale, angularVel);

		pCar->body->setVelocity(&linearVel);
		pCar->body->setAngularVelocity(&angularVel);
	}
	virtual bool HasHadCollision() { return false; }
};