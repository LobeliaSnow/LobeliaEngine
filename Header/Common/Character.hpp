#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//		Character
	//
	//---------------------------------------------------------------------------------------------
	//影に登録するときに面倒だからModelクラスを継承
	//普段は持たせる。
	class Character :public Actor, public Graphics::Model {
	public:
		Character();
		~Character() = default;
		void SetTerrainData(std::shared_ptr<Graphics::Model> terrain);
#ifdef GPU_RAYCASTER
		void SetTerrainData(std::shared_ptr<RayMesh> terrain);
#endif
		void Update(const Math::Vector3& front);
#ifdef GPU_RAYCASTER
	private:
		void GPURaycastFloor1Pass();
		void GPURaycastFloor2Pass();
		void GPURaycastWall();
#endif
	private:
		std::weak_ptr<Graphics::Model> terrain;
#ifdef GPU_RAYCASTER
		std::weak_ptr<RayMesh> rayMesh;
		std::unique_ptr<RayResult> result;
#endif
	};
}
