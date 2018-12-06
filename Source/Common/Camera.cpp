#include "Lobelia.hpp"
#include "Common/Camera.hpp"

namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
//
//		Camera
//
//---------------------------------------------------------------------------------------------
	Camera::Camera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at) :pos(pos), at(at) {
		//複数のビューを使わない前提 使いたい場合は引数でとる
		view = std::make_shared<Graphics::View>(Math::Vector2(), scale);
		up = TakeUp();
	}
	void Camera::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void Camera::SetTarget(const Math::Vector3& at) { this->at = at; }
	void Camera::SetUp(const Math::Vector3& up) { this->up = up; }
	std::shared_ptr<Graphics::View> Camera::GetView() { return view; }
	Math::Vector3 Camera::TakeFront() {
		Math::Vector3 ret = at - pos; ret.Normalize();
		return ret;
	}
	Math::Vector3 Camera::TakeRight() {
		Math::Vector3 front = TakeFront();
		Math::Vector3 tempUP(0.00f, 1.0f, 0.00f); tempUP.Normalize();
		Math::Vector3 ret = Math::Vector3::Cross(tempUP, front); /*ret.Normalize();*/
		return ret;
	}
	Math::Vector3 Camera::TakeUp() {
		Math::Vector3 tempFront = TakeFront();
		Math::Vector3 tempRight = TakeRight();
		Math::Vector3 ret = Math::Vector3::Cross(tempFront, tempRight); /*ret.Normalize();*/
		return ret;
	}
	void Camera::Activate() {
		view->FrameEnd();
		view->SetEyePos(pos);
		view->SetEyeTarget(at);
		view->SetEyeUpDirection(up);
		view->Activate();
	}
	ViewerCamera::ViewerCamera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at) :Camera(scale, pos, at) {
		radius = (pos - at).Length();
		front = TakeFront();
		right = TakeRight();
	}
	void ViewerCamera::Update() {
		float elapsedTime = Application::GetInstance()->GetProcessTimeSec();
		Math::Vector3 rightMove = {};
		Math::Vector3 upMove = {};
		//直径
		float circumference = radius * PI;
		//ちょうどいい感じに動く割合算出
		circumference /= 600.0f;
		//マウス
		float wheel = Input::Mouse::GetInstance()->GetWheel();
		Math::Vector2 mmove = Input::Mouse::GetInstance()->GetMove();
		rightMove += right * mmove.x * circumference;
		upMove += up * mmove.y * circumference;
		if (wheel)radius -= (wheel*circumference*0.1f);
		if (Input::GetMouseKey(0)) pos += rightMove + upMove;
		if (Input::GetMouseKey(1)) {
			at += rightMove + upMove;
			front = TakeFront();
			at = pos + front * radius;
		}
		if (Input::GetMouseKey(2)) {
			pos += rightMove + upMove;
			at += rightMove + upMove;
		}
		if (radius < 2.0f)radius = 2.0f;
		front = TakeFront();
		pos = at - front * radius;
		right = Math::Vector3::Cross(up, front);
		up = Math::Vector3::Cross(front, right);
	}
	void ViewerCamera::SetPos(const Math::Vector3& pos) {
		this->pos = pos;
		radius = (pos - at).Length();
	}
	void ViewerCamera::SetTarget(const Math::Vector3& at) {
		this->at = at;
		radius = (pos - at).Length();
	}
}