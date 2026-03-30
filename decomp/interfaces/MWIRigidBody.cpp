class IRigidBodyMW : public IMWInterface {
public:
	IRigidBodyMW(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "IRigidBody";

	Car* pCar;

	virtual float GetSpeed() {
		return GetLinearVelocity()->length();
	}
	virtual float GetMass() {
		return mCOMObject->Find<ICollisionBody>()->GetMass();
	}
	virtual void GetMatrix4(UMath::Matrix4* out) {
		pCar->body->getWorldMatrix(out, 0.0);
		out->p = {0,0,0}; // todo is this correct?
	}
	virtual UMath::Vector3* GetPosition() {
		static UMath::Vector3 tmp;
		pCar->body->getPosition(&tmp, 0.0);
		return &tmp;
	}
	virtual void GetRightVector(UMath::Vector3* out) {
		UMath::Matrix4 tmp;
		pCar->body->getWorldMatrix(&tmp, 0.0);
		*out = tmp.x;
	}
	virtual void GetUpVector(UMath::Vector3* out) {
		UMath::Matrix4 tmp;
		pCar->body->getWorldMatrix(&tmp, 0.0);
		*out = tmp.y;
	}
	virtual void GetForwardVector(UMath::Vector3* out) {
		UMath::Matrix4 tmp;
		pCar->body->getWorldMatrix(&tmp, 0.0);
		*out = tmp.z;
	}
	virtual void GetDimension(UMath::Vector3 *out) { // todo is this correct?
		out->x = std::abs((pCar->bounds.max.x - pCar->bounds.min.x)) * 0.5;
		out->y = std::abs((pCar->bounds.max.y - pCar->bounds.min.y)) * 0.5;
		out->z = std::abs((pCar->bounds.max.z - pCar->bounds.min.z)) * 0.5;
	}
	virtual void ConvertWorldToLocal(UMath::Vector3 *val, bool translate) {
		::ConvertWorldToLocal(pCar, *val, translate);
	}
	virtual void ConvertLocalToWorld(UMath::Vector3 *val, bool translate) {
		::ConvertLocalToWorld(pCar, *val, translate);
	}
	virtual UMath::Vector3* GetLinearVelocity() {
		static UMath::Vector3 tmp;
		pCar->body->getVelocity(&tmp);
		return &tmp;
	}
	virtual UMath::Vector3* GetAngularVelocity() {
		static UMath::Vector3 tmp;
		pCar->body->getAngularVelocity(&tmp);
		return &tmp;
	}
	virtual void SetLinearVelocity(UMath::Vector3 *val) {
		pCar->body->setVelocity(val);
	}
	virtual void SetAngularVelocity(UMath::Vector3 *val) {
		pCar->body->setAngularVelocity(val);
	}
	virtual void Resolve(const UMath::Vector3* _force, const UMath::Vector3* _torque) {
		auto force = *_force;
		auto torque = *_torque;

		float oom = 1.0 / GetMass();
		auto dT = fGlobalDeltaTime;

		UMath::Matrix4 bodyMatrix;
		GetMatrix4(&bodyMatrix);
		auto mInvWorldTensor = GetInverseWorldTensor(*mCOMObject->Find<ICollisionBody>()->GetInertiaTensor(), bodyMatrix);

		auto vel = *GetLinearVelocity();
		vel += (force * oom * dT);
		SetLinearVelocity(&vel);

		auto avel = *GetAngularVelocity();
		avel.x += (mInvWorldTensor.x.x * torque.x * dT) + (mInvWorldTensor.z.x * torque.z * dT) + (mInvWorldTensor.y.x * torque.y * dT);
		avel.y += (mInvWorldTensor.x.y * torque.x * dT) + (mInvWorldTensor.z.y * torque.z * dT) + (mInvWorldTensor.y.y * torque.y * dT);
		avel.z += (mInvWorldTensor.x.z * torque.x * dT) + (mInvWorldTensor.z.z * torque.z * dT) + (mInvWorldTensor.y.z * torque.y * dT);
		SetAngularVelocity(&avel);

		GetMatrix4(&bodyMatrix);

		mInvWorldTensor = GetInverseWorldTensor(*mCOMObject->Find<ICollisionBody>()->GetInertiaTensor(), bodyMatrix);
	}
	virtual void ResolveForce(UMath::Vector3* force) {
		UMath::Vector3 tmp = {0,0,0};
		Resolve(force, &tmp);
	}
	virtual void ResolveTorque(UMath::Vector3* torque) {
		UMath::Vector3 tmp = {0,0,0};
		Resolve(&tmp, torque);
	}
	virtual void ResolveTorque(const UMath::Vector3 *force, const UMath::Vector3 *p) {
		UMath::Matrix4 bodyMatrix;
		GetMatrix4(&bodyMatrix);
		auto position = *GetPosition();

		UMath::Vector3 mCOG = *mCOMObject->Find<ICollisionBody>()->GetCenterOfGravity();

		UMath::Vector3 cg;
		UMath::Vector3 torque;
		UMath::Vector3 r;
		UMath::Rotate(mCOG, bodyMatrix, cg);
		UMath::Add(cg, position, cg);
		UMath::Sub(*p, cg, r);
		UMath::Cross(r, *force, torque);
		UMath::Add(torque, torque, torque);

		UMath::Vector3 tmp = {0,0,0};
		Resolve(&tmp, &torque);
	}
	virtual void ResolveForce(const UMath::Vector3 *force, const UMath::Vector3 *p) {
		UMath::Matrix4 bodyMatrix;
		GetMatrix4(&bodyMatrix);
		auto position = *GetPosition();

		UMath::Vector3 mCOG = *mCOMObject->Find<ICollisionBody>()->GetCenterOfGravity();

		UMath::Vector3 cg;
		UMath::Vector3 torque;
		UMath::Vector3 r;
		UMath::Rotate(mCOG, bodyMatrix, cg);
		UMath::Add(cg, position, cg);
		UMath::Sub(*p, cg, r);
		UMath::Cross(r, *force, torque);
		Resolve(force, &torque);
	}
	virtual void ModifyXPos(float offset) {
		auto pos = *GetPosition();
		pos.x += offset;
		pCar->body->setPosition(&pos);
	}
	virtual void ModifyYPos(float offset) {
		auto pos = *GetPosition();
		pos.y += offset;
		pCar->body->setPosition(&pos);
	}
	virtual void ModifyZPos(float offset) {
		auto pos = *GetPosition();
		pos.z += offset;
		pCar->body->setPosition(&pos);
	}

	virtual void DoDrag() {
		bool bDoLinearDrag = true;
		bool bDoAngularDrag = true;

		UMath::Vector3 DRAG = {0,0,0};
		UMath::Vector3 DRAG_ANGULAR = {0,0,0};

		if (bDoAngularDrag) {
			auto angularVel = *GetAngularVelocity();
			ConvertWorldToLocal(&angularVel, false);

			UMath::Vector3 dim;
			GetDimension(&dim);

			UMath::Vector3 v33;
			v33.x = std::sqrt(dim.y * dim.y + dim.z * dim.z) * dim.x * 4.0;
			v33.y = std::sqrt(dim.z * dim.z + dim.x * dim.x) * dim.y * 4.0;
			v33.z = std::sqrt(dim.x * dim.x + dim.y * dim.y) * dim.z * 4.0;

			UMath::Vector3 outDrag;
			Dynamics::Aero::Drag(outDrag, angularVel, DRAG_ANGULAR, v33, 1.225);
			ConvertLocalToWorld(&outDrag, false);
			ResolveTorque(&outDrag);
		}
		if (bDoLinearDrag) {
			auto linearVel = *GetLinearVelocity();
			ConvertWorldToLocal(&linearVel, false);

			UMath::Vector3 dim;
			GetDimension(&dim);

			UMath::Vector3 v33;
			v33.x = (dim.x * dim.x) * 4.0;
			v33.y = (dim.y * dim.y) * 4.0;
			v33.z = (dim.z * dim.z) * 4.0;

			UMath::Vector3 outDrag;
			Dynamics::Aero::Drag(outDrag, linearVel, DRAG, v33, 1.225);
			ConvertLocalToWorld(&outDrag, false);
			ResolveForce(&outDrag);
		}
	}
};