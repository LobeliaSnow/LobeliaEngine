#include "Lobelia.hpp"
#include "SceneDeferred.hpp"

//シェーダーのコンパイルが長すぎる、デバッグ時以外はcsoに逃がしてやりたい
//その際、現在は1つのシェーダーファイルに複数のエントリポイントがあるのでまとめてコンパイル&出力をするツールを作成したい

//TODO : 影、シェーダー重さ、フォグの濃さ
//TODO : MipFog 調べる
//TODO : SSAOPSの可変解像度対応(ビューを作ってやるだけ？)
//TODO : シェーダーの整理
//TODO : 警告消す
//TODO : defineとフラグが荒れてるので、リファクタリング(constつけれる部分を付けていく作業)
//TODO : GaussianFileterのCS版最適化(1pass処理)(2passのほうが早いみたい?)
//↑参考URL
//https://github.com/Unity-Technologies/PostProcessing/blob/v2/PostProcessing/Shaders/Builtins/GaussianDownsample.compute
//http://sygh.hatenadiary.jp/entry/2014/07/05/194143
//TODO : Temporal Reprojectionでモーションブラーの黒縁を修正
//TODO : マテリアルIDバッファのシャドウバッファとの結合B要素にいれる
//TODO : sRGBフォーマットのテクスチャに切り替え
//TODO : SSSの実装
//TODO : 支援ツールの作成
//TODO : 頂点バッファを複数セットする形式で、スロット1には頂点、スロット2にはUVみたいな感じです
//TODO : 描画部分とデータ部分を切り離す

//Hexa-Driveアドバイス
//そろそろエンジン部分を作り始めれば？
//フォグいらなくない？(または、薄くする)
//影
//Screen Space Reflection
//マテリアルについては、計算モデルを一つ用意して、パラメーターをG-Bufferに書き込むことで制御するのが良い
//つまり、Phongへの係数が0ならそれはランバートになる。よくあるのはメタルネスや、Emission Intensity
//Lambert 1/PI倍できるとよい(半球状に光が散乱するシミュレーション)
//パーティクルも、エミッターを用意してGPU上でそのエミッターから発生させればメモリアクセスが減って高速化ができるよね
//GUIのチェックボックスでオンオフできるようにしたほうが良い。あまりデバッグキーはよくない。

//テストシーン
//最終的にここでの経験と結果を用いてレンダリングエンジンを作る予定ではいるが、就活終わった後になると思われる。
//その際には、GBufferの再利用も考えたほうが良いかも。
//ポストエフェクト実装の際に現在使用しているレンダーターゲットを、外部から受け取る形にすればもう少しマシにはなる

//Define.hのスイッチで一部の機能のスイッチが可能
//現状実装されてるもの一覧(このシーンはディファードシェーディングです)
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
//GPURaycast(当たり判定用 現状実行自体は爆速のはずだが、パイプラインストールの回避ができない。回避にはフレーム遅延させるしかないのでCPU側で頑張る)
//Gaussian Depth of Field (被写界深度)
//Screen Space Motion Blur
//川瀬式ブルーム
//倍率色収差
//指数トーンマップ
//ガンマ補正
//周辺減光
//標準マテリアル追加


namespace Lobelia::Game {
	namespace {
		const constexpr int LIGHT_COUNT = 256;
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
		//デモ用
		useLight = TRUE; useFog = TRUE; useMotionBlur = TRUE; useCamera = true; useDof = true;
		useShadow = true; useVariance = true; renderGBuffer = true; renderShadowMap = true;
		renderSSAOBuffer = true; useSSAO = true; ssaoDepthThreshold = 5.0f; focusRange = 150.0f;
		renderBlumeBuffer = true; chromaticAberrationIntensity = 0.005f; useVignette = true;
		blumeIntensity = 6.0f;	radius2 = 5.0f; smooth = 2.1f; mechanicalScale = 0.25f;
		cosFactor = 0.53f; cosPower = 0.45f; naturalScale = 0.09f;
#ifdef _DEBUG
		HostConsole::GetInstance()->IntRegister("deferred", "use light", &useLight, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use fog", &useFog, false);
		HostConsole::GetInstance()->IntRegister("deferred", "use motion blur", &useMotionBlur, false);
#endif
		//stage = std::make_shared<Graphics::Model>("Data/Model/Deferred/stage.dxd", "Data/Model/Deferred/stage.mt");
		stage = std::make_shared<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
		stage->Translation(Math::Vector3(0.0f, 1.0f, 0.0f));
		//stage->Scalling(3.0f);
		stage->CalcWorldMatrix();
		box = std::make_shared<Graphics::Model>("Data/Model/box.dxd", "Data/Model/box.mt");
		box->Translation(Math::Vector3(0.0f, 5.0f, 0.0f));
		box->Scalling(3.0f);
		//box->Scalling(9.0f);
		box->CalcWorldMatrix();
		stageCollision = std::make_shared<Graphics::Model>("Data/Model/maps/collision.dxd", "Data/Model/maps/collision.mt");
		stageCollision->Translation(Math::Vector3(0.0f, 1.0f, 0.0f));
		stageCollision->CalcWorldMatrix();
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
		shadow = std::make_unique<ShadowBuffer>(Math::Vector2(1024, 1024)*QUALITY, 4, true);
#else
		shadow = std::make_unique<ShadowBuffer>(scale*QUALITY, 1, true);
#endif
#ifdef USE_DOF
		dof = std::make_unique<DepthOfField>(scale, DOF_QUALITY);
		dof->SetFocusRange(150.0f);
#endif
		skybox = std::make_shared<SkyBox>("Data/Model/skybox.dxd", "Data/Model/skybox.mt");
		skybox->SetCamera(camera);
		deferredBuffer->SetSkybox(skybox);
#ifdef USE_CHARACTER
		Raycaster::Initialize();
		character = std::make_shared<Character>();
#ifdef GPU_RAYCASTER
		//レイ関係の初期化
		rayMesh = std::make_shared<RayMesh>(stageCollision.get());
		character->SetTerrainData(rayMesh);
#endif
		character->SetTerrainData(stageCollision);
#endif
		//shadow = std::make_unique<ShadowBuffer>(Math::Vector2(1280, 720), 1, false);
		//gaussian = std::make_unique<GaussianFilterCS>(scale);
		rad = 0.0f;
		shadow->SetNearPlane(1.0f);
		shadow->SetFarPlane(1000.0f);
		shadow->SetLamda(0.8f);
#ifdef USE_MOTION_BLUR
		motionBlur = std::make_unique<SSMotionBlur>(scale);
#endif
#ifdef _DEBUG
		//----------------------------------------------------------------------------------------------------------------------------------
		//
		//			この先デモ用メニュー
		//
		//----------------------------------------------------------------------------------------------------------------------------------
		//ImGuiウインドウを作成
		console = std::make_unique<AdaptiveConsole>("Operation Console");
		//表示内容を構築
		//カメラ
		console->AddFunction([this]() {
			ImGui::Checkbox("Move Camera", &this->useCamera);
			if (ImGui::TreeNode("Camera")) {
				if (ImGui::Button("Reset Position Start")) {
					this->camera->SetPos(Math::Vector3(57.0f, 66.0f, 106.0f));
					this->camera->SetTarget(Math::Vector3());
					this->camera->SetUp(Math::Vector3(0.0f, 1.0f, 0.0f));
				}
				if (ImGui::Button("Reset Position HDR")) {
					this->camera->SetPos(Math::Vector3(-172.0f, 68.0f, -0.90f));
					this->camera->SetTarget(Math::Vector3(-185.5f, 0.0f, -3.0f));
					this->camera->SetUp(Math::Vector3(0.0f, 1.0f, 0.0f));
				}
				if (ImGui::Button("Reset Position Light")) {
					this->camera->SetPos(Math::Vector3(-343.0f, 33.0f, -11.0f));
					this->camera->SetTarget(Math::Vector3(-185.5f, 0.0f, -3.0f));
					this->camera->SetUp(Math::Vector3(0.0f, 1.0f, 0.0f));
				}
				ImGui::TreePop();
			}
		});
		//G-Buffer
		console->AddFunction([this]() {
			ImGui::Checkbox("Debug Render", &Application::GetInstance()->debugRender);
			if (Application::GetInstance()->debugRender && ImGui::TreeNode("G-Buffer")) {
				ImGui::Checkbox("Render G-Buffer", &this->renderGBuffer);
				ImGui::Checkbox("Render AO Buffer", &this->renderSSAOBuffer);
				ImGui::Checkbox("Render Shadow Map", &this->renderShadowMap);
				ImGui::Checkbox("Render Blume & HDR Buffer", &this->renderBlumeBuffer);
				ImGui::TreePop();
			}
		});
		//ブラー
		console->AddFunction([this]() {
			bool blur = this->useMotionBlur;
			ImGui::Checkbox("Use Blur", &blur);
			this->useMotionBlur = blur;
		});
		//影
		console->AddFunction([this]() {
			ImGui::Checkbox("Use Shadow", &this->useShadow);
			if (this->useShadow && ImGui::TreeNode("Shadow")) {
				ImGui::Checkbox("Use Variance", &this->useVariance);
				ImGui::TreePop();
			}
		});
		//SSAO
		console->AddFunction([this] {
			ImGui::Checkbox("Use SSAO", &this->useSSAO);
			if (useSSAO && ImGui::TreeNode("AO")) {
				ImGui::SliderFloat("Deoth Threshold", &ssaoDepthThreshold, 1.0f, 30.0f);
				ImGui::TreePop();
			}
		});
		//被写界深度
		console->AddFunction([this]() {
			ImGui::Checkbox("Use DoF", &this->useDof);
			if (useDof && ImGui::TreeNode("DoF")) {
				ImGui::SliderFloat("Focus Range", &focusRange, 1.0f, 300.0f);
				ImGui::TreePop();
			}
		});
		//フォグ
		console->AddFunction([this]() {
			bool fog = this->useFog;
			ImGui::Checkbox("Use Fog", &fog);
			this->useFog = fog;
			static float fogBegin = 300.0f;
			static float fogEnd = 1000.0f;
			if (this->useFog && ImGui::TreeNode("Fog")) {
				ImGui::SliderFloat("Begin", &fogBegin, 0.0f, 999.0f);
				ImGui::SliderFloat("End", &fogEnd, 1.0f, 1000.0f);
				ImGui::TreePop();
			}
			Graphics::Environment::GetInstance()->SetFogBegin(fogBegin);
			Graphics::Environment::GetInstance()->SetFogEnd(fogEnd);
		});
#ifdef FULL_EFFECT
		//ライト
		console->AddFunction([this]() {
			bool light = this->useLight;
			ImGui::Checkbox("Use Light", &light);
			this->useLight = light;
			static int lightCount = LIGHT_COUNT;
			if (useLight) {
				if (ImGui::TreeNode("Light")) {
					ImGui::SliderInt("count", &lightCount, 0, LIGHT_COUNT);
					if (ImGui::Button("Relocation")) {
						FullEffectDeferred::PointLight light;
						for (int i = 0; i < LIGHT_COUNT; i++) {
							light.pos = Math::Vector4(Utility::Frand(-60.0f, 60.0f), Utility::Frand(-10.0f, 10.0f), Utility::Frand(-40.0f, 40.0f), 0.0f);
							light.pos += Math::Vector4(-213.0f, 5.0f, -5.0f, 0.0f);
							light.color = Utility::Color(rand() % 255, rand() % 255, rand() % 255, 255);
							light.attenuation = Utility::Frand(0.5f, 10.0f);
							deferredShader->SetLightBuffer(i, light);
						}
					}
					ImGui::TreePop();
				}
				deferredShader->SetUseCount(lightCount);
			}
			else deferredShader->SetUseCount(0);
		});
#endif
		//レンズ
		console->AddFunction([this] {
			if (ImGui::TreeNode("Lens")) {
				ImGui::SliderFloat("Chromatic Aberration", &chromaticAberrationIntensity, 0.0f, 0.1f);
				ImGui::Checkbox("Vignette", &useVignette);
				ImGui::SliderFloat("Vignette Radius", &radius2, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Smooth", &smooth, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Mechanical Scale", &mechanicalScale, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Cos Factor", &cosFactor, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Cos Power", &cosPower, 0.0f, 10.0f);
				ImGui::SliderFloat("Vignette Natural Scale", &naturalScale, 0.0f, 10.0f);
				ImGui::TreePop();
			}
		});
		//ブルーム
		console->AddFunction([this] {
			ImGui::SliderFloat("Blume Intensity", &blumeIntensity, 0.0f, 20.0f);
		});

		//----------------------------------------------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------------------------------------------
#endif
	}
	SceneDeferred::~SceneDeferred() {
#ifdef _DEBUG
		HostConsole::GetInstance()->VariableUnRegister("deferred");
#endif
	}
	void SceneDeferred::AlwaysUpdate() {
		if (Input::GetKeyboardKey(DIK_0) == 1) useShadow = !useShadow;
		if (Input::GetKeyboardKey(DIK_2) == 1) useVignette = !useVignette;
		if (Input::GetKeyboardKey(DIK_3) == 1) useMotionBlur = !useMotionBlur;
		if (Input::GetKeyboardKey(DIK_4) == 1) useDof = !useDof;
		if (Input::GetKeyboardKey(DIK_5) == 1) useLight = !useLight;
		if (Input::GetKeyboardKey(DIK_6) == 1) useFog = !useFog;
		if (Input::GetKeyboardKey(DIK_9) == 1) useVariance = !useVariance;
		shadow->SetEnable(useShadow);
		shadow->SetVariance(useVariance);
		deferredBuffer->AddModel(stage, DeferredBuffer::MATERIAL_TYPE::LAMBERT, 1.0f, 1.0f);
		deferredBuffer->AddModel(box, DeferredBuffer::MATERIAL_TYPE::COLOR, 0.0f, blumeIntensity);
		//deferredBuffer->AddModel(stageCollision, normalMap);
#ifdef USE_CHARACTER
		deferredBuffer->AddModel(character, false);
#endif
#ifdef FULL_EFFECT
		deferredShader->Update();
#endif
		shadow->AddModel(box);
		shadow->AddModel(stage);
#ifdef USE_CHARACTER
		shadow->AddModel(character);
#endif
		//回転ライト
		rad += Application::GetInstance()->GetProcessTimeSec()*0.1f;
		lpos = Math::Vector3(sinf(rad), 0.0f, cos(rad))*100.0f;
		//lpos.x = lpos.z = 0.0f;
		lpos.y = 100.0f;
		//固定ライト
		//lpos = Math::Vector3(200.0f, 130.0f, 200.0f);
		shadow->SetPos(lpos);
		shadow->SetTarget(Math::Vector3());
		//shadow->SetPos(pos);
		if (useCamera)camera->Update();
#ifdef USE_DOF
		dof->SetEnable(useDof);
		dof->SetFocusRange(focusRange);
#endif
#ifdef USE_SSAO
		ssao->SetEnable(useSSAO);
		ssao->SetThresholdDepth(ssaoDepthThreshold);
#endif
		deferredShader->EnableVignette(useVignette);
		deferredShader->SetChromaticAberrationIntensity(chromaticAberrationIntensity);
		deferredShader->SetRadius2(radius2);
		deferredShader->SetSmooth(smooth);
		deferredShader->SetMechanicalScale(mechanicalScale);
		deferredShader->SetCosFactor(cosFactor);
		deferredShader->SetCosPower(cosPower);
		deferredShader->SetNaturalScale(naturalScale);
	}
	void SceneDeferred::AlwaysRender() {
		//Graphics::Environment::GetInstance()->SetLightDirection(-Math::Vector3(1.0f, 1.0f, 1.0f));
		Graphics::Environment::GetInstance()->SetActiveLinearFog(useFog);
		Graphics::Environment::GetInstance()->Activate();
		camera->Activate();
		rt->Clear(0xFF000000);
		Graphics::RenderTarget* backBuffer = rt.get();
		//shadow->CreateShadowMap(view.get(), backBuffer);
		shadow->CreateShadowMap(camera->GetView().get(), backBuffer);
		//view->Activate();
		shadow->Begin();
		deferredBuffer->RenderGBuffer();
		backBuffer->Activate();
#ifdef USE_SSAO
#ifdef SSAO_PS
		ssao->CreateAO(backBuffer, deferredBuffer.get());
#endif
#ifdef SSAO_CS
		ssao->CreateAO(backBuffer, camera->GetView().get(), deferredBuffer.get());
#endif
		ssao->Begin(11);
#endif
#ifdef USE_CHARACTER
		Math::Vector3 front = camera->TakeFront(); front.y = 0.0f;
		character->Update(front);
#endif
#ifdef USE_HDR
		//そのうちスカイボックスはこの中で持たせるようにするかも
		deferredShader->RenderHDR(camera->GetView().get(), backBuffer, deferredBuffer.get());
#else
		deferredShader->Render(deferredBuffer.get());
#endif
		shadow->End();
#ifdef USE_SSAO
		ssao->End();
#endif
#ifdef USE_DOF
		dof->Dispatch(camera->GetView().get(), backBuffer, rt.get(), deferredBuffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS).get());
#else
		//Graphics::SpriteRenderer::Render(rt.get());
		//Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
#endif
		//バックバッファをスワップチェインのものに変更
		if (!useMotionBlur) {
			backBuffer = Application::GetInstance()->GetSwapChain()->GetRenderTarget();
			backBuffer->Activate();
			//skybox->Render();
#ifndef USE_DOF
			Graphics::SpriteRenderer::Render(rt.get());
			Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
#endif
		}
#ifdef USE_DOF
		dof->Render();
#endif
		if (useMotionBlur) {
#ifdef USE_MOTION_BLUR
			motionBlur->Dispatch(backBuffer->GetTexture(), deferredBuffer->GetRenderTarget(DeferredBuffer::BUFFER_TYPE::VIEW_POS)->GetTexture());
#endif
			backBuffer = Application::GetInstance()->GetSwapChain()->GetRenderTarget();
			backBuffer->Activate();
			//skybox->Render();
#ifdef USE_MOTION_BLUR
			motionBlur->Render();
#else
			Graphics::SpriteRenderer::Render(rt.get());
			Graphics::Texture::Clean(0, Graphics::ShaderStageList::PS);
#endif
		}

#ifdef _DEBUG
		console->Update();
		if (Application::GetInstance()->debugRender) {
			if (renderGBuffer)deferredBuffer->DebugRender();
			if (renderShadowMap)shadow->DebugRender();
#ifdef USE_SSAO
			if (renderSSAOBuffer)ssao->Render();
#endif
#ifdef USE_HDR
			if (renderBlumeBuffer) deferredShader->DebugRender();
#endif
		}
#endif
		//今回のフレーム結果を保存
		camera->GetView()->FrameEnd();
	}
}