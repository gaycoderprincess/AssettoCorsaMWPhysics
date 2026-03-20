class ICheater : public IMWInterface {
public:
	ICheater(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "ICheater";

	Car* pCar;

	// todo
	virtual float GetCatchupCheat() { return false; }
};