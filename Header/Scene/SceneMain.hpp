#pragma once
namespace Lobelia::Game {
	class SceneMain :public Lobelia::Scene {
	private:
		std::unique_ptr<Graphics::View> view;
		std::unique_ptr<Graphics::Font> font;
	public:
		SceneMain();
		~SceneMain();
		void Initialize()override;
		void Update()override;
		void Render()override;
	};
}