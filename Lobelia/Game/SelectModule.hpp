#pragma once
namespace Lobelia::Game {
	class SelectModule {
	private:
		int maxSelect;
		int select;
	private:
		void Limitter() {
			if (maxSelect == 0)return;
			while (select < 0)select += maxSelect;
			select %= maxSelect;
		}
	public:
		SelectModule(int select_count) :maxSelect(select_count), select(0) {};
		~SelectModule() = default;
		int GetSelect() { return select; }
		void SetSelect(int select_no) {
			select = select_no;
			Limitter();
		}
		void AddSelectCount() { maxSelect++; }
		void SubSelectCount() {
			maxSelect--;
			Limitter();
		}
		void SetSelectCount(int select_count) {
			maxSelect = select_count;
			Limitter();
		}
		int GetSelectCount() { return maxSelect; }
		void Next() {
			select++;
			Limitter();
		}
		void Previous() {
			select--;
			Limitter();
		}
		int operator++() {
			Next();
			return GetSelect();
		}
		int operator--() {
			Previous();
			return GetSelect();
		}
		operator int() { return GetSelect(); }
	};
}