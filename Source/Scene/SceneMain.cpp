#include "Lobelia.hpp"
#include "SceneMain.hpp"

namespace Lobelia::Game {
	SceneMain::SceneMain() :view(std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize())) {
	}
	SceneMain::~SceneMain() {
	}
	void SceneMain::Initialize() {
	}
	void SceneMain::Update() {
	}
	void SceneMain::Render() {
		view->Activate();
	}
}
