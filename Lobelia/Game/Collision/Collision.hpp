#pragma once
namespace Lobelia::Game {
	class Collision {
	public:
		static bool RotationRectToPoint(const Math::Vector2& r_pos, const Math::Vector2& r_size, float r_rad, const Math::Vector2& p_point);
	};
}