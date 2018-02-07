#pragma once
namespace Lobelia::Input {
	struct MouseData {
		std::array<BYTE, 3> buffer = {};
		Math::Vector2 move = {};
		short wheel = {};
	};
	struct KeyboardData {
		std::array<BYTE, 256> buffer = {};
	};
	struct DualShock4Data {
		std::array<BYTE, 14> buffer = {};
	};
	namespace _ {
		inline void Push(BYTE* dest, const BYTE* source, int count) {
			for (int i = 0; i < count; i++) {
				dest[i] <<= 1;
				if (source[i])dest[i] |= 1;
				else dest[i] |= 0;
			}
		}
	}
}