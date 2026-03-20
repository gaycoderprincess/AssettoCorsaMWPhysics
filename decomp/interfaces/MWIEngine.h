class IEngine : public IMWInterface {
public:
	static inline uint32_t _IIDName = 0x404020;

	virtual float GetRPM() = 0;
	virtual float GetRedline() = 0;
	virtual float GetMaxRPM() = 0;
	virtual float GetMinRPM() = 0;
	virtual float GetPeakTorqueRPM() = 0;
	virtual void MatchSpeed(float speed) = 0;
	virtual float GetNOSCapacity() = 0;
	virtual bool IsNOSEngaged() = 0;
	virtual float GetNOSFlowRate() = 0;
	virtual float GetNOSBoost() = 0;
	virtual bool HasNOS() = 0;
	virtual void ChargeNOS(float) = 0;
	virtual float GetMaxHorsePower() = 0;
	virtual float GetMinHorsePower() = 0;
	virtual float GetHorsePower() = 0;
};