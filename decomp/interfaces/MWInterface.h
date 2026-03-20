class IPlayer;
class MWInterfaceList;
class IMWInterface {
public:
	MWInterfaceList* mCOMObject;

	virtual ~IMWInterface() {}
};

class MWInterfaceList {
public:
	Car* pCar;
	struct MWInterfaceEntry {
		IMWInterface* pInterface;
		uintptr_t id;
	};
    std::vector<MWInterfaceEntry> aInterfaces;

	template<typename T>
	void Add(T* ptr) {
		ptr->mCOMObject = this;
		aInterfaces.push_back({ptr, (uintptr_t)T::_IIDName});
	}
	template<typename T>
	T* Find() {
		for (auto& ptr : aInterfaces) {
			if (ptr.id == (uintptr_t)T::_IIDName) return (T*)ptr.pInterface;
		}
		return nullptr;
	}
	template<typename T>
	void Remove(T* target) {
		for (auto& ptr : aInterfaces) {
			if (ptr.pInterface == target) {
				aInterfaces.erase(aInterfaces.begin() + (&ptr - &aInterfaces[0]));
				return;
			}
		}
	}

	template<typename T>
	bool QueryInterface(T** out) {
		auto p = Find<T>();
		*out = p;
		return p != nullptr;
	}

	IPlayer* GetPlayer() {
		return Find<IPlayer>();
	}
};
MWInterfaceList aPlayerInterfaces[32] = {};

MWInterfaceList* GetPlayerInterface(Car* car) {
	for (auto& ply : aPlayerInterfaces) {
		if (ply.pCar == car) return &ply;
	}
	return nullptr;
}