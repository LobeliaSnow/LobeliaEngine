#pragma once
namespace Lobelia::Game {
	//�����ł�GPGPU��p����Raycast(Raypick)���������Ă���
	class StructuredBuffer;
	//---------------------------------------------------------------------------------------------
	//
	//	Raycaster
	//
	//---------------------------------------------------------------------------------------------
	//Ray�p�ɍ쐬����郁�b�V���̌`��o�b�t�@
	class RayMesh {
		friend class RayResult;
	public:
		RayMesh(Graphics::Model* model);
		~RayMesh() = default;
		void Set();
		int GetPolygonCount();
	private:
		//GPU�ւ̓��� �|���S�����
		struct Input {
			Math::Vector3 pos[3];
		};
	private:
		std::shared_ptr<StructuredBuffer> structuredBuffer;
		int polygonCount;
	};
	//---------------------------------------------------------------------------------------------
	class RayResult {
	public:
		//GPU����̏o�͗p�o�b�t�@
		struct Output {
			int hit;
			float length;
			Math::Vector3 normal;
		};
	public:
		RayResult(RayMesh* mesh);
		~RayResult() = default;
		void Set();
		void Clean();
		const Output* Lock();
		void UnLock();
	private:
		//���ʂ̓ǂݍ���
		void Load();
	private:
		std::shared_ptr<StructuredBuffer> structuredBuffer;
		std::unique_ptr<class UnorderedAccessView> uav;
		std::unique_ptr<class ReadGPUBuffer> readBuffer;
	};
	//---------------------------------------------------------------------------------------------
	//Ray�����N���X
	//Singleton�ł��悢�����H
	class Raycaster {
	public:
		Raycaster() = delete;
		~Raycaster() = delete;
		static void Initialize();
		//�������̓��[���h�ϊ��s��
		//����͗���������Ă��������Ă��܂�
		//�������ʂ����o�����Ƃ���ƒ������p�t�H�[�}���X�𗎂Ƃ�
		//�߂�l��Ray������ɔ�΂��ꂽ���ǂ���
		static bool Dispatch(const DirectX::XMMATRIX& world, RayMesh* mesh, RayResult* result, const Math::Vector3& begin, const Math::Vector3& end);
	private:
		//�R���X�^���g�o�b�t�@
		struct Info {
			Math::Vector4 rayBegin;
			Math::Vector4 rayEnd;
			DirectX::XMFLOAT4X4 world;
			DirectX::XMFLOAT4X4 worldInverse;
		};
	private:
		static std::unique_ptr<Graphics::ComputeShader> cs;
		static std::unique_ptr<Graphics::ConstantBuffer<Info>> cbuffer;
		static Info info;
	};

}