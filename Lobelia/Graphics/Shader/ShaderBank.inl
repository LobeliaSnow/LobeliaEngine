namespace Lobelia::Graphics {
	template<class ShaderElement, class ...Args> void ShaderBank::Register(const std::string& key, Args ...args) {
		ResourceBank<ShaderElement>::Factory(key, args...);
	}
	template<class ShaderElement> void ShaderBank::UnRegister(const std::string& key) {
		ResourceBank<ShaderElement>::Erase(key);
	}
	template<class ShaderElement> ShaderElement* ShaderBank::Get(const std::string& key) {
		return ResourceBank<ShaderElement>::Get(key);
	}
}