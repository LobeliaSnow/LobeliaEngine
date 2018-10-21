#pragma once
//***************************************************************************************************
//このあたり一度すべて書き直す
//***************************************************************************************************
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
	};
	class MaterialImporter {
	private:
		struct Material {
			int nameLength;
			std::string name;
			int textureNameLength;
			std::string textureName;
		};
	public:
		MaterialImporter(const char* file_path);
		~MaterialImporter();
		int GetMaterialCount();
		const std::vector<Material>& GetMaterials();
		const Material& GetMaterial(int index);
	private:
		int materialCount;
		std::vector<Material> materials;
	private:
		void Load(std::weak_ptr<Utility::FileController> file);
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
	public:
		AnimationImporter(const char* file_path);
		~AnimationImporter();
		const std::string& GetName();
		int GetSampleFramePerSecond();
		int GetKeyFrameCount();
		int GetMeshCount();
		const std::vector<Info>& GetInfos();
		const Info& GetInfo(int index);
	private:
		void LoadName(std::weak_ptr<Utility::FileController> file);
		void SettingLoad(std::weak_ptr<Utility::FileController> file);
		void KeyFramesLoad(std::weak_ptr<Utility::FileController> file);
	private:
		int nameLength;
		std::string name;
		int framePerSecond;
		int keyFrameCount;
		int meshCount;
		//メッシュ数
		std::vector<Info> infos;
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
	public:
		Animation(const char* file_path);
		~Animation();
		void AddElapsedTime(float time);
		void ResetTime();
		void Update(int meshIndex);
		//アニメーション名取得
		const std::string& GetName();
		float GetMaxTime();
		void CalcAnimationMatrix(int bone_index, DirectX::XMFLOAT4X4* anim);
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
		ModelData(const char* dxd_path, const char* mt_path);
		~ModelData();
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
	public:
		AnimationData(const char* file_path);
		~AnimationData();
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
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//  Model
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class Model :public RenderableObject<Model> {
	public:
		struct Vertex {
			Math::Vector4 pos;
			Math::Vector4 normal;
			Math::Vector2 tex;
			UINT clusteIndex[4];
			Math::Vector4 weights;
		};
	private:
		struct Subset {
			//サブセットのインデックス
			int index;
			//開始頂点番号
			int start;
			//合計頂点数
			int sum;
			//試していない
			bool ThisIsMyVertex(int vertex_index)const { return (s_cast<UINT>(vertex_index * 3 - start) < s_cast<UINT>(sum - start)); }
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
		ALIGN(16) struct Info {
			DirectX::XMFLOAT4X4 world;
			int useAnimation;
		};
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
		void CalcWorldMatrix();
		//行列取得
		void GetTranslateMatrix(DirectX::XMMATRIX* translate);
		void CalcInverseTranslateMatrix(DirectX::XMMATRIX* inv_translate);
		void GetScallingMatrix(DirectX::XMMATRIX* scalling);
		void CalcInverseScallingMatrix(DirectX::XMMATRIX* inv_scalling);
		void GetRotationMatrix(DirectX::XMMATRIX* rotation);
		void CalcInverseRotationMatrix(DirectX::XMMATRIX* inv_rotation);
		void GetWorldMatrix(DirectX::XMMATRIX* world);
		void CalcInverseWorldMatrix(DirectX::XMMATRIX* inv_world);
		//ボーン行列回り
		int GetBoneCount();
		const DirectX::XMMATRIX& GetBone(int index);
		//アニメーション関連
		AnimationNo AnimationLoad(const char* file_path);
		void AnimationActivate(AnimationNo index);
		void AnimationUnActive();
		const std::string& GetAnimationName(AnimationNo index);
		void AnimationUpdate(float elapsed_time);
		std::string GetMaterialName(int poly_index);
		float GetAnimationTime(AnimationNo index);
		void CalcAnimationMatrix(AnimationNo index, int bone_index, DirectX::XMFLOAT4X4* anim);
		//マテリアル取得
		Material* GetMaterial(const char* mt_name);
		//描画関連
		void Render(D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, bool no_set = false);
		int RayPickWorld(Math::Vector3* out_pos, Math::Vector3* out_normal, const Math::Vector3& ray_pos, const Math::Vector3& ray_vec, float dist);
		int RayPickLocal(Math::Vector3* out_pos, Math::Vector3* out_normal, const Math::Vector3& ray_pos, const Math::Vector3& ray_vec, float dist);
		Mesh<Vertex>* GetMesh() { return mesh.get(); }
		//即席
		void ChangeAnimVS(std::shared_ptr<Graphics::VertexShader> vs);
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
	private:
		std::unique_ptr<Mesh<Vertex>> mesh;
		std::map<std::string, std::shared_ptr<Material>> materials;
		std::vector<Material*> renderIndexMaterial;
		std::unique_ptr<ConstantBuffer<Info>> constantBuffer;
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
		std::shared_ptr<VertexShader> vsAnim;
		int boneCount;
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
	public:
		InstancingEngine(const int instance_count);
		~InstancingEngine();
		//これでデータの変更を行う
		InstancedData* Map();
		void Unmap();
		ComPtr<ID3D11Buffer>& GetBuffer();
		const int GetInstanceMax();
	private:
		const int INSTANCE_MAX;
		ComPtr<ID3D11Buffer> buffer;
	};
	class ModelInstanced :public RenderableObject<ModelInstanced> {
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
	public:
		ModelInstanced(const char* dxd_path, const char* mt_path, const int instance_count);
		~ModelInstanced();
		void Begin();
		void Set(const InstancedData& data);
		void End();
		void Render();
		std::map<std::string, std::shared_ptr<Material>> GetMaterials() { return materials; }
	private:
		ModelData * modelData;
		std::unique_ptr<InstancingEngine> instancingEngine;
		std::unique_ptr<Mesh<Vertex>> mesh;
		std::map<std::string, std::shared_ptr<Material>> materials;
		std::vector<Material*> renderIndexMaterial;
		//メッシュ数
		std::vector<Bone> bones;
		std::vector<Subset> subsets;
		InstancedData* mapBuffer;
		int renderCount;
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// InstancingAnimationEngine
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class InstancingAnimationEngine {
	public:
		InstancingAnimationEngine(const int instance_max);
		~InstancingAnimationEngine();
		AnimationNo LoadAnimation(const char* anm_path);
		float AnimationTimeMax(AnimationNo animation_index);
		void CreateTexture();
		void SetData(int instance_index, int animation_index, float* times);
		void TextureWriteFrames(int mesh_index, int render_count);
		void CalcAnimationMatrix(AnimationNo animation_index, float time, int bone_index, DirectX::XMFLOAT4X4* anim);
		void Activate();
	private:
		const int INSTANCE_MAX;
		std::vector<AnimationData*> animationData;
		int animationCount;
		int clusterMax;
		std::unique_ptr<Texture> animationFrames;
		std::vector<float*> times;
		std::vector<int> animationIndex;
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// ModelInstancedAnimation
	///////////////////////////////////////////////////////////////////////////////////////////////////
	class ModelInstancedAnimation :public RenderableObject<ModelInstancedAnimation> {
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
	public:
		ModelInstancedAnimation(const char* dxd_path, const char* mt_path, const int instance_count);
		~ModelInstancedAnimation();
		AnimationNo LoadAnimation(const char* anm_path);
		void CreateAnimationFramesTexture();
		float GetAnimationTimeMax(AnimationNo animation_index);
		void CalcAnimationMatrix(AnimationNo animation_index, float time, int bone_index, DirectX::XMFLOAT4X4* anim);
		void Begin();
		//time部分要調整
		void Set(const InstancedData& data, int animationIndex, float* time);
		void End();
		void Render();
	private:
		std::unique_ptr<InstancingEngine> instancingEngine;
		std::unique_ptr<InstancingAnimationEngine> animationEngine;
		std::unique_ptr<Mesh<Vertex>> mesh;
		std::map<std::string, std::shared_ptr<Material>> materials;
		std::vector<Material*> renderIndexMaterial;
		//メッシュ数
		std::vector<Bone> bones;
		std::vector<Subset> subsets;
		InstancedData* mapBuffer;
		int renderCount;
	};
	class MultiTextureModel :public ModelInstancedAnimation {
	public:
		struct TexInfo {
			Texture* texture;
			int slot;
		};
	public:
		MultiTextureModel(const char* dxd_path, const char* mt_path, const int instance_count);
		void SetMultiTexture(const char* texture_path, int tex_slot = 32);
		void Begin();
		void Set(const InstancedData& data, int animationIndex, float* time, int type);
		void End();
		void Render();
	private:
		std::unique_ptr<Texture> texture;
		std::vector<TexInfo> textures;
		D3D11_MAPPED_SUBRESOURCE resource;
		Texture* subTexture;
		int renderCount;
		UINT* buffer;
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
}
