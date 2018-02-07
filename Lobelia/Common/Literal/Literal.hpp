#pragma once
namespace Lobelia {
	//ì¬’†
	struct Int {
	private:
		int data;
	public:
		Int(int data) :data(data) {}
		Int() = default;
		operator int() { return data; }
		int operator=(int data) { return this->data + data; }
		void Clamp(int min, int max) {
			if (max < min)throw;
			if (data > max) data = max;
			if (data < min) data = min;
		}
		void ClampMin(int min) { if (data < min)data = min; }
		void ClampMax(int max) { if (data > max)data = max; }

	};
}
//using Int = Lobelia::Int;