#include "Lobelia.hpp"

//template RenderableObjectみたいなのを作ってそれにステートやらシェーダーやらを保存それの派生で描画部分を作る
//2DSpriteはレンダラ形式に変更
//TODO : pipelineシステムは根絶する

namespace Lobelia {
	void CreateStencilStatePreset() {
		Graphics::StencilDesc desc0 = {};
		desc0.readMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc0.writeMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		desc0.front.depthFail = Graphics::StencilOperation::KEEP;
		desc0.front.faile = Graphics::StencilOperation::KEEP;
		desc0.front.pass = Graphics::StencilOperation::INCR_CLAMP;
		desc0.front.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		desc0.back.depthFail = Graphics::StencilOperation::KEEP;
		desc0.back.faile = Graphics::StencilOperation::KEEP;
		desc0.back.pass = Graphics::StencilOperation::DECR_CLAMP;
		desc0.back.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		Graphics::RenderStateBank::DepthStencilFactory(D_SWRITER_ZOFF, Graphics::DepthPreset::ALWAYS, false, desc0, true);
		Graphics::RenderStateBank::DepthStencilFactory(D_SWRITE_Z_ON, Graphics::DepthPreset::ALWAYS, true, desc0, true);
		Graphics::StencilDesc desc1 = {};
		desc1.readMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc1.writeMask = 0x00;
		desc1.front.depthFail = Graphics::StencilOperation::KEEP;
		desc1.front.faile = Graphics::StencilOperation::ZERO;
		desc1.front.pass = Graphics::StencilOperation::KEEP;
		desc1.front.testFunc = Graphics::StencilFunction::LESS_EQUAL;
		desc1.back.depthFail = Graphics::StencilOperation::KEEP;
		desc1.back.faile = Graphics::StencilOperation::ZERO;
		desc1.back.pass = Graphics::StencilOperation::KEEP;
		desc1.back.testFunc = Graphics::StencilFunction::LESS_EQUAL;
		Graphics::RenderStateBank::DepthStencilFactory(D_SREAD_ZOFF, Graphics::DepthPreset::ALWAYS, false, desc1, true);
		Graphics::RenderStateBank::DepthStencilFactory(D_SREAD_ZON, Graphics::DepthPreset::ALWAYS, true, desc1, true);
		//User
		Graphics::StencilDesc desc2 = {};
		desc2.readMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc2.writeMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		desc2.front.depthFail = Graphics::StencilOperation::KEEP;
		desc2.front.faile = Graphics::StencilOperation::KEEP;
		desc2.front.pass = Graphics::StencilOperation::INVERT;
		desc2.front.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		desc2.back.depthFail = Graphics::StencilOperation::KEEP;
		desc2.back.faile = Graphics::StencilOperation::KEEP;
		desc2.back.pass = Graphics::StencilOperation::INVERT;
		desc2.back.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		Graphics::RenderStateBank::DepthStencilFactory("StencilWrite", Graphics::DepthPreset::ALWAYS, false, desc2, true);
		Graphics::StencilDesc desc3 = {};
		desc3.readMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc3.writeMask = 0x00;
		desc3.front.depthFail = Graphics::StencilOperation::KEEP;
		desc3.front.faile = Graphics::StencilOperation::ZERO;
		desc3.front.pass = Graphics::StencilOperation::KEEP;
		desc3.front.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		desc3.back.depthFail = Graphics::StencilOperation::KEEP;
		desc3.back.faile = Graphics::StencilOperation::ZERO;
		desc3.back.pass = Graphics::StencilOperation::KEEP;
		desc3.back.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		Graphics::RenderStateBank::DepthStencilFactory("StencilRead", Graphics::DepthPreset::ALWAYS, false, desc3, true);

	}
	void CreateDefaultPipeline() {
		//共通部
		Graphics::RenderStateBank::BlendFactory("Copy", Graphics::BlendPreset::COPY, true, false);
		Graphics::RenderStateBank::BlendFactory("Add", Graphics::BlendPreset::ADD, true, false);
		Graphics::RenderStateBank::BlendFactory("Sub", Graphics::BlendPreset::SUB, true, false);
		Graphics::RenderStateBank::BlendFactory("Screen", Graphics::BlendPreset::SCREEN, true, false);
		Graphics::RenderStateBank::BlendFactory("Copy AlphaCoverage", Graphics::BlendPreset::COPY, true, true);
		Graphics::RenderStateBank::BlendFactory("Add AlphaCoverage", Graphics::BlendPreset::ADD, true, true);
		Graphics::RenderStateBank::BlendFactory("Sub AlphaCoverage", Graphics::BlendPreset::SUB, true, true);
		Graphics::RenderStateBank::BlendFactory("Screen AlphaCoverage", Graphics::BlendPreset::SCREEN, true, true);
		Graphics::RenderStateBank::SamplerFactory("Point", Graphics::SamplerPreset::POINT);
		Graphics::RenderStateBank::RasterizerFactory("Cull None", Graphics::RasterizerPreset::NONE);
		//2D用パイプライン構築
		//Zバッファ、ステンシルテストともにオフ
		Graphics::RenderStateBank::DepthStencilFactory(D_SOFF_ZOFF, Graphics::DepthPreset::ALWAYS, false, {}, false);
		Graphics::RenderStateBank::RasterizerFactory("Cull Back", Graphics::RasterizerPreset::BACK);
		Graphics::PixelShader* ps = ResourceBank<Graphics::PixelShader>::Factory(D_PS2D, "Data/ShaderFile/2D/PS.hlsl", "Main2D", Graphics::PixelShader::Model::PS_5_0, true);
		//ここは連番で0~IDが返ってきます
		//デフォルトのdefineは0~連番の直値ですのでここを変える場合はdefineのほうも変えてください
		ps->GetLinkage()->CreateInstance(D_PS2D_INAME_TEX);
		ps->GetLinkage()->CreateInstance(D_PS2D_INAME_COLOR);
		ps->GetLinkage()->CreateInstance(D_PS2D_INAME_INVTEX);
		ps->GetLinkage()->CreateInstance(D_PS2D_INAME_GRAYSTEX);
		ps->GetLinkage()->CreateInstance(D_PS2D_INAME_SEPIATEX);
		//2DSprite用パイプライン構築
		ResourceBank<Graphics::VertexShader>::Factory(D_VS2D, "Data/ShaderFile/2D/VS.hlsl", "Main2D", Graphics::VertexShader::Model::VS_5_0);
		//2Dインスタンシング用
		ResourceBank<Graphics::VertexShader>::Factory(D_VS2D_BATCH, "Data/ShaderFile/2D/VS.hlsl", "Main2DInst", Graphics::VertexShader::Model::VS_5_0);

		//スプライト用パイプライン構築
		Graphics::InstanceID id = D_PS2D_INS_TEX_ID;
		Graphics::PipelineManager::PipelineRegister(D_PIPE2D_S, new Graphics::Pipeline("Copy", "Point", D_SOFF_ZOFF, "Cull Back", D_VS2D, 0, nullptr, D_PS2D, 1, &id));
		//バッチ用パイプライン構築
		Graphics::PipelineManager::PipelineRegister(D_PIPE2D_BATCH, new Graphics::Pipeline("Copy", "Point", D_SOFF_ZOFF, "Cull Back", D_VS2D_BATCH, 0, nullptr, D_PS2D, 1, &id));
		//3D用パイプライン構築
		Graphics::RenderStateBank::SamplerFactory("Anisotropic", Graphics::SamplerPreset::ANISOTROPIC);
		//Zバッファオン ステンシルオフ
		Graphics::RenderStateBank::DepthStencilFactory(D_SOFF_ZON, Graphics::DepthPreset::ALWAYS, true, {}, false);
		//FBX用
		Graphics::RenderStateBank::RasterizerFactory("Cull Front", Graphics::RasterizerPreset::FRONT);
		//スタティックメッシュ用
		Graphics::ShaderBank::Register<Graphics::VertexShader>(D_VS3D_S, "Data/Shaderfile/3D/VS.hlsl", "Main3DNoSkin", Graphics::VertexShader::Model::VS_4_0);
		//スキニング用
		Graphics::ShaderBank::Register<Graphics::VertexShader>(D_VS3D_D, "Data/Shaderfile/3D/VS.hlsl", "Main3D", Graphics::VertexShader::Model::VS_4_0);
		//インスタンシング用(スタティックメッシュ)
		Graphics::ShaderBank::Register<Graphics::VertexShader>(D_VS3D_IS, "Data/Shaderfile/3D/VS.hlsl", "Main3DInstancingNoSkin", Graphics::VertexShader::Model::VS_4_0);
		//インスタンシング用(スキニング)
		Graphics::ShaderBank::Register<Graphics::VertexShader>(D_VS3D_ID, "Data/Shaderfile/3D/VS.hlsl", "Main3DInstancing", Graphics::VertexShader::Model::VS_4_0);
		Graphics::ShaderBank::Register<Graphics::PixelShader>(D_PS3D, "Data/Shaderfile/3D/PS.hlsl", "Main3D", Graphics::PixelShader::Model::PS_5_0, true);

		Graphics::PixelShader* ps3d = Graphics::ShaderBank::Get<Graphics::PixelShader>(D_PS3D);
		//ここは連番で0~IDが返ってきます
		//デフォルトのdefineは0~連番の直値ですのでここを変える場合はdefineのほうも変えてください
		ps3d->GetLinkage()->CreateInstance(D_PS3D_INAME_LAMBERT);
		ps3d->GetLinkage()->CreateInstance(D_PS3D_INAME_LAMBERT_FOG);
		ps3d->GetLinkage()->CreateInstance(D_PS3D_INAME_PHONG);
		//Graphics::InstanceID id3d = D_PS3D_INS_LAMB_ID;
		Graphics::InstanceID id3d = D_PS3D_INS_PHONG_ID;
		Graphics::PipelineManager::PipelineRegister(D_PIPE3D_S, new Graphics::Pipeline("Copy", "Anisotropic", D_SOFF_ZON, "Cull Front", D_VS3D_S, 0, nullptr, D_PS3D, 1, &id3d));
		Graphics::PipelineManager::PipelineRegister(D_PIPE3D_D, new Graphics::Pipeline("Copy", "Anisotropic", D_SOFF_ZON, "Cull Front", D_VS3D_D, 0, nullptr, D_PS3D, 1, &id3d));
		Graphics::PipelineManager::PipelineRegister(D_PIPE3D_IS, new Graphics::Pipeline("Copy", "Anisotropic", D_SOFF_ZON, "Cull Front", D_VS3D_IS, 0, nullptr, D_PS3D, 1, &id3d));
		Graphics::PipelineManager::PipelineRegister(D_PIPE3D_ID, new Graphics::Pipeline("Copy", "Anisotropic", D_SOFF_ZON, "Cull Front", D_VS3D_ID, 0, nullptr, D_PS3D, 1, &id3d));
	}
	void Bootup() {
		srand(s_cast<unsigned>(time(NULL)));
		if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))STRICT_THROW("COMの初期化に失敗しました");
		//グラフィックドライバ周り
		/*Graphics::GraphicDriverInfoList::Bootup();
		int driverCount = Graphics::GraphicDriverInfoList::GetGraphicDriverCount();
		Graphics::GraphicDriverInfo* driver = nullptr;
		for (int i = 0; i < driverCount; i++) {
			Graphics::GraphicDriverInfo* info = Graphics::GraphicDriverInfoList::GetDriver(i).get();
			if (!driver)driver = info;
			else if (info->GetVideoMemory() > driver->GetVideoMemory())driver = info;
		}*/
		//デバイス作成
		DWORD flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
		flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		Graphics::Device::Create(flag, /*driver*/nullptr);
		//Graphics::GraphicDriverInfoList::Shutdown();
		Graphics::Direct2DSystem::Initialize();
#ifdef USE_IMGUI_AND_CONSOLE
		HostConsole::GetInstance()->Bootup();
#endif
		Audio::Device::Create();
		Audio::MasterVoice::Create();
		Audio::EffectVoice::Create();
		Audio::Sound3DSystem::Setting({}, { 0.0f,0.0f,1.0f });
		CreateDefaultPipeline();
		CreateStencilStatePreset();
		Graphics::GaussianFilter::Setting();
		Network::System::Startup();
		Network::SocketList::Initialize();
		XMLSystem::Initialize();

		Graphics::SpriteRenderer::Initialize();
		Lobelia::Graphics::Environment::GetInstance()->SetAmbientColor(0xFFFFFFFF);
		Lobelia::Math::Vector3 lightDir(1.0f, -1.0f, 1.0f); lightDir.Normalize();
		Lobelia::Graphics::Environment::GetInstance()->SetLightDirection(lightDir);
		Lobelia::Graphics::Environment::GetInstance()->Activate();
		Lobelia::Graphics::EffekseerWrapper::Setting();
	}

	void Shutdown() {
#ifdef USE_IMGUI_AND_CONSOLE
		HostConsole::GetInstance()->Shutdown();
#endif
		Audio::Bank::Clear();
		Audio::EffectVoice::Destroy();
		Audio::MasterVoice::Destroy();
		Audio::Device::Destroy();
		Network::System::Cleanup();
		//Movie::MovieSystem::Shutdown();
		Network::System::Cleanup();
		CoUninitialize();
	}
}
