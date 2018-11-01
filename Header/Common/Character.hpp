#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//		Character
	//
	//---------------------------------------------------------------------------------------------
	class RayMesh;
	class RayResult;
	//‰e‚É“o˜^‚·‚é‚Æ‚«‚É–Ê“|‚¾‚©‚çModelƒNƒ‰ƒX‚ğŒp³
	//•’i‚Í‚½‚¹‚éB
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
