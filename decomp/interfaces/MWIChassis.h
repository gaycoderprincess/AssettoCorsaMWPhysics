class IChassis : public IMWInterface {
public:
	static inline const char* _IIDName = "IChassis";

	virtual float GetWheelSkid(unsigned int i);
	virtual float GetWheelSteer(unsigned int i);
	virtual float GetWheelSlip(unsigned int i);
	virtual float GetWheelAngularVelocity(int i);
	virtual bool IsWheelOnGround(unsigned int i);
	virtual unsigned int GetNumWheels();
	virtual int GetNumWheelsOnGround();
	virtual void SetWheelAngularVelocity(int i, float f);
};