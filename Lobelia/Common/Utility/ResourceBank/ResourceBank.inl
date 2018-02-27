namespace Lobelia {
	template<class T, class Key> __forceinline ResourceBank<T, Key>::ResourceBank()noexcept = default;
	template<class T, class Key> __forceinline ResourceBank<T, Key>::~ResourceBank()noexcept = default;
	template<class T, class Key> template<class U, class... Args> __forceinline T* ResourceBank<T, Key>::Factory(Key key, Args... args) {
		try {
			if (IsExist(key)) 	return resource[key].get();
			resource[key] = std::make_shared<U>(std::forward<Args>(args)...);
			return resource[key].get();
		}
		catch (...) {
			throw;
		}
	}
	template<class T, class Key> __forceinline void ResourceBank<T, Key>::Register(Key key, std::shared_ptr<T> p)noexcept { if (!IsExist(key))	resource[key].swap(p); }
	template<class T, class Key> __forceinline bool ResourceBank<T, Key>::IsExist(Key key) noexcept { return ((resource.find(key) != resource.end()) && (resource[key].use_count() != 0)); }
	template<class T, class Key> __forceinline void ResourceBank<T, Key>::Clear() noexcept { resource.clear(); }
	template<class T, class Key> __forceinline T* ResourceBank<T, Key>::Get(Key key) noexcept { return resource[key].get(); }
	template<class T, class Key> __forceinline std::map<Key, std::shared_ptr<T>>& ResourceBank<T, Key>::Get()noexcept { return resource; }
	template<class T, class Key> __forceinline void ResourceBank<T, Key>::Erase(Key key)noexcept { resource.erase(key); }
	//staticïœêî
	template<class T, class Key> std::map<Key, std::shared_ptr<T>> ResourceBank<T, Key>::resource;

}