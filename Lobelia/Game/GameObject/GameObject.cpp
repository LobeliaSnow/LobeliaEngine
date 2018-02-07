#pragma once
#include "Lobelia.hpp"
#include "GameObject.hpp"

namespace Lobelia::Game {
	Math::Vector2 GetWindowSizeBias() {
		Math::Vector2 defaultSize(1280, 720);
		Math::Vector2 nowSize = Lobelia::Application::GetInstance()->GetWindow()->GetSize();
		return nowSize / defaultSize;
	}
	GameObject2D::GameObject2D(const char* file_path, const Graphics::Transform2D& transform, Utility::Color color) :transform(transform), color(color), isExist(true) {
		sprite = std::make_unique<Graphics::Sprite>(file_path);
		script = new ScriptManager();
	}
	GameObject2D::~GameObject2D() {
		Utility::SafeDelete(script);
	}
	void GameObject2D::MouseUpdate() {
		POINT mpos = {};
		GetCursorPos(&mpos);
		ScreenToClient(Application::GetInstance()->GetWindow()->GetHandle(), &mpos);
		Math::Vector2 pos = {};
		pos.x = f_cast(mpos.x);	pos.y = f_cast(mpos.y);
		//TODO : ŽÎ‚ß‚É‚à‘Î‰ž‚Å‚«‚é‚æ‚¤‚ÉC³
		if (Collision::RotationRectToPoint(transform.position*GetWindowSizeBias(), transform.scale*GetWindowSizeBias(), transform.rotation, pos)) {
			OnCursor();
			if (Input::GetMouseKey(0) == 1)OnClickEnter();
			if (Input::GetMouseKey(0) == 3)OnClickStay();
			if (Input::GetMouseKey(0) == 2)OnClickLeave();
		}
	}
	bool GameObject2D::IsExist() { return isExist; }
	void GameObject2D::Kill() { isExist = false; }
	void GameObject2D::OnClickEnter() {
		script->OnClickEnter(this);
	}
	void GameObject2D::OnClickStay() {
		script->OnClickStay(this);
	}
	void GameObject2D::OnClickLeave() {
		script->OnClickLeave(this);
	}
	void GameObject2D::OnCursor() {
		script->OnCursor(this);
	}
	void GameObject2D::Translation(const Math::Vector2& pos) { transform.position = pos; }
	void GameObject2D::TranslationMove(const Math::Vector2& move) { transform.position += move; }
	void GameObject2D::Scalling(const Math::Vector2& scale) { transform.scale = scale; }
	void GameObject2D::Rotation(float rotation) { transform.rotation = rotation; }
	const Graphics::Transform2D& GameObject2D::GetTransform() { return transform; }
	void GameObject2D::SetColor(Utility::Color color) { this->color = color; }
	Utility::Color GameObject2D::GetColor() { return color; }
	void GameObject2D::Update() {
		script->Update(this);
		MouseUpdate();
	}
	void* GameObject2D::GetBuffer() { return s_cast<void*>(buffer); }
	void GameObject2D::Render() {
		sprite->Render(transform, Math::Vector2(), sprite->GetTexture()->GetSize(), color);
	}
	void GameObject2D::LinkScript(class BaseScript* script) {
		this->script->LinkScript(script);
	}

	ScriptManager::ScriptManager() {}
	ScriptManager::~ScriptManager() {
		for each(auto script in scripts) {
			delete script;
		}
	}
	void ScriptManager::OnClickEnter(GameObject2D* object) {
		for each(auto& script in scripts) {
			script->OnClickEnter(object);
		}
	}
	void ScriptManager::OnClickStay(GameObject2D* object) {
		for each(auto& script in scripts) {
			script->OnClickStay(object);
		}
	}
	void ScriptManager::OnClickLeave(GameObject2D* object) {
		for each(auto& script in scripts) {
			script->OnClickLeave(object);
		}
	}
	void ScriptManager::OnCursor(GameObject2D* object) {
		for each(auto& script in scripts) {
			script->OnCursor(object);
		}
	}
	void ScriptManager::Update(GameObject2D* object) {
		for each(auto& script in scripts) {
			script->Update(object);
		}
	}
	void ScriptManager::LinkScript(BaseScript* script) {
		scripts.push_back(script);
	}

	void GameObject2DManager::Update() {
		if (clear) {
			Clear();
			clear = false;
		}
		for (auto& object = objects.begin(); object != objects.end();) {
			(*object)->Update();
			if (!(*object)->IsExist())object = objects.erase(object);
			else object++;
		}
	}
	void GameObject2DManager::Render() {
		for each(auto& object in objects) {
			object->Render();
		}
	}
	void GameObject2DManager::Clear() {
		objects.clear();
	}
	void GameObject2DManager::ClearReserve() { clear = true; }
}
