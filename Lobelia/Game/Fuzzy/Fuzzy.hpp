#pragma once
//ファジー理論を実装するためのヘルパー群
namespace Lobelia::Game {
	//ファジー手順
	//1,マッピングプロセス(ファジー化 クリスプ集合をメンバーシップ関数によるメンバーシップ度算出)
	//2,ルールに基づきファジー出力
	//3,その出力をクリスプ化(非ファジー化)
	//クリスプ化は自分で実装するしかない
	//Lobelia::Mathに可変長でとれるMin関数とMax関数があるのでクリスプ化の際に役立ててほしい

	//メンバーシップ関数
	//入力されるクリスプ値からメンバーシップど算出するためのもの
	class FuzzyMembership {
	public:
		//右肩上がり
		static float Grade(float value, float min, float max);
		//右肩下がり
		static float ReverseGrade(float value, float min, float max);
		//三角形型
		static float Triangle(float value, float min, float center, float max);
		//台形型
		static float Trapezoid(float value, float min, float center_left, float center_right, float max);
	private:
		//線形傾斜を作るためのヘルパー関数
		static float CompareGrade(float value, float min, float max) { return (value / (max - min)) - (min / (max - min)); }
		static float CompareReverseGrade(float value, float min, float max) { return (max / (max - min)) - (value / (max - min)); }
	};
	//メンバーシップ度を調整するためのもの
	class FuzzyHedges {
	public:
		//ただの二乗
		static float Very(float value) { return value*value; }
		//ただの平方根
		static float NotVery(float value) { return sqrtf(value); }
	};
	//ファジールール
	class FuzzyRule {
		//論理積 &&
		static float And(float value0, float value1) { return fminf(value0, value1); }
		//論理和 ||
		static float Or(float value0, float value1) { return fmaxf(value0, value1); }
		//論理否定 !
		static float Not(float value) { return 1.0f - value; }
	};
}