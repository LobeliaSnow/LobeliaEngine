#pragma once
namespace Lobelia::Game {
	//---------------------------------------------------------------------------------------------
	//
	//		Character
	//
	//---------------------------------------------------------------------------------------------
	class Character :public Actor {
	public:
		Character();
		~Character() = default;
	private:
		std::unique_ptr<Graphics::Model> model;
	};
}
