#include "Lobelia.hpp"
#include "SceneDeferred.hpp"

//Screen Space Motion Blurやってみたい

//シェーダーのコンパイルが長すぎる、デバッグ時以外はcsoに逃がしてやりたい感

//TODO : SSAOPSの可変解像度対応(ビューを作ってやるだけ？)
//TODO : シェーダーの整理
//TODO : 警告消す

//テストシーン
//最終的にここでの経験と結果を用いてレンダリングエンジンを作る予定ではいるが、就活終わった後になると思われる。
//その際には、GBufferの再利用も考えたほうが良いかも。
//今は何も考えずに湯水のごとくGBufferを使用している。
//ポストエフェクト実装の際に現在使用しているレンダーターゲットを、外部から受け取る形にすればもう少しマシにはなる

//現状実装されてるもの一覧(このシーンはディファードシェーディングです)
//Define.hのスイッチで一部の機能のスイッチが可能
//ハーフランバート
//線形フォグ
//法線マップ
//複数ポイントライト(最適化無し)
//SSAOPS 遅い
//SSAOCS PSより早い
//ガウスフィルタPS
//ガウスフィルタCS 現状遅い 最適化不足
//シャドウマップ
//バリアンスシャドウマップ
//カスケードシャドウマップ
//カスケードバリアンスシャドウマップ(カスケードシャドウマップと、バリアンスシャドウマップの合わせ技)
//GPURaycast(当たり判定用 現状実行自体は爆速のはずだが、どこかがボトルネックになり激遅)
//Gaussian Depth of Field (被写界深度)

namespace Lobelia::Game {
	namespace {
		const constexpr int LIGHT_COUNT = 128;
	}
	//---------------------------------------------------------------------------------------------
	//
	//		Scene
	//
	//---------------------------------------------------------------------------------------------
	void SceneDeferred::Initialize() {
		Math::Vector2 scale = Application::GetInstance()->GetWindow()->GetSize();
		camera = std::make_shared<ViewerCamera>(scale, Math::Vector3(57.0f, 66.0f, 106.0f), Math::Vector3(0.0f, 0.0f, 0.0f));
		rt = std::make_unique<Graphics::RenderTarget>(scale, DXGI_SAMPLE_DESC{ 1,0 });
		deferredBuffer = std::make_unique<DeferredBuffer>(scale);
		normalMap = TRUE; useLight = TRUE; useFog = TRUE;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "normal map", &normalMap, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use light", &useLight, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use fog", &useFog, false);
#endif
		//model = std::make_shared<Graphics::Model>("Data/Model/Deferred/stage.dxd", "Data/Model/Deferred/stage.mt");
		stage = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
		stage->Translation(Math::Vector3(0.0f, 1.0f, 0.0f));
		stage->CalcWorldMatrix();
#ifdef SIMPLE_SHADER
		deferredShader = std::make_unique<SimpleDeferred>();
#endif
#ifdef FULL_EFFECT
		//光源設置
		deferredShader = std::make_unique<FullEffectDeferred>();
		FullEffectDeferred::PointLight light;
		for (int i = 0; i < LIGHT_COUNT; i++) {
			light.pos = Math::Vector4(Utility::Frand(-60.0f, 60.0f), Utility::Frand(-10.0f, 10.0f), Utility::Frand(-40.0f, 40.0f), 0.0f);
			light.pos += Math::Vector4(-213.0f, 5.0f, -5.0f, 0.0f);
			light.color = Utility::Color(rand() % 255, rand() % 255, rand() % 255, 255);
			light.attenuation = Utility::Frand(0.5f, 10.0f);
			deferredShader->SetLightBuffer(i, light);
		}
#endif
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao = std::make_unique<SSAOPS>(scale);
#endif
#ifdef SSAO_CS
		ssao = std::make_unique<SSAOCS>(scale*(QUALITY > 1.0f ? 1.0f : QUALITY));
#endif
#endif
#ifdef CASCADE
		shadow = std::make_unique<ShadowBuffer>(scale*QUALITY, 4, true);
#else
		shadow = std::make_unique<ShadowBuffer>(scale*QUALITY, 1, true);
#endif
#ifdef USE_DOF
		dof = std::make_unique<DepthOfField>(scale, DOF_QUALITY);
		dof->SetFocus(150.0f);
#endif
		skybox = std::make_unique<SkyBox>("Data/Model/skybox.dxd", "Data/Model/skybox.mt");
		skybox->SetCamera(camera);
#ifdef USE_CHARACTER
		Raycaster::Initialize();
		character = std::make_shared<Character>();
#ifdef GPU_RAYCASTER
		//レイ関係の初期化
		rayMesh = std::make_shared<RayMesh>(stage.get());
		character->SetTerrainData(rayMesh);
#endif
		character->SetTerrainData(stage);
#endif
		//shadow = std::make_unique<ShadowBuffer>(Math::Vector2(1280, 720), 1, false);
		//gaussian = std::make_unique<GaussianFilterCS>(scale);
		rad = 0.0f;
		shadow->SetNearPlane(10.0f);
		shadow->SetFarPlane(500.0f);
		shadow->SetLamda(1.0f);

	}
	SceneDeferred::~SceneDeferred() {
#ifdef _DEBUG
		HostConsole::GetInstance()->VariableUnRegister("deferred");
#endif
	}
	void SceneDeferred::AlwaysUpdate() {
		if (Input::GetKeyboardKey(DIK_7) == 1) normalMap = !normalMap;
		if (Input::GetKeyboardKey(DIK_6) == 1) useFog = !useFog;
		if (Input::GetKeyboardKey(DIK_5) == 1) useLight = !useLight;

		deferredBuffer->AddModel(stage, normalMap);
#ifdef USE_CHARACTER
		deferredBuffer->AddModel(character, false);
#endif
#ifdef FULL_EFFECT
		if (useLight)deferredShader->SetUseCount(LIGHT_COUNT);
		else deferredShader->SetUseCount(0);
		deferredShader->Update();
#endif
		shadow->AddModel(stage);
#ifdef USE_CHARACTER
		shadow->AddModel(character);
#endif
		//回転ライト
		//rad += Application::GetInstance()->GetProcessTimeSec()*0.1f;
		//lpos = Math::Vector3(sinf(rad), 0.0f, cos(rad))*300.0f;
		//lpos.y = 150.0f;
		//固定ライト
		lpos = Math::Vector3(200.0f, 130.0f, 200.0f);
		shadow->SetPos(lpos);
		//shadow->SetPos(pos);
		camera->Update();
#ifdef USE_CHARACTER
		Math::Vector3 front = camera->TakeFront(); front.y = 0.0f;
		character->Update(front);
#endif
	}
	void SceneDeferred::AlwaysRender() {
		Graphics::Environment::GetInstance()->SetLightDirection(-Math::Vector3(1.0f, 1.0f, 1.0f));
		Graphics::Environment::GetInstance()->SetActiveLinearFog(useFog);
		Graphics::Environment::GetInstance()->SetFogBegin(150.0f);
		Graphics::Environment::GetInstance()->SetFogEnd(400.0f);
		Graphics::Environment::GetInstance()->Activate();
		camera->Activate();
		rt->Clear(0x00000000);
		Graphics::RenderTarget* backBuffer = rt.get();
		//shadow->CreateShadowMap(view.get(), backBuffer);
		shadow->CreateShadowMap(camera->GetView().get(), backBuffer);
		skybox->Render();
		//view->Activate();
		shadow->Begin();
		deferredBuffer->RenderGBuffer();
		shadow->End();
		backBuffer->Activate();
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao->CreateAO(backBuffer, deferredBuffer.get());
#endif
#ifdef SSAO_CS
		ssao->CreateAO(backBuffer, camera->GetView().get(), deferredBuffer.get());
#endif
		ssao->Begin(5);
#endif
		deferredBuffer->Begin();
		deferredShader->Render();
		deferredBuffer->End();
#ifdef USE_SSAO
		ssao->End();
#endif
		//バックバッファをスワップチェインのものに変更
		backBuffer = Application::GetInstance()->GetSwapChain()->GetRenderTarget();
		backBuffer->Activate();
#ifdef USE_DOF
		dof->Dispatch(camera->GetView().get(), backBuffer, rt.get(), deferredBuffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS).get());
		dof->Render();
#else
		Graphics::SpriteRenderer::Render(rt.get());
		Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
#endif
#ifdef _DEBUG
		if (Application::GetInstance()->debugRender) {
			deferredBuffer->DebugRender();
			shadow->DebugRender();
#ifdef USE_SSAO
			ssao->Render();
#endif
		}
#endif
	}
}