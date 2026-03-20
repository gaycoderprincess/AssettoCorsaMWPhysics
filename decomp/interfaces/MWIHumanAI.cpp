class IHumanAI : public IMWInterface {
public:
	static inline const char* _IIDName = "IHumanAI";

	virtual bool IsPlayerSteering() { return true; }
};