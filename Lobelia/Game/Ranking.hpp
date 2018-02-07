#pragma once
namespace Lobelia::Game {
	//重要！ここですべてのオブジェクトをvoidに帰結
	template <class ...> using void_t = void;
	template<class, class = void> struct is_greater_than : std::false_type {};
	template<class T> struct is_greater_than<T, void_t<decltype(std::declval<T&>() > std::declval<T&>())>> : std::true_type{};
	template<class, class = void> struct is_less_than : std::false_type {};
	template<class T> struct is_less_than<T, void_t<decltype(std::declval<T&>() < std::declval<T&>())>> : std::true_type{};
	//std::sortのラップみたいなもの
	class VectorSort {
	public:
		VectorSort() = delete;
		~VectorSort() = delete;
	public:
		template<class T> static void Desc(std::vector<T>& data) {
			static_assert(is_greater_than<T>::value, "operator>() Unimplemented");
			std::sort(data.begin(), data.end(), std::greater<T>());
		}
		template<class T> static void Asc(std::vector<T>& data) {
			static_assert(is_less_than<T>::value, "operator<() Unimplemented");
			std::sort(data.begin(), data.end());
		}
	};

	//ソートが降順か昇順かの列挙体
	enum class SortOrder :int { DESC, ASC };
	//テンプレート引数のSortOrderによる特殊化
	template <SortOrder order> struct OrderTypeTraits {};
	template<> struct OrderTypeTraits<SortOrder::ASC> { template<class T> const static void Sort(std::vector<T>& data) { VectorSort::Asc<T>(data); }; };
	template<> struct OrderTypeTraits<SortOrder::DESC> { template<class T> const static void Sort(std::vector<T>& data) { VectorSort::Desc<T>(data); }; };
	//ランキングをロードするためのデータ構造体
	template<class T> struct RankingData {
	public:
		std::vector<T> data;
	public:
		RankingData(const char* file_path, int ranking_count, const T default_value = {});
		~RankingData();
		//コンストラクタで呼ばれるので値の更新目的以外では特に呼ばなくてよい
		void Load(const char* file_path, int ranking_count, const T default_value = {});
	};
	template<class T> RankingData<T>::RankingData(const char* file_path, int ranking_count, const T default_value) {
		data.resize(ranking_count);
		Load(file_path, ranking_count, default_value);
	}
	template<class T> RankingData<T>::~RankingData() = default;
	template<class T> void RankingData<T>::Load(const char* file_path, int ranking_count, const T default_value) {
		try {
			std::unique_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
			size_t recodeCount = fc->Read(&data[0], sizeof(T)*ranking_count, sizeof(T), ranking_count);
			if (recodeCount < ranking_count) {
				for (int i = i_cast(recodeCount); i < ranking_count; i++) {
					data[i] = default_value;
				}
			}
			fc->Close();
		}
		catch (...) {
			std::unique_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
			fc->Close();
			for (int i = 0; i < ranking_count; i++) {
				data[i] = default_value;
			}
		}
	}
	//templateに適する条件 
	//operator< >が適切に実装されている
	//非virtual
	//実際にソートしたりするランキング本体
	template<class T, SortOrder order> class Ranking final {
	public:
	private:
		RankingData<T> rankingData;
	public:
		Ranking(const RankingData<T>& rankingData);
		~Ranking();
		const std::vector<T>& Get();
		void Save(const char* file_path);
		void AddData(const T& data);
	};
	template<class T, SortOrder order> Ranking<T, order>::Ranking(const RankingData<T>& rankingData) :rankingData(rankingData) {
		static_assert(!std::has_virtual_destructor<T>::value, "	virtual exists in the member method");
		OrderTypeTraits<order>::Sort(this->rankingData.data);
	}
	template<class T, SortOrder order> Ranking<T, order>::~Ranking() = default;
	template<class T, SortOrder order> const std::vector<T>& Ranking<T, order>::Get() { return rankingData.data; }
	template<class T, SortOrder order> void Ranking<T, order>::Save(const char* file_path) {
		std::unique_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
		fc->Write<T>(&rankingData.data[0], rankingData.data.size());
		fc->Close();
	}
	template<class T, SortOrder order> void Ranking<T, order>::AddData(const T& data) {
		size_t count = rankingData.data.size();
		rankingData.data.push_back(data);
		OrderTypeTraits<order>::Sort(rankingData.data);
		rankingData.data.resize(count);
	}
}