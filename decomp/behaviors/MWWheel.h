class WheelMW {
public:
	WWorldPos mWorldPos;
	UMath::Vector4 mNormal;
	UMath::Vector3 mPosition;
	unsigned int mFlags;
	UMath::Vector3 mForce;
	float mAirTime;
	UMath::Vector3 mLocalArm;
	float mCompression;
	UMath::Vector3 mWorldArm;
	int pad;
	UMath::Vector3 mVelocity;
	int pad2;
	int mSurface;
	float mSurfaceStick;
	UMath::Vector4 mIntegral;

	WheelMW(unsigned int flags) {
		WHEEL_FUNCTION_LOG("Wheel::Wheel");
		memset(this,0,sizeof(*this));
		mFlags = flags;
		Reset();
	}

	~WheelMW() {
		Reset();
	}

	void Reset() {
		WHEEL_FUNCTION_LOG("Wheel::Reset");
		mSurfaceStick = 0.0;
		mIntegral.x = 0.0;
		mIntegral.y = 0.0;
		mIntegral.z = 0.0;
		mIntegral.w = 0.0;
		mAirTime = 0.0;
		mVelocity.x = 0.0;
		mVelocity.y = 0.0;
		mVelocity.z = 0.0;
		mCompression = 0.0;
		mNormal.x = 0.0;
		mNormal.y = 0.0;
		mNormal.z = 0.0;
		mNormal.w = 0.0;
		mForce.x = 0.0;
		mForce.y = 0.0;
		mForce.z = 0.0;
		mWorldPos = WWorldPos();
		mSurface = 0;
	}

	void UpdateSurface(int surface);
	bool InitPosition(ICollisionBody* cb, IRigidBody *rb, double maxcompression);
	bool UpdatePosition(const UMath::Vector3 &body_av, const UMath::Vector3 &body_lv,
							   const UMath::Matrix4 &body_matrix, const UMath::Vector3 &cog,
							   float dT, float wheel_radius, bool usecache, float vehicle_height);

	const UMath::Vector4 &GetNormal() const {
		return mNormal;
	}

	const UMath::Vector3 &GetPosition() const {
		return mPosition;
	}

	void SetPosition(UMath::Vector3 &p) {
		mPosition = p;
	}

	void SetY(float y) {
		mPosition.y = y;
	}

	const UMath::Vector3 &GetForce() const {
		return mForce;
	}

	void SetForce(const UMath::Vector3 &f) {
		mForce = f;
	}

	void SetVelocity(const UMath::Vector3 &v) {
		mVelocity = v;
	}

	void IncAirTime(float dT) {
		mAirTime += dT;
	}

	void SetAirTime(float f) {
		mAirTime = f;
	}

	const UMath::Vector3 &GetLocalArm() const {
		return mLocalArm;
	}

	void SetLocalArm(UMath::Vector3 &arm) {
		mLocalArm = arm;
	}

	const UMath::Vector3 &GetWorldArm() const {
		return mWorldArm;
	}

	float GetCompression() const {
		return mCompression;
	}

	void SetCompression(float c) {
		mCompression = UMath::Max(c, 0.0f);
	}

	int GetSurface() const {
		return mSurface;
	}

	const UMath::Vector3 &GetVelocity() const {
		return mVelocity;
	}

	bool IsOnGround() const {
		return mCompression > 0.0f;
	}

	void UpdateTime(float dT) {
		if (mSurfaceStick <= 0.0 || dT >= mSurfaceStick) mSurfaceStick = 0.0;
		else mSurfaceStick = mSurfaceStick - dT;
	}
};