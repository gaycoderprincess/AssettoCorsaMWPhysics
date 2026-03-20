class IRaceEngine : public IMWInterface {
public:
	static inline uint32_t _IIDName = 0x668390;

	virtual float GetPerfectLaunchRange(float* range) = 0;
};