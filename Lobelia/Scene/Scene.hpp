namespace Lobelia {
	class Scene Interface {
	public:
		/**@brief コンストラクタ*/
		Scene() = default;
		/**@brief シーン遷移元のインスタンスが消えた際に動く*/
		virtual void Initialize() {}
		/**@brief デストラクタ*/
		virtual ~Scene() = default;
		/**@brief 更新処理 純粋仮想関数*/
		virtual void Update() = 0;
		/**@brief 描画処理 純粋仮想関数*/
		virtual void Render() = 0;
	protected:
	
	};
}