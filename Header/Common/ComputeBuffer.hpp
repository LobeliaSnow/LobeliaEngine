#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//template�ɂ��悩��
	class StructuredBuffer {
		friend class UnorderedAccessView;
		friend class ReadGPUBuffer;
	public:
		StructuredBuffer(int struct_size, int count);
		~StructuredBuffer() = default;
		//�z��T�C�Y����������Ɗm�ۂ��ꂽ�������̃|�C���^�ȊO�̓_��
		void Update(const void* resource);
		void Set(int slot, Graphics::ShaderStageList stage);
		int GetStructSize();
		int GetCount();
		int GetBufferSize();
	private:
		ComPtr<ID3D11Buffer> buffer;
		ComPtr<ID3D11ShaderResourceView> srv;
		const int STRUCT_SIZE;
		const int COUNT;
	};
	//---------------------------------------------------------------------------------------------
	class UnorderedAccessView {
	public:
		UnorderedAccessView(Graphics::Texture* texture);
		UnorderedAccessView(StructuredBuffer* structured_buffer);
		void Set(int slot);
		void Clean(int slot);
	private:
		ComPtr<ID3D11UnorderedAccessView> uav;
	};
	//---------------------------------------------------------------------------------------------
	//STAGING�ō����o�b�t�@
	//�ŏI�I�ɍ�蒼���Ƃ���Buffer�N���X���ł���\��Ȃ̂�
	//����������ΏۂƂ���\�肾���A�v���g�^�C�v�Ȃ̂œ���̃N���X�݂̂ɑΉ�����
	class ReadGPUBuffer {
	public:
		ReadGPUBuffer(std::shared_ptr<StructuredBuffer> buffer);
		~ReadGPUBuffer() = default;
		//�\�[�X�̃o�b�t�@����ǂ߂�`�ɃR�s�[����
		void ReadCopy();
		//�}�b�v�̊J�n
		void* ReadBegin();
		template<class T> T* ReadBegin() { return r_cast<T*>(ReadBegin()); }
		//�A���}�b�v
		void ReadEnd();
	private:
		ComPtr<ID3D11Buffer> buffer;
		std::weak_ptr<StructuredBuffer> origin;
	};
	//---------------------------------------------------------------------------------------------
	struct RWByteAddressBuffer {
	public:
		ComPtr<ID3D11Buffer> buffer;
		ComPtr<ID3D11UnorderedAccessView> uav;
		RWByteAddressBuffer(void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args);
		~RWByteAddressBuffer();
		void ResourceUpdate(void* data_buffer, UINT element_size, UINT element_count);
	private:
		void CreateRWByteAddressBuffer(ComPtr<ID3D11Buffer>& buffer, void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args);
		void CreateUAV(ComPtr<ID3D11UnorderedAccessView>& uav, const ComPtr<ID3D11Buffer>& buffer);
	};
}