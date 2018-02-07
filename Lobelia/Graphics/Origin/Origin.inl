namespace Lobelia::Graphics {
	template<class T> __forceinline Origin<T>::Origin() {
		static int issuedId = 0;
		uniqueId = issuedId;
		issuedId++;
	}
	template<class T> __forceinline Origin<T>::~Origin() {}
	template<class T> __forceinline bool Origin<T>::IsSet() {
		static int ID = -1; 
		if (uniqueId == ID)return true;
		else {
			ID = uniqueId;
			return false;
		}
		return true;
	}
}