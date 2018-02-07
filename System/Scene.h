#pragma once
#include <memory>
namespace Hdx {
	/**
	*@file Scene.h
	*@brief シーン基底クラス(インターフェース クラス)
	*@author Lobelia_Snow
	*/

	/**
	*@brief シーンの基底クラス
	*/
	class Scene Interface {
	public:
		/**@brief コンストラクタ*/
		Scene() = default;
		/**@brief デストラクタ*/
		virtual ~Scene() = default;
		/**@brief 初期化処理 純粋仮想関数*/
		virtual bool Initialize() = 0;
		/**@brief 更新処理 純粋仮想関数*/
		virtual void Update() = 0;
		/**@brief 描画処理 純粋仮想関数*/
		virtual void Render() = 0;

	};
}