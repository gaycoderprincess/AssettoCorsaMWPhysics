class IRigidBody : public IMWInterface {
public:
	IRigidBody(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "IRigidBody";

	Car* pCar;

	virtual float GetSpeed() {
		return pCar->GetVelocity()->length();
	}
	virtual float GetMass() {
		return pCar->fMass;
	}
	virtual void GetMatrix4(UMath::Matrix4* out) {
		*out = *pCar->GetMatrix();
		out->p = {0,0,0}; // todo is this correct?
	}
	virtual UMath::Vector3* GetPosition() {
		return (UMath::Vector3*)&pCar->GetMatrix()->p;
	}
	virtual void GetRightVector(UMath::Vector3* out) {
		*out = pCar->GetMatrix()->x;
	}
	virtual void GetUpVector(UMath::Vector3* out) {
		*out = pCar->GetMatrix()->y;
	}
	virtual void GetForwardVector(UMath::Vector3* out) {
		*out = pCar->GetMatrix()->z;
	}
	virtual void GetDimension(UMath::Vector3 *out) {
		out->x = std::max(std::abs(pCar->vCollisionFullMin.x), std::abs(pCar->vCollisionFullMax.x));
		out->y = std::max(std::abs(pCar->vCollisionFullMin.y), std::abs(pCar->vCollisionFullMax.y));
		out->z = std::max(std::abs(pCar->vCollisionFullMin.z), std::abs(pCar->vCollisionFullMax.z));
	}
	virtual void ConvertWorldToLocal(UMath::Vector3 *val, bool translate) {
		::ConvertWorldToLocal(pCar, *val, translate);
	}
	virtual UMath::Vector3* GetLinearVelocity() {
		return (UMath::Vector3*)pCar->GetVelocity();
	}
	virtual UMath::Vector3* GetAngularVelocity() {
		return (UMath::Vector3*)pCar->GetAngVelocity();
	}
	virtual void SetLinearVelocity(UMath::Vector3 *val) {
		*pCar->GetVelocity() = *val;
	}
	virtual void SetAngularVelocity(UMath::Vector3 *val) {
		*pCar->GetAngVelocity() = *val;
	}
	virtual void Resolve(const UMath::Vector3* _force, const UMath::Vector3* _torque) {
		auto force = *_force;
		auto torque = *_torque;

		float oom = 1.0 / pCar->fMass;
		auto dT = gGlobalTimer.fDeltaTime;

		auto bodyMatrix = *pCar->GetMatrix();
		bodyMatrix.p = {0,0,0}; // hmm...
		auto mInvWorldTensor = GetInverseWorldTensor(*mCOMObject->Find<ICollisionBody>()->GetInertiaTensor(), bodyMatrix);

		UMath::Vector3 mCOG = *mCOMObject->Find<ICollisionBody>()->GetCenterOfGravity();

		NyaVec3 v22;
		v22.x = ((bodyMatrix.x.x * mCOG.x) + (bodyMatrix.z.x * mCOG.z) + (bodyMatrix.y.x * mCOG.y));
		v22.y = ((bodyMatrix.x.y * mCOG.x) + (bodyMatrix.z.y * mCOG.z) + (bodyMatrix.y.y * mCOG.y));
		v22.z = ((bodyMatrix.x.z * mCOG.x) + (bodyMatrix.z.z * mCOG.z) + (bodyMatrix.y.z * mCOG.y));

		*pCar->GetVelocity() += (force * oom * dT);
		//pCar->GetMatrix()->p += (force * oom * dT) * dT;

		pCar->GetAngVelocity()->x += (mInvWorldTensor.x.x * torque.x * dT) + (mInvWorldTensor.z.x * torque.z * dT) + (mInvWorldTensor.y.x * torque.y * dT);
		pCar->GetAngVelocity()->y += (mInvWorldTensor.x.y * torque.x * dT) + (mInvWorldTensor.z.y * torque.z * dT) + (mInvWorldTensor.y.y * torque.y * dT);
		pCar->GetAngVelocity()->z += (mInvWorldTensor.x.z * torque.x * dT) + (mInvWorldTensor.z.z * torque.z * dT) + (mInvWorldTensor.y.z * torque.y * dT);

		bodyMatrix = *pCar->GetMatrix();
		bodyMatrix.p = {0,0,0}; // hmm...

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
		auto bodyMatrix = *pCar->GetMatrix();
		auto position = pCar->GetMatrix()->p;

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
		auto bodyMatrix = *pCar->GetMatrix();
		auto position = pCar->GetMatrix()->p;

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
		pCar->GetMatrix()->p.x += offset;
	}
	virtual void ModifyYPos(float offset) {
		pCar->GetMatrix()->p.y += offset;
	}
	virtual void ModifyZPos(float offset) {
		pCar->GetMatrix()->p.z += offset;
	}
};