#pragma once
namespace Lobelia::Graphics {
	template<class T> class Mesh final {
	private:
		ComPtr<ID3D11Buffer> vertexBuffer;
		std::vector<T> originBuffer;
		//std::vector<int> delimiters;
		size_t bufferCount;
		//int delimiterCount;
	public:
		Mesh(int buffer_count);
		~Mesh();
		//void SetDelimiters(const std::vector<int>& delimiters);
		const ComPtr<ID3D11Buffer>& GetVertexBuffer();
		T* GetBuffer();
		size_t GetCount();
		const std::string& GetMaterialName(int delimiter_index);
		//int GetDelimiter(int index);
		//size_t GetDelimiterCount();
		void Update();
		void Set();
	};
}
#include "Mesh.inl"