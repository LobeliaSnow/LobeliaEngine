#pragma once
namespace Lobelia::Graphics {
	template<class T> inline Mesh<T>::Mesh(int buffer_count) :originBuffer(buffer_count), bufferCount(static_cast<size_t>(buffer_count)) {
		BufferCreator::Create(vertexBuffer.GetAddressOf(), originBuffer.data(), static_cast<UINT>(sizeof(T)*bufferCount), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, sizeof(float));
	}
	template<class T> inline Mesh<T>::~Mesh() = default;
	template<class T> inline const ComPtr<ID3D11Buffer>& Mesh<T>::GetVertexBuffer() { return vertexBuffer; }
	template<class T> inline T* Mesh<T>::GetBuffer() { return originBuffer.data(); }
	template<class T> inline size_t Mesh<T>::GetCount() { return bufferCount; }
	template<class T> inline const std::string& Mesh<T>::GetMaterialName(int delimiter_index) { return delimiters[delimiter_index].first; }
	template<class T> void Mesh<T>::Update() { Device::GetContext()->UpdateSubresource(vertexBuffer.Get(), 0, nullptr, originBuffer.data(), 0, 0); }
	template<class T> inline void Mesh<T>::Set() {
		Update();
		UINT strides = sizeof(T);
		UINT offset = 0;
		Device::GetContext()->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &strides, &offset);
	}
}