#include "Lobelia.hpp"
//現在失敗しているので使わない
#include "../Data/ShaderFile/Define.h"
#ifdef __PARABOLOID__
//没になった実装
#include "ParaboloidEnvironmentMap.hpp"
#endif
#include "SceneSea.hpp"

//屈折ようやく思いついた。
//Screen Space Distortionの応用でScreen Space Refractを実装。
//アルゴリズムはまんまDistortionだけど、歪ませる際に使用するのを屈折ベクトルで代用
//現状の問題点はuvが画面外に言った時破綻

//後海面の動き何とかすれば終了

namespace Lobelia::Game {
	CubeEnvironmentMap::CubeEnvironmentMap(const Math::Vector2& size, const Math::Vector3& pos, float radius) :size(size), pos(pos), radius(radius) {
		for (int i = 0; i < 6; i++) {
			views[i] = std::make_unique<Graphics::View>(Math::Vector2(), size, PI / 2.0f, 1.0f, radius);
		}
		rt = std::make_unique<Graphics::RenderTarget>(size, DXGI_SAMPLE_DESC{ 1, 0 }, DXGI_FORMAT_R32G32B32A32_FLOAT, 6);
		info.isLighting = false;
	}
	Math::Vector3 CubeEnvironmentMap::GetPos() { return pos; }
	float CubeEnvironmentMap::GetRadius() { return radius; }
	Math::Vector2 CubeEnvironmentMap::GetTextureSize() { return size; }
	void CubeEnvironmentMap::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void CubeEnvironmentMap::SetRadius(float radius) { this->radius = radius; }
	void CubeEnvironmentMap::UpdateInfo() {
		static const Math::Vector3 lookDir[6] = { { 1.0f, 0.0f, 0.0f },{ -1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f, -1.0f } };
		static const Math::Vector3 upDir[6] = { { 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } };
		for (int i = 0; i < 6; i++) {
			views[i]->SetEyePos(pos);
			views[i]->SetEyeTarget(pos + lookDir[i]);
			views[i]->SetEyeUpDirection(pos + upDir[i]);
			views[i]->Update();
			DirectX::XMMATRIX mat = views[i]->GetColumnViewMatrix();
			DirectX::XMStoreFloat4x4(&info.views[i], mat);
		}
		DirectX::XMMATRIX mat = views[0]->GetColumnProjectionMatrix();
		DirectX::XMStoreFloat4x4(&info.projection, mat);
	}
	void CubeEnvironmentMap::Clear(Utility::Color color) { rt->Clear(color); }
	std::shared_ptr<Graphics::RenderTarget> CubeEnvironmentMap::GetRenderTarget() { return rt; }
	CubeInfo& CubeEnvironmentMap::GetCubeInfo() { return info; }
	void CubeEnvironmentMap::Activate() {
		//ビューポートをアクティブに
		views[0]->ViewportActivate();
		//テクスチャのクリア(自分のレンダーターゲットがセットされている可能性があるため)
		Graphics::Texture::Clean(4, Graphics::ShaderStageList::PS);
		rt->Clear(0x00000000);
		rt->Activate();
	}

	CubeEnvironmentMapManager::CubeEnvironmentMapManager() {
		//シェーダーのロード
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/Sea.hlsl", "VS_CREATE_CUBE", Graphics::VertexShader::Model::VS_5_0);
		gs = std::make_shared<Graphics::GeometryShader>("Data/ShaderFile/3D/Sea.hlsl", "GS_CREATE_CUBE");
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/Sea.hlsl", "PS_CREATE_CUBE", Graphics::PixelShader::Model::PS_5_0);
		//コンスタントバッファの作成
		constantBuffer = std::make_unique<Graphics::ConstantBuffer<CubeInfo>>(7, Graphics::ShaderStageList::GS);
	}
	CubeEnvironmentMapManager::~CubeEnvironmentMapManager() {	}
	void CubeEnvironmentMapManager::AddModelList(std::weak_ptr<Graphics::Model> model, bool lighting) { models.push_back(std::make_pair(model, lighting)); }
	//環境マップへの書き込み
	void CubeEnvironmentMapManager::RenderEnvironmentMap() {
		if (environmentMaps.empty())return;
		std::shared_ptr<Graphics::VertexShader> defaultVS = Graphics::Model::GetVertexShader();
		std::shared_ptr<Graphics::PixelShader> defaultPS = Graphics::Model::GetPixelShader();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		gs->Set();
		for (auto&& map = environmentMaps.begin(); map != environmentMaps.end();) {
			//期限切れ
			if (map->expired()) {
				map = environmentMaps.erase(map);
				continue;
			}
			std::shared_ptr<CubeEnvironmentMap> cube = map->lock();
			bool renderFlag = false;
			for (auto&& model = models.begin(); model != models.end();) {
				if (model->first.expired()) {
					model = models.erase(model);
					continue;
				}
				std::shared_ptr<Graphics::Model> modelInstance = model->first.lock();
				//適応範囲にいるか否か、離れたオブジェクトは反射に含めない
				if ((cube->GetPos() - modelInstance->GetTransform().position).Length() > cube->GetRadius()) {
					model++;
					continue;
				}
				//情報の更新 描画される対象がいない時の更新を防ぐため
				if (!renderFlag) {
					cube->UpdateInfo();
					cube->Activate();
					renderFlag = true;
				}
				//この先描画されることが確定
				CubeInfo& info = cube->GetCubeInfo();
				info.isLighting = static_cast<int>(model->second);
				constantBuffer->Activate(info);
				//モデルの描画
				modelInstance->Render();
				model++;
			}
			map++;
		}
		//後片付け
		Graphics::Model::ChangeVertexShader(defaultVS);
		Graphics::Model::ChangePixelShader(defaultPS);
		Graphics::GeometryShader::Clean();
		//デフォルトのレンダーターゲットへ戻す
		Application::GetInstance()->GetSwapChain()->GetRenderTarget()->Activate();
		//モデルリストをフラッシュ
		ClearModelList();
	}
	//戻り値を受けなければ内部でもクリーンされる
	//受取先が生存している間のみ内部で生存する
	std::shared_ptr<CubeEnvironmentMap> CubeEnvironmentMapManager::CreateEnvironmentMap(const Math::Vector2& size, const Math::Vector3& pos, float radius) {
		std::shared_ptr<CubeEnvironmentMap> map = std::make_shared<CubeEnvironmentMap>(size, pos, radius);
		environmentMaps.push_back(map);
		return map;
	}
	//一番近い影響下にある環境マップをテクスチャスロット4にセットする
	//TODO :現状、描画対象が範囲内に収まっていなくても選択される可能性があるのでそれがなくなるように
	//TODO :AABBでの判定に切り替えたりできるようにしたい。
	void CubeEnvironmentMapManager::Activate(const Math::Vector3& pos) {
		struct Info {
			float distance = 1000.0f;
			std::shared_ptr<CubeEnvironmentMap> dualParaboloid;
		};
		Info info;
		for (auto&& map = environmentMaps.begin(); map != environmentMaps.end();) {
			if (map->expired()) {
				map = environmentMaps.erase(map);
				continue;
			}
			std::shared_ptr<CubeEnvironmentMap> dualParaboloid = map->lock();
			float distance = (dualParaboloid->GetPos() - pos).Length();
			if (info.distance > distance) {
				info.distance = distance;
				info.dualParaboloid = dualParaboloid;
			}
			map++;
		}
		if (info.dualParaboloid) info.dualParaboloid->GetRenderTarget()->GetTexture()->Set(4, Graphics::ShaderStageList::PS);
	}
	void CubeEnvironmentMapManager::ClearModelList() { models.clear(); }

	WaterShader::WaterShader() :displacement(nullptr), normal(nullptr) {
		//シェーダーのロード
		vs = std::make_shared<Graphics::VertexShader>("Data/ShaderFile/3D/Sea.hlsl", "VS", Graphics::VertexShader::Model::VS_5_0);
		hs = std::make_shared<Graphics::HullShader>("Data/ShaderFile/3D/Sea.hlsl", "HS");
		ds = std::make_shared<Graphics::DomainShader>("Data/ShaderFile/3D/Sea.hlsl", "DS");
		gs = std::make_shared<Graphics::GeometryShader>("Data/ShaderFile/3D/Sea.hlsl", "GS");
		ps = std::make_shared<Graphics::PixelShader>("Data/ShaderFile/3D/Sea.hlsl", "PS", Graphics::PixelShader::Model::PS_5_0);
		//コンスタントバッファ作成
		constantBufferSeaInfo = std::make_unique<Graphics::ConstantBuffer<SeaInfo>>(6, Graphics::ShaderStageList::HS | Graphics::ShaderStageList::DS | Graphics::ShaderStageList::PS);
		//値の設定
		seaInfo.min = 0.0f;
		seaInfo.max = 70.0f;
		seaInfo.maxDevide = 64.0f;
		seaInfo.height = 1.0f;
		seaInfo.time = 0.0f;
		seaInfo.reflectiveRatio = 0.2f;
		//メニューに値を追加
		HostConsole::GetInstance()->FloatRegister("Sea Info", "min dist", &seaInfo.min, false);
		HostConsole::GetInstance()->FloatRegister("Sea Info", "max dist", &seaInfo.max, false);
		HostConsole::GetInstance()->FloatRegister("Sea Info", "max devide", &seaInfo.maxDevide, false);
		HostConsole::GetInstance()->FloatRegister("Sea Info", "height", &seaInfo.height, false);
		HostConsole::GetInstance()->FloatRegister("Sea Info", "reflectiveRatio", &seaInfo.reflectiveRatio, false);
	}
	void WaterShader::LoadDisplacementMap(const char* file_path) {
		Graphics::TextureFileAccessor::Load(file_path, &displacement);
	}
	void WaterShader::LoadNormalMap(const char* file_path) {
		Graphics::TextureFileAccessor::Load(file_path, &normal);
	}
	void WaterShader::Activate(std::shared_ptr<Graphics::Model> model) {
		//シェーダーの変更＆セット
		model->ChangeVertexShader(vs);
		model->ChangePixelShader(ps);
		hs->Set(); ds->Set(); gs->Set();
		//時間経過
		seaInfo.time += Application::GetInstance()->GetProcessTimeSec();
		//コンスタントバッファとテクスチャの更新
		constantBufferSeaInfo->Activate(seaInfo);
		displacement->Set(1, Graphics::ShaderStageList::DS);
		normal->Set(3, Graphics::ShaderStageList::PS);
	}
	void WaterShader::Clean() {
		//他で使わないシェーダーをもとに戻す
		hs->Clean(); ds->Clean(); gs->Clean();
	}

	SceneSea::SceneSea() {
	}
	SceneSea::~SceneSea() {
		//ステートをもとに戻す
		waterShader->Clean();
		Graphics::Model::ChangeVertexShader(vs);
		Graphics::Model::ChangePixelShader(ps);
		HostConsole::GetInstance()->VariableUnRegister("Sea Info");
	}
	void SceneSea::Initialize() {
		//カメラ作成
		camera = std::make_unique<ViewerCamera>(Application::GetInstance()->GetWindow()->GetSize(), Math::Vector3(0.0f, 10.0f, -10.0f), Math::Vector3());
		rt = std::make_unique<Graphics::RenderTarget>(Application::GetInstance()->GetWindow()->GetSize(), DXGI_SAMPLE_DESC{ 1,0 });
		//バックバッファと深度バッファを共通化
		Application::GetInstance()->GetSwapChain()->GetRenderTarget()->ChangeDepthStencil(rt.get());
		//モデル読み込み、ワールド変換行列作成
		model = std::make_unique<Graphics::Model>("Data/Model/plane.dxd", "Data/Model/plane.mt");
		//model = std::make_unique<Graphics::Model>("Data/Model/sphere.dxd", "Data/Model/sphere.mt");
		model->ChangeBlendState(std::make_shared<Graphics::BlendState>(Graphics::BLEND_PRESET::COPY, true, false));
		//model->RotationRollPitchYow(Math::Vector3(-PI / 5.0f, 0.0, 0.0f));
		//model->Scalling(3.0f);
		model->CalcWorldMatrix();
		stage = std::make_unique<Graphics::Model>("Data/Model/stage.dxd", "Data/Model/stage.mt");
		//stage = std::make_unique<Graphics::Model>("Data/Model/stage2.dxd", "Data/Model/stage2.mt");
		//stage = std::make_unique<Graphics::Model>("Data/Model/maps/stage.dxd", "Data/Model/maps/stage.mt");
		//stage->Translation(Math::Vector3(0.0f, -1.0f, 0.0f));
		//stage->RotationRollPitchYow(Math::Vector3(0.0f, PI, 0.0f));
		//stage->Scalling(3.0f);
		stage->CalcWorldMatrix();
		skyBox = std::make_unique<Graphics::Model>("Data/Model/skybox.dxd", "Data/Model/skybox.mt");
		skyBox->Scalling(10.0f);
		skyBox->CalcWorldMatrix();
		character = std::make_shared<Character>();
		character->Scalling(0.015f);
		character->CalcWorldMatrix();
		//デフォルトのシェーダーの退避
		vs = Graphics::Model::GetVertexShader();
		ps = Graphics::Model::GetPixelShader();
		//ラスタライザ差分作成
		wireState = std::make_shared<Graphics::RasterizerState>(Graphics::RASTERIZER_PRESET::FRONT, true);
		solidState = model->GetRasterizerState();
		//水シェーダー周り読み込み
		waterShader = std::make_unique<WaterShader>();
		waterShader->LoadDisplacementMap("Data/Model/displacement.png");
		waterShader->LoadNormalMap("Data/Model/normal.png");
		Graphics::TextureFileAccessor::Load("Data/Model/caustics.png", &caustics);
		//waterShader->LoadDisplacementMap("Data/Model/displacementTest.png");
		//waterShader->LoadNormalMap("Data/Model/normalTest.png");
#ifdef __PARABOLOID__
		environmentManager = std::make_unique<DualParaboloidMapManager>();
		paraboloidMap = environmentManager->CreateEnvironmentMap(Math::Vector2(512, 512), Math::Vector3(0.0f, 5.0f, 0.0f), 100.0f);
#else
		environmentManager = std::make_unique<CubeEnvironmentMapManager>();
		cubeMap = environmentManager->CreateEnvironmentMap(Math::Vector2(512, 512), Math::Vector3(0.0f, 7.0f, 0.0f), 100.0f);
#endif
		//値の設定
		wire = FALSE; sea = TRUE;
		//メニューに値を追加
		HostConsole::GetInstance()->IntRegister("Sea Info", "wire", &wire, false);
		HostConsole::GetInstance()->IntRegister("Sea Info", "sea", &sea, false);
	}
	void SceneSea::AlwaysUpdate() {
		//カメラ移動
		//情報算出
		Math::Vector3 front = camera->TakeFront();
		Math::Vector3 right = camera->TakeRight();
		float elapsedTime = Application::GetInstance()->GetProcessTimeSec();
		camera->Update();
		front.y = 0.0f; front.Normalize();
		right.y = 0.0f; right.Normalize();
		//モデルのセット
		environmentManager->AddModelList(skyBox, false);
		//この先水面下のキャラクター
		Math::Vector3 characterMove;
		//前後
		if (Input::GetKeyboardKey(DIK_W)) characterMove += front * elapsedTime;
		if (Input::GetKeyboardKey(DIK_S)) characterMove -= front * elapsedTime;
		//左右
		if (Input::GetKeyboardKey(DIK_D)) characterMove += right * elapsedTime;
		if (Input::GetKeyboardKey(DIK_A)) characterMove -= right * elapsedTime;
		characterMove.Normalize();
		if (Input::GetKeyboardKey(DIK_SPACE) == 1)character->Jump(10);
		character->SetMove(characterMove, 5.0f);
		character->CalcMove();
		character->Gravity(-60.0f);
		character->CollisionTerrainFloor(stage.get(), nullptr);
		character->CollisionTerrainWall(stage.get(), nullptr);
		character->Move();
		character->SmoothRotateY();
		character->RotationYAxis(character->GetRad());
		character->Translation(character->GetPos());
		character->CalcWorldMatrix();
		character->AnimationUpdate(Application::GetInstance()->GetProcessTimeMili());
		//environmentManager->AddModelList(stage, true);
	}
	void SceneSea::AlwaysRender() {
		//シェーダーをデフォルトに
		stage->ChangeVertexShader(vs);
		stage->ChangePixelShader(ps);
		//環境マップを作成
		environmentManager->RenderEnvironmentMap();
		camera->Activate();
		rt->Clear(0x00000000);
		rt->Activate();
		//事前に確保したインスタンスインデックス1番でライティングをオフに
		Graphics::Model::GetPixelShader()->SetLinkage(1);
		skyBox->Render();
		//ライティングをオンに
		Graphics::Model::GetPixelShader()->SetLinkage(0);
		//ステージの描画
		stage->Render();
		character->Render();
		//バックバッファの有効か
		Application::GetInstance()->GetSwapChain()->GetRenderTarget()->Activate();
		//背景シーンの描画
		Graphics::SpriteRenderer::Render(rt.get());
		rt->GetTexture()->Set(5, Graphics::ShaderStageList::PS);
		//海として描画するか否か
		if (sea) {
			//テッセレーション用
			topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
			waterShader->Activate(model);
		}
		else {
			topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			model->ChangeVertexShader(vs);
			model->ChangePixelShader(ps);
		}
		//ワイヤーフレームか否か
		if (wire) model->ChangeRasterizerState(wireState);
		else model->ChangeRasterizerState(solidState);
		//環境マップのセット
		environmentManager->Activate(model->GetTransform().position);
		//caustics->Set(5, Graphics::ShaderStageList::PS);
		//水描画
		model->Render(topology);
		//ステートを戻す
		waterShader->Clean();
#if defined(_DEBUG)&&defined(__PARABOLOID__)
		environmentManager->DebugRender();
#endif
		Graphics::Texture::Clean(5, Graphics::ShaderStageList::PS);
	}
}