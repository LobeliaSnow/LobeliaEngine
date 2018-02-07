#pragma once
namespace Lobelia::Graphics {
	struct Transform2D {
		Math::Vector2 position;
		Math::Vector2 scale;
		float rotation;
	};
	struct Transform3D {
		Math::Vector3 position;
		Math::Vector3 scale;
		Math::Vector3 rotation;
	};
}