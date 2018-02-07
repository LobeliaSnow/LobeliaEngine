#pragma once
#include "../Data/ShaderFile/GPUParticle/GPUParticleDefine.hlsli"

namespace Lobelia::Graphics {
	class Pipeline;
	//�C���X�^���X�����������Ԃ�����\������
	//������CPU�ŁA�X�V�A�`���GPU��
	//TODO : ��ɒl���s�ς̂��̂́Astatic�ɂ��Ă܂Ƃ߂�
	class GPUParticleSystem {
	public:
		enum class BlendMode { Copy, Add, Sub, Screen };
	private:
		ALIGN(16)struct Info {
		public:
			//���̃t���[���̒ǉ���
			int appendCount = 0;
			//���̃t���[���̌o�ߎ���
			float elapsedTime = 0.0f;
			//�o�C�g�j�b�N�\�[�g�̔�r����v�f�̊Ԋu
			int compareInterval = 0;
			//�����\�[�g���~���\�[�g���̔��ʗp
			BOOL divideLevel = 0;
			BOOL isBitonicFinal = false;
		public:
			void Update(float elapsed_time);
		};
		struct RWByteAddressBuffer {
			ComPtr<ID3D11Buffer> buffer;
			ComPtr<ID3D11UnorderedAccessView> uav;
			RWByteAddressBuffer(void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args);
			~RWByteAddressBuffer();
			void ResourceUpdate(void* data_buffer, UINT element_size, UINT element_count);
		private:
			void CreateRWByteAddressBuffer(ComPtr<ID3D11Buffer>& buffer, void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args);
			void CreateUAV(ComPtr<ID3D11UnorderedAccessView>& uav, const ComPtr<ID3D11Buffer>& buffer);
		};
	public:
		//�p�[�e�B�N������`����\����
		struct Particle {
			Math::Vector3 pos;
			//�ړ���
			Math::Vector3 move;
			//�����x (���[�g�����b���b)
			Math::Vector3 power;
			//�g�p����e�N�X�`���̔ԍ�
			int textureIndex;
			//0.0f~1.0f
			Math::Vector2 uvPos;
			Math::Vector2 uvSize;
			//��������
			float aliveTime;
			//�o�ߎ���
			float elapsedTime;
			//�t�F�[�h�C���ɂ����鎞��
			float fadeInTime;
			//�t�F�[�h�A�E�g�J�n����
			float fadeOutTime;
			//�J�n���̊g�嗦
			float startScale;
			//�I�����̊g�嗦
			float endScale;
			//�J�n���̉�]�p
			float startRad;
			//�I�����̉�]�p
			float endRad;
			//�␳�F
			Math::Vector3 color;
			Particle(const Math::Vector3& pos, const Math::Vector3& move, const Math::Vector3& power, int texture_index, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, float alive_time, float fade_in_time, float fade_out_time, float start_scale, float end_scale, float start_rad, float end_rad, Utility::Color color);
			Particle();
			~Particle();
		};
	private:
		static const int TEXTURE_COUNT = 8;
		static const std::string BLEND_LIST[4];
	private:
		std::unique_ptr<ConstantBuffer<Info>> infoCBuffer;
		Info info;
		std::unique_ptr<RWByteAddressBuffer> appendData;
		std::unique_ptr<RWByteAddressBuffer> indexBuffer;
		std::unique_ptr<RWByteAddressBuffer> dataBuffer;
		std::unique_ptr<RWByteAddressBuffer> indirectArgs;
		std::unique_ptr<ComputeShader> appendCS;
		std::unique_ptr<ComputeShader> sortCS;
		std::unique_ptr<ComputeShader> updateCS;
		std::unique_ptr<GeometryShader> gs;
		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<InputLayout> inputLayout;
		Texture* textureList[TEXTURE_COUNT];
		Particle appendParticles[APPEND_PARTICLE_MAX];
	private:
		void RunAppend();
		void RunSort();
		void RunUpdate();
	public:
		GPUParticleSystem();
		~GPUParticleSystem();
		void SetTexture(int slot, Texture* texture);
		void Append(const Particle& particle);
		//���{���ɂ��邩�̌���
		void Update(float time_scale = 1.0f);
		void Render(BlendMode mode = BlendMode::Add);
	};
}