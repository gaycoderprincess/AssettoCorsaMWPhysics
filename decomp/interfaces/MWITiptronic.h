class ITiptronic : public IMWInterface {
public:
	static inline uint32_t _IIDName = 0x68A8A0;

	virtual bool SportShift(GearID) = 0;
};