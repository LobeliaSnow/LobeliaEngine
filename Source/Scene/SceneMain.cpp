#include "Lobelia.hpp"
#include "SceneMain.hpp"

//TODO : 描画ステートのパイプラインクラスを調整
//TODO : コントローラー入力
//TODO : テクスチャブレンドや雨等
//TODO : トーン入出力処理(DXGI)を実装？
//TODO : deltabaseでの飛び問題解決
//Raypickする際に-1されていることを忘れてはならない。

namespace Lobelia::Game {
	//テクスチャ読み込み用変数(スマポはダメ)
	Graphics::Texture* texture0 = nullptr;
	Graphics::Texture* texture1 = nullptr;
	//スプライトクラス
	std::unique_ptr<Graphics::Sprite> sprite;
	//スプライトバッチクラス このクラスは非推奨
	std::unique_ptr<Graphics::SpriteBatch> batch;
	//操作自機位置
	Math::Vector2 pos(100, 100);

	SceneMain::SceneMain() :view(std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize())) {
		//テクスチャロード
		Graphics::TextureFileAccessor::Load("test.png", &texture0);
		Graphics::TextureFileAccessor::Load("images.jpg", &texture1);

		//スロットは8個、0~7まで使用可能
		Graphics::SpriteBatchRenderer::GetInstance()->SetTexture(0, texture0);
		Graphics::SpriteBatchRenderer::GetInstance()->SetTexture(1, texture1);

		//スプライトクラスとしてテクスチャロード
		sprite = std::make_unique<Graphics::Sprite>("test.png");
		//スプライトバッチクラスとしてテクスチャロード、一度に描画できる最大数が第二引数
		batch = std::make_unique<Graphics::SpriteBatch>("images.jpg", 10);
	}
	SceneMain::~SceneMain() {
	}
	void SceneMain::Initialize() {	}
	void SceneMain::Update() {
		//キー入力とデルタベースプログラムのサンプル
		if (Input::GetKeyboardKey(VK_UP) == 3)pos.y -= 200.0f*Application::GetInstance()->GetProcessTime()*0.001f;
		if (Input::GetKeyboardKey(VK_DOWN) == 3)pos.y += 200.0f*Application::GetInstance()->GetProcessTime()*0.001f;
		if (Input::GetKeyboardKey(VK_LEFT) == 3)pos.x -= 200.0f*Application::GetInstance()->GetProcessTime()*0.001f;
		if (Input::GetKeyboardKey(VK_RIGHT) == 3)pos.x += 200.0f*Application::GetInstance()->GetProcessTime()*0.001f;
		//バッチレンダラクラスへの登録を開始。テクスチャの枚数が足りなくなったらこのクラスを別でインスタンス化して使用する
		Graphics::SpriteBatchRenderer::GetInstance()->Begin();
		//ここの前後関係が描画順に影響
		Graphics::SpriteBatchRenderer::GetInstance()->Set(1, Math::Vector2(50, 50), texture1->GetSize(), 0.0f, Math::Vector2(), texture1->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteBatchRenderer::GetInstance()->Set(0, pos, texture0->GetSize(), 0.0f, Math::Vector2(), texture0->GetSize(), 0xFFFFFFFF);
		Graphics::SpriteBatchRenderer::GetInstance()->End();
	}
	void SceneMain::Render() {
		view->Activate();
		//スプライトレンダラクラスで描画
		Graphics::SpriteRenderer::Render(texture1);
		//バッチレンダラクラスで描画
		Graphics::SpriteBatchRenderer::GetInstance()->Render();
		//スプライトクラスの機能で描画
		sprite->Render(Math::Vector2(), sprite->GetTexture()->GetSize(), 0.0f, Math::Vector2(), sprite->GetTexture()->GetSize(), 0xFFFFFFFF);
		//スプライトバッチクラスへ登録 
		batch->BeginRender();
		//引数めんどい。察して
		batch->Render({}, {}, batch->GetMaterial()->GetTexture()->GetSize(), 0.0f, 0.0f, 0xFFFFFFFF);
		batch->EndRender();

	}
}
