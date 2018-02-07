#include "Lobelia.hpp"

namespace Lobelia::Game {
	bool Collision::RotationRectToPoint(const Math::Vector2& r_pos, const Math::Vector2& r_size, float r_rad, const Math::Vector2& p_point) {
		float sin = sinf(-r_rad);
		float cos = cosf(-r_rad);
		Math::Vector2 halfSize = r_size * 0.5f;
		Math::Vector2 rectCenter = r_pos + halfSize;
		//矩形の中心を原点と見立てる
		//原点から見た点の位置
		Math::Vector2 relativePos = p_point - rectCenter;
		//原点中心に回転させた点
		Math::Vector2 rotatePos;
		rotatePos.x = relativePos.x*cos - relativePos.y*sin;
		rotatePos.y = relativePos.x*sin + relativePos.y*cos;
		//座標をもとに戻す
		rotatePos += rectCenter;
		//矩形判定
		return (rectCenter.x - halfSize.x < rotatePos.x && rectCenter.x + halfSize.x > rotatePos.x && rectCenter.y - halfSize.y < rotatePos.y&& rectCenter.y + halfSize.y > rotatePos.y);
	}
}