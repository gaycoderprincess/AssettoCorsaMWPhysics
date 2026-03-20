class ITransmission : public IMWInterface {
public:
	static inline uint32_t _IIDName = 0x404010;

	virtual GearID GetGear() = 0;
	virtual GearID GetTopGear() = 0;
	virtual bool Shift(GearID gear) = 0;
	virtual bool IsGearChanging() = 0;
	virtual bool IsReversing() = 0;
	virtual float GetSpeedometer() = 0;
	virtual float GetMaxSpeedometer() = 0;
	virtual float GetDriveTorque() = 0;
	virtual float GetShiftPoint(GearID from_gear, GearID to_gear) = 0;
	virtual ShiftStatus GetShiftStatus(void) = 0;
	virtual ShiftPotential GetShiftPotential(void) = 0;
};