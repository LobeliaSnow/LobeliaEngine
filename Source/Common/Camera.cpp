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
	std::shared_ptr<Graphics::View> Camera::GetView() { return view; }
	Math::Vector3 Camera::TakeFront() {
		Math::Vector3 ret = at - pos; ret.Normalize();
		return ret;
	}
	Math::Vector3 Camera::TakeRight() {
		Math::Vector3 front = TakeFront();
		Math::Vector3 tempUP(0.001f, 1.0f, 0.001f); tempUP.Normalize();
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
		//注視点までの距離
		Math::Vector2 mmove = Input::Mouse::GetInstance()->GetMove();
		rightMove += right * mmove.x * circumference;
		upMove += up * mmove.y * circumference;
		if (wheel)radius -= (wheel*circumference*0.1f);
		if (Input::GetMouseKey(0)) pos += rightMove + upMove;
		if (Input::GetMouseKey(1)) at += rightMove + upMove;
		if (Input::GetMouseKey(2)) {
			pos += rightMove + upMove;
			at += rightMove + upMove;
		}
		if (radius < 2.0f)radius = 2.0f;
		front = TakeFront();
		pos = at - front * radius;
		right = Math::Vector3::Cross(up, front);
		up = Math::Vector3::Cross(front, right);
		//ここ外から追加できるようにしても良いが、外から使う予定もないためこれで
		//カメラをAOが映える場所へ
		if (Input::GetKeyboardKey(DIK_P)) {
			pos = Math::Vector3(57.0f, 66.0f, 106.0f);
			at = Math::Vector3();
			up = Math::Vector3(0.0f, 1.0f, 0.0f);
			radius = (pos - at).Length();
		}
		//ライトが映える場所へ
		if (Input::GetKeyboardKey(DIK_O)) {
			pos = Math::Vector3(-343.0f, 33.0f, -11.0f);
			at = Math::Vector3();
			up = Math::Vector3(0.0f, 1.0f, 0.0f);
			radius = (pos - at).Length();
		}
	}
}