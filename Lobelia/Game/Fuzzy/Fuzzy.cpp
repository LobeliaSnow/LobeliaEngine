#include "Common/Common.hpp"
#include "Fuzzy.hpp"

namespace Lobelia::Game {
	float FuzzyMembership::Grade(float value, float min, float max) {
		float result = 0.0f;
		if (value <= min)result = 0.0f;
		else if (value >= max)result = 1.0f;
		else result = CompareGrade(value, min, max);
		return result;
	}
	float FuzzyMembership::ReverseGrade(float value, float min, float max) {
		float result = 0.0f;
		if (value <= min)result = 1.0f;
		else if (value >= max)result = 0.0f;
		else result = CompareReverseGrade(value, min, max);
		return result;
	}
	float FuzzyMembership::Triangle(float value, float min, float center, float max) {
		float result = 0.0f;
		if (value <= min || value >= max)result = 0.0f;
		else if (value == center)result = 1.0f;
		else if (value > min&&value < center)result = CompareGrade(value, min, center);
		else result = CompareReverseGrade(value, center, max);
		return result;
	}
	float FuzzyMembership::Trapezoid(float value, float min, float center_left, float center_right, float max) {
		float result = 0;
		if (value <= min || value >= max) result = 0;
		else if ((value >= center_left) && (value <= center_right)) result = 1;
		else if ((value > min) && (value < center_left)) result = Grade(value, min, center_left);
		else result = ReverseGrade(value, center_right, max);
		return result;
	}
}