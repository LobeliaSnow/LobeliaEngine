#pragma once
namespace Lobelia::Game {
	extern Math::Vector2 GetWindowSizeBias();
	//TODO : 描画機構がない基底を作って、継承させて描画機能実装する
	//フォントやインスタンシングに対応するため
	//それに伴い、マネージャーも変更する
	class GameObject2D abstract {
	private:
		std::unique_ptr<Graphics::Sprite> sprite;
		Graphics::Transform2D transform;
		class ScriptManager* script;
		Utility::Color color;
		bool isExist;
		//自由領域
		char buffer[32];
	private:
		void MouseUpdate();
	public:
		GameObject2D(const char* file_path = "", const Graphics::Transform2D& transform = {}, Utility::Color color = 0xFFFFFFFF);
		virtual ~GameObject2D();
		bool IsExist();
		void Kill();
		void OnClickEnter();
		void OnClickStay();
		void OnClickLeave();
		void OnCursor();
		void Translation(const Math::Vector2& pos);
		void TranslationMove(const Math::Vector2& move);
		void Scalling(const Math::Vector2& scale);
		void Rotation(float rotation);
		const Graphics::Transform2D& GetTransform();
		void SetColor(Utility::Color color);
		Utility::Color GetColor();
		void Update();
		void* GetBuffer();
		virtual void Render();
		void LinkScript(class BaseScript* script);
		template<class Object> void LinkScript() {
			script->LinkScript<Object>();
		}
	};
	class BaseScript {
	public:
		BaseScript() = default;
		~BaseScript() = default;
		virtual void OnClickEnter(GameObject2D* object) {}
		virtual void OnClickStay(GameObject2D* object) {}
		virtual void OnClickLeave(GameObject2D* object) {}
		//TODO : マウスが離れた時のイベントも欲しい
		virtual void OnCursor(GameObject2D* object) {}
		virtual void Update(GameObject2D* object) {}
	};
	class ScriptManager {
	private:
		std::list<BaseScript*> scripts;
	public:
		ScriptManager();
		~ScriptManager();
		void OnClickEnter(GameObject2D* object);
		void OnClickStay(GameObject2D* object);
		void OnClickLeave(GameObject2D* object);
		void OnCursor(GameObject2D* object);
		void Update(GameObject2D* object);
		void LinkScript(BaseScript* script);
		template<class Object> void LinkScript() {
			static_assert(std::is_base_of<BaseScript, Object>::value, "error of class type mismatch");
			scripts.push_back(new Object);
		}
	};
	template<class Object> class ScriptLinker :public Utility::Singleton<ScriptLinker<Object>> {
		friend class GameObject2DManager;
	private:
		std::vector<std::function<BaseScript*()>> factory;
	public:
		template<class Script> void LinkScript() {
			factory.push_back([=]() {return new Script; });
		}
		void Clear() { factory.clear(); }
	};
	class GameObject2DManager :public Utility::Singleton<GameObject2DManager> {
		friend class Utility::Singleton<GameObject2DManager>;
	private:
		std::list<std::shared_ptr<GameObject2D>> objects;
		bool clear = false;
	public:
		template<class Object, class... Args> void Create(Args... args) {
			std::shared_ptr<Object> object = std::make_shared<Object>(args...);
			int size = ScriptLinker<Object>::GetInstance()->factory.size();
			for (int i = 0; i < size; i++) {
				object->LinkScript(ScriptLinker<Object>::GetInstance()->factory[i]());
			}
			objects.push_back(object);
		}
		void Update();
		void Render();
		void Clear();
		void ClearReserve();
	};


}