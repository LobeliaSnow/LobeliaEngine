#pragma once
namespace Lobelia::Graphics {
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//  importer周り
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class DxdImporter {
	private:
		struct Vertex {
			Math::Vector4 pos;
			Math::Vector4 normal;
			Math::Vector2 tex;
		};
		struct Mesh {
			int indexCount;
			int uvCount;
			std::vector<Vertex> vertices;
			int materialNameLength;
			std::string materialName;
		};
		struct Bone {
			bool isEntity;
			struct Info {
				int clusterIndex;
				float weight;
			};
			//頂点数->影響度
			std::vector<std::vector<Info>> infos;
			std::vector<DirectX::XMMATRIX> initPoseMatrices;
		};
	private:
		int meshCount;
		std::vector<Mesh> meshes;
		//ボーンが存在するか否か
		std::vector<int> clusterCount;
		//TODO : 名前おかしい。改善せよ。
		//メッシュ数
		std::vector<Bone> meshsBoneInfo;
	private:
		void MeshLoad(std::weak_ptr<Utility::FileController> file);
		void VertexLoad(std::weak_ptr<Utility::FileController> file);
		void SkinLoad(std::weak_ptr<Utility::FileController> file);
		void ClusterLoad(std::weak_ptr<Utility::FileController> file, int mesh_index);
	public:
		DxdImporter(const char* file_path);
		~DxdImporter();
		//TODO : アクセサー追加
		int GetMeshCount();
		const std::vector<Mesh>& GetMeshes();
		Mesh& GetMesh(int index);
		int GetBoneCount(int mesh_index);
		const std::vector<Bone>& GetMeshsBoneInfos();
		const Bone& GetMeshBoneInfo(int index);
	};
	class MaterialImporter {
	private:
		struct Material {
			int nameLength;
			std::string name;
			int textureNameLength;
			std::string textureName;
		};
	private:
		int materialCount;
		std::vector<Material> materials;
	private:
		void Load(std::weak_ptr<Utility::FileController> file);
	public:
		MaterialImporter(const char* file_path);
		~MaterialImporter();
		int GetMaterialCount();
		const std::vector<Material>& GetMaterials();
		const Material& GetMaterial(int index);
	};
	class AnimationImporter {
	private:
		struct Info {
			int clusetCount;
			//キーフレーム数
			struct ClusterFrames { std::vector<DirectX::XMFLOAT4X4> keyFrames; };
			//クラスター数
			std::vector<ClusterFrames> clusterFrames;
		};
	private:
		int nameLength;
		std::string name;
		int framePerSecond;
		int keyFrameCount;
		int meshCount;
		//メッシュ数
		std::vector<Info> infos;
	private:
		void LoadName(std::weak_ptr<Utility::FileController> file);
		void SettingLoad(std::weak_ptr<Utility::FileController> file);
		void KeyFramesLoad(std::weak_ptr<Utility::FileController> file);
	public:
		AnimationImporter(const char* file_path);
		~AnimationImporter();
		const std::string& GetName();
		int GetSampleFramePerSecond();
		int GetKeyFrameCount();
		int GetMeshCount();
		const std::vector<Info>& GetInfos();
		const Info& GetInfo(int index);
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//  Animation
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//時間の関係上ほぼ既存の流用
	//もう少し時間のある時に分けること
	class Animation {
		struct Constant {
			enum KEY_FRAME { MAX = 256 };
			//思ったけどXMMATRIXでよくね？
			DirectX::XMFLOAT4X4 keyFrame[KEY_FRAME::MAX] = {};
		};
		//メッシュ->クラスター->フレーム
		using KeyFrames = std::vector<std::vector<std::vector<DirectX::XMFLOAT4X4>>>;
	private:
		std::string name;
		int framePerCount;
		int frameCount;
		int meshCount;
		std::vector<int> clusterCount;
		KeyFrames keyFrames;
		Constant buffer;
		std::unique_ptr<ConstantBuffer<Constant>> constantBuffer;
		float time;
	public:
		Animation(const char* file_path);
		~Animation();
		void AddElapsedTime(float time);
		void ResetTime();
		void Update(int meshIndex);
		//アニメーション名取得
		const std::string& GetName();
	};
	//とりあえず今はusing、状況次第で構造体やクラスに代わる
	using AnimationNo = int;
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//  ModelData
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//これはModelクラスの読み込み部分だけコピペしてきたもの
	//そのうち統一化を図りたい
	struct ModelData {
	public:
		struct Vertex {
			Math::Vector4 pos;
			Math::Vector4 normal;
			Math::Vector2 tex;
			UINT clusteIndex[4];
			Math::Vector4 weights;
		};
		struct Subset {
			//サブセットのインデックス
			int index;
			//開始頂点番号
			int start;
			//合計頂点数
			int sum;
			std::string materialName;
		};
		struct Bone {
			int clusterCount;
			//クラスター数
			std::vector<DirectX::XMMATRIX> initPoseMatrices;
		};
		struct MaterialInfo {
			std::string name;
			std::string texturePath;
		};
	public:
		int allMeshVertexCountSum;
		std::vector<Vertex> vertices;
		std::vector<Bone> bones;
		std::vector<Subset> subsets;
		int materialCount;
		std::vector<MaterialInfo> materials;
	private:
		//モデルデータ構築関連
		void ConfigureVertex(std::weak_ptr<DxdImporter> dxd);
		void ConfigureBones(std::weak_ptr<DxdImporter> dxd, int mesh_index, int* vertex_index);
		void ConfigureMaterial(std::weak_ptr<MaterialImporter> mt, const std::string& directory);
	public:
		ModelData(const char* dxd_path, const char* mt_path);
		~ModelData();
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//  AnimationData
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//これはAnimationクラスの読み込み部分だけコピペしてきたもの
	//そのうち統一化を図りたい
	struct AnimationData {
		friend class InstancingAnimationEngine;
	private:
		//メッシュ->クラスター->フレーム
		using KeyFrames = std::vector<std::vector<std::vector<DirectX::XMFLOAT4X4>>>;
	private:
		std::string name;
		int framePerCount;
		int frameCount;
		int meshCount;
		std::vector<int> clusterCount;
		KeyFrames keyFrames;
	public:
		AnimationData(const char* file_path);
		~AnimationData();
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//  Model
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class Model {
	private:
		struct Vertex {
			Math::Vector4 pos;
			Math::Vector4 normal;
			Math::Vector2 tex;
			UINT clusteIndex[4];
			Math::Vector4 weights;
		};
		struct Subset {
			//サブセットのインデックス
			int index;
			//開始頂点番号
			int start;
			//合計頂点数
			int sum;
			//試していない
			bool ThisIsMyVertex(int vertex_index)const { return (s_cast<UINT>(vertex_index - start) > s_cast<UINT>(sum - start)); }
			//描画
			void Render(Model* model) {
				if (!model->renderIndexMaterial[index]->IsVisible())return;
				model->renderIndexMaterial[index]->Set();
				Device::GetContext()->Draw(sum, start);
			}
		};
		struct Bone {
			int clusterCount;
			//クラスター数
			std::vector<DirectX::XMMATRIX> initPoseMatrices;
		};
	private:
		std::unique_ptr<Mesh<Vertex>> mesh;
		std::map<std::string, std::shared_ptr<Material>> materials;
		std::vector<Material*> renderIndexMaterial;
		std::unique_ptr<InputLayout> inputLayout;
		std::unique_ptr<ConstantBuffer<DirectX::XMMATRIX>> constantBuffer;
		//メッシュ数
		std::vector<Bone> bones;
		std::vector<Subset> subsets;
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX translate;
		DirectX::XMMATRIX scalling;
		DirectX::XMMATRIX rotation;
		int allMeshVertexCountSum;
		Transform3D transform;
		AnimationNo activeAnimation;
		//アニメーション
		std::vector<std::unique_ptr<Animation>> animations;
		int animationCount;
		Model* parent;
	private:
		//非ポインター変数初期化
		void StateInitialize();
		//モデルデータ構築関連
		void ConfigureVertex(std::weak_ptr<DxdImporter> dxd);
		void ConfigureBones(std::weak_ptr<DxdImporter> dxd, int mesh_index, int* vertex_index);
		void ConfigureMaterial(std::weak_ptr<MaterialImporter> mt, const std::string& directory);
	private:
		//transformから移動行列生成
		void CalcTranslateMatrix();
		//transformから拡縮行列生成
		void CalcScallingMatrix();
	public:
		Model(const char* dxd_path, const char* mt_path);
		~Model();
		//親子関係構築(予定)
		void LinkParent(Model* model);
		void UnLinkParent();
		//描画情報更新関連
		void SetTransformAndCalcMatrix(const Transform3D& transform);
		const Transform3D& GetTransform();
		//移動 行列計算も行われます
		void Translation(const Math::Vector3& pos);
		void Translation(float x, float y, float z);
		void TranslationMove(const Math::Vector3& move);
		void TranslationMove(float x, float y, float z);
		//回転 行列計算も行われます
		void RotationQuaternion(const DirectX::XMVECTOR& quaternion);
		void RotationAxis(const Math::Vector3& axis, float rad);
		void RotationRollPitchYow(const Math::Vector3& rpy);
		void RotationYAxis(float rad);
		//拡縮 行列計算も行われます
		void Scalling(const Math::Vector3& scale);
		void Scalling(float x, float y, float z);
		void Scalling(float scale);
		//更新処理
		void CalculationWorldMatrix();
		//行列取得
		void GetTranslateMatrix(DirectX::XMMATRIX* translate);
		void CalcInverseTranslateMatrix(DirectX::XMMATRIX* inv_translate);
		void GetScallingMatrix(DirectX::XMMATRIX* scalling);
		void CalcInverseScallingMatrix(DirectX::XMMATRIX* inv_scalling);
		void GetRotationMatrix(DirectX::XMMATRIX* rotation);
		void CalcInverseRotationMatrix(DirectX::XMMATRIX* inv_rotation);
		void GetWorldMatrix(DirectX::XMMATRIX* world);
		void CalcInverseWorldMatrix(DirectX::XMMATRIX* inv_world);
		//アニメーション関連
		AnimationNo AnimationLoad(const char* file_path);
		void AnimationActivate(AnimationNo index);
		void AnimationUnActive();
		const std::string& GetAnimationName(AnimationNo index);
		void AnimationUpdate(float elapsed_time);
		//マテリアル取得
		Material* GetMaterial();
		//描画関連
		void Render(bool no_set = false);
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//	Transformer
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//これはModelクラスの行列部分だけコピペしてきたもの
	//そのうち統一化を図りたい
	class Transformer {
	private:
		Transform3D transform;
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX translate;
		DirectX::XMMATRIX scalling;
		DirectX::XMMATRIX rotation;
	private:
		//transformから移動行列生成
		void CalcTranslateMatrix();
		//transformから拡縮行列生成
		void CalcScallingMatrix();
	public:
		Transformer();
		//移動 行列計算も行われます
		void Translation(const Math::Vector3& pos);
		void Translation(float x, float y, float z);
		void TranslationMove(const Math::Vector3& move);
		void TranslationMove(float x, float y, float z);
		//回転 行列計算も行われます
		void RotationQuaternion(const DirectX::XMVECTOR& quaternion);
		void RotationAxis(const Math::Vector3& axis, float rad);
		void RotationRollPitchYow(const Math::Vector3& rpy);
		void RotationYAxis(float rad);
		//拡縮 行列計算も行われます
		void Scalling(const Math::Vector3& scale);
		void Scalling(float x, float y, float z);
		void Scalling(float scale);
		//更新処理
		void CalculationWorldMatrix();
		//行列取得
		void GetTranslateMatrix(DirectX::XMMATRIX* translate);
		void CalcInverseTranslateMatrix(DirectX::XMMATRIX* inv_translate);
		void GetScallingMatrix(DirectX::XMMATRIX* scalling);
		void CalcInverseScallingMatrix(DirectX::XMMATRIX* inv_scalling);
		void GetRotationMatrix(DirectX::XMMATRIX* rotation);
		void CalcInverseRotationMatrix(DirectX::XMMATRIX* inv_rotation);
		//転置行列を返します
		void GetWorldMatrix(DirectX::XMMATRIX* t_world);
		void GetWorldMatrixTranspose(DirectX::XMMATRIX* world);
		void CalcInverseWorldMatrix(DirectX::XMMATRIX* inv_world);
		const Transform3D& GetTransform();
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//	ModelInstanced
	///////////////////////////////////////////////////////////////////////////////////////////////////
	struct InstancedData {
		DirectX::XMMATRIX world;
	};
	class InstancingEngine {
	private:
		const int INSTANCE_MAX;
		ComPtr<ID3D11Buffer> buffer;
	public:
		InstancingEngine(const int instance_count);
		~InstancingEngine();
		//これでデータの変更を行う
		InstancedData* Map();
		void Unmap();
		ComPtr<ID3D11Buffer>& GetBuffer();
		const int GetInstanceMax();
	};
	//現在はスキンメッシュは非対応
	class ModelInstanced {
	private:
		struct Vertex {
			Math::Vector4 pos;
			Math::Vector4 normal;
			Math::Vector2 tex;
		};
		struct Subset {
			//サブセットのインデックス
			int index;
			//開始頂点番号
			int start;
			//合計頂点数
			int sum;
			//試していない
			bool ThisIsMyVertex(int vertex_index)const { return (s_cast<UINT>(vertex_index - start) > s_cast<UINT>(sum - start)); }
			//描画
			void Render(ModelInstanced* model) {
				if (!model->renderIndexMaterial[index]->IsVisible())return;
				model->renderIndexMaterial[index]->Set();
				//Device::GetContext()->DrawInstancedIndirect(sum, start);
				Device::GetContext()->DrawInstanced(sum, model->renderCount, start, 0);
			}
		};
		struct Bone {
			int clusterCount;
			//クラスター数
			std::vector<DirectX::XMMATRIX> initPoseMatrices;
		};
	private:
		ModelData * modelData;
		std::unique_ptr<InstancingEngine> instancingEngine;
		std::unique_ptr<Mesh<Vertex>> mesh;
		std::map<std::string, std::shared_ptr<Material>> materials;
		std::vector<Material*> renderIndexMaterial;
		std::unique_ptr<InputLayout> inputLayout;
		//メッシュ数
		std::vector<Bone> bones;
		std::vector<Subset> subsets;
		InstancedData* mapBuffer;
		int renderCount;
	public:
		ModelInstanced(const char* dxd_path, const char* mt_path, const int instance_count);
		~ModelInstanced();
		void Begin();
		void Set(const InstancedData& data);
		void End();
		void Render();
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// InstancingAnimationEngine
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class InstancingAnimationEngine {
	private:
		const int INSTANCE_MAX;
		std::vector<AnimationData*> animationData;
		int animationCount;
		int clusterMax;
		std::unique_ptr<Texture> animationFrames;
		std::vector<float*> times;
		std::vector<int> animationIndex;

	public:
		InstancingAnimationEngine(const int instance_max);
		~InstancingAnimationEngine();
		AnimationNo LoadAnimation(const char* anm_path);
		void CreateTexture();
		void SetData(int instance_index, int animation_index, float* times);
		void TextureWriteFrames(int mesh_index, int render_count);
		void Activate();
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// ModelInstancedAnimation
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ModelInstancedAnimation {
	private:
		struct Vertex {
			Math::Vector4 pos;
			Math::Vector4 normal;
			Math::Vector2 tex;
			UINT clusteIndex[4];
			float weights[4];
		};
		struct Subset {
			//サブセットのインデックス
			int index;
			//開始頂点番号
			int start;
			//合計頂点数
			int sum;
			//試していない
			bool ThisIsMyVertex(int vertex_index)const { return (s_cast<UINT>(vertex_index - start) > s_cast<UINT>(sum - start)); }
			//描画
			void Render(ModelInstancedAnimation* model) {
				if (!model->renderIndexMaterial[index]->IsVisible())return;
				model->renderIndexMaterial[index]->Set();
				model->animationEngine->TextureWriteFrames(index, model->renderCount);
				Device::GetContext()->DrawInstanced(sum, model->renderCount, start, 0);
			}
		};
		struct Bone {
			int clusterCount;
			//クラスター数
			std::vector<DirectX::XMMATRIX> initPoseMatrices;
		};
	private:
		std::unique_ptr<InstancingEngine> instancingEngine;
		std::unique_ptr<InstancingAnimationEngine> animationEngine;
		std::unique_ptr<Mesh<Vertex>> mesh;
		std::map<std::string, std::shared_ptr<Material>> materials;
		std::vector<Material*> renderIndexMaterial;
		std::unique_ptr<InputLayout> inputLayout;
		//メッシュ数
		std::vector<Bone> bones;
		std::vector<Subset> subsets;
		InstancedData* mapBuffer;
		int renderCount;
	public:
		ModelInstancedAnimation(const char* dxd_path, const char* mt_path, const int instance_count);
		~ModelInstancedAnimation();
		AnimationNo LoadAnimation(const char* anm_path);
		void CreateAnimationFramesTexture();
		void Begin();
		void Set(const InstancedData& data, int animationIndex, float* time);
		void End();
		void Render();
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
}
