#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//		Character
	//
	//---------------------------------------------------------------------------------------------
	class RayMesh;
	class RayResult;
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
		void GPURaycastFloor();
		void GPURaycastWall();
#endif
	private:
		std::weak_ptr<Graphics::Model> terrain;
#ifdef GPU_RAYCASTER
		std::weak_ptr<RayMesh> rayMesh;
		std::shared_ptr<RayResult> result;
#endif
	};
}
