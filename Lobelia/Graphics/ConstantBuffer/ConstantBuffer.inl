namespace Lobelia::Graphics {
	template<class T> ConstantBuffer<T>::ConstantBuffer(UINT slot, byte activate_shader_elements) :slot(slot) {
		SetFunctor(activate_shader_elements);
		CreateConstantBuffer();
	}
	template<class T> ConstantBuffer<T>::~ConstantBuffer() = default;
	//TODO  : BufferCreator‚ğg‚Á‚Ä‘‚«’¼‚·‚±‚Æ
	template<class T> void ConstantBuffer<T>::ConstantBuffer::CreateConstantBuffer() {
		BufferCreator::Create(buffer.GetAddressOf(), nullptr, sizeof(T), D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, 0);
	}
	template<class T> void ConstantBuffer<T>::SetFunctor(byte activate_shader_elements) {
		if (activate_shader_elements&ShaderStageList::VS)	functor.push_back([this]() {Device::GetContext()->VSSetConstantBuffers(slot, 1, buffer.GetAddressOf()); });
		if (activate_shader_elements&ShaderStageList::PS)	functor.push_back([this]() {Device::GetContext()->PSSetConstantBuffers(slot, 1, buffer.GetAddressOf()); });
		if (activate_shader_elements&ShaderStageList::HS)	functor.push_back([this]() {Device::GetContext()->HSSetConstantBuffers(slot, 1, buffer.GetAddressOf()); });
		if (activate_shader_elements&ShaderStageList::GS)	functor.push_back([this]() {Device::GetContext()->GSSetConstantBuffers(slot, 1, buffer.GetAddressOf()); });
		if (activate_shader_elements&ShaderStageList::DS)	functor.push_back([this]() {Device::GetContext()->DSSetConstantBuffers(slot, 1, buffer.GetAddressOf()); });
		if (activate_shader_elements&ShaderStageList::CS)	functor.push_back([this]() {Device::GetContext()->CSSetConstantBuffers(slot, 1, buffer.GetAddressOf()); });
	}
	template<class T> void ConstantBuffer<T>::Activate(const T& cb_struct) {
		Device::GetContext()->UpdateSubresource(buffer.Get(), 0, nullptr, &cb_struct, 0, 0);
		for each(auto&& Set in functor) {
			Set();
		}
	}

}