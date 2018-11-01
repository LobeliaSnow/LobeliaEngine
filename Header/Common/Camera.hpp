#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//		Camera
	//
	//---------------------------------------------------------------------------------------------
	class Camera {
	public:
		Camera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at);
		virtual ~Camera() = default;
		void SetPos(const Math::Vector3& pos);
		void SetTarget(const Math::Vector3& at);
		std::shared_ptr<Graphics::View> GetView();
		Math::Vector3 TakeFront();
		Math::Vector3 TakeRight();
		Math::Vector3 TakeUp();
		virtual void Update() {}
		void Activate();
	protected:
		std::shared_ptr<Graphics::View> view;
		Math::Vector3 pos;
		Math::Vector3 at;
		Math::Vector3 up;
	};
	//デバッグ用なので実装めちゃくちゃ
	class ViewerCamera :public Camera {
	public:
		ViewerCamera(const Math::Vector2& scale, const Math::Vector3& pos, const Math::Vector3& at);
		void Update()override;
	private:
		float radius;
		Math::Vector3 front;
		Math::Vector3 right;
	};

}