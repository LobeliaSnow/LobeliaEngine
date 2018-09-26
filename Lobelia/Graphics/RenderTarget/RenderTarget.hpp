#pragma once
namespace Lobelia::Graphics {
	class Texture;
	class RenderTarget {
	private:
		std::shared_ptr<Texture> texture;
		//�����_�[�^�[�Q�b�g�z��̏ꍇ�͘Z��������x�ɂ���ō��
		//�ꖇ�̏ꍇ�͂��ꂪ�����_�[�^�[�Q�b�g���̂���
		ComPtr<ID3D11RenderTargetView> renderTarget;
		//�����_�[�^�[�Q�b�g�z����g�p����ꍇ�̂ݎg���B
		//���̏��Ɉꖇ���A�N�Z�X�ł���悤�ɂ���
		std::vector<ComPtr<ID3D11RenderTargetView>> oneFaceTarget;
		std::shared_ptr<Texture> depth;
		ComPtr<ID3D11DepthStencilView> depthView;
	private:
		void CreateRenderTarget(const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample, int array_count);
		void CreateDepthView(int array_count);
	public:
		RenderTarget(const Math::Vector2& size, const DXGI_SAMPLE_DESC& sample = Config::GetRefPreference().msaa, const DXGI_FORMAT&  format = DXGI_FORMAT_R32G32B32A32_FLOAT, int array_count = 1);
		RenderTarget(const ComPtr<ID3D11Texture2D>& texture);
		~RenderTarget();
		Texture* GetTexture()const;
		void Clear(Utility::Color color);
		void ChangeDepthStencil(RenderTarget* view);
		void Activate();
		//TODO : ���������ɃZ�b�g�ł���悤��(MRT)
		//�[�x�o�b�t�@��0�Ԗڂ̂����g���܂�
		static void Activate(int rt_count, RenderTarget** rts);
	};

}