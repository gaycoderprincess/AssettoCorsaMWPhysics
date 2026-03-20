enum eTireDamage {
	TIRE_DAMAGE_NONE = 0,
	TIRE_DAMAGE_PUNCTURED = 1,
	TIRE_DAMAGE_BLOWN = 2,
};

class ISpikeable : public IMWInterface {
public:
	ISpikeable(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "ISpikeable";

	Car* pCar;

	virtual eTireDamage GetTireDamage(unsigned int wheelId) { return TIRE_DAMAGE_NONE; }
};