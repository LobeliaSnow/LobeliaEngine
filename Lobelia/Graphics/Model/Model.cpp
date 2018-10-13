#include "Common/Common.hpp"
#include "Graphics/Transform/Transform.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/Mesh/Mesh.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/ShaderBank.hpp"
#include "Graphics/InputLayout/InputLayout.hpp"
#include "Graphics/Texture/Texture.hpp"
#include "Graphics/Material/Material.hpp"
#include "Graphics/Shader/Reflection/Reflection.hpp"
#include "Config/Config.hpp"
#include "Graphics/RenderState/RenderState.hpp"
#include "Graphics/RenderableObject/RenderableObject.hpp"
#include "Graphics/Model/Model.hpp"
#include "Graphics/Renderer/Renderer.hpp"

namespace Lobelia::Graphics {
#define EXCEPTION_FC(fc)		if (!fc)STRICT_THROW("�t�@�C������Ɏ��s���܂���");
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Dxd
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	DxdImporter::DxdImporter(const char* file_path) {
		std::shared_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
		//�t�@�C���J��
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		//�J����Ă��邩�ǂ����H
		if (!fc->IsOpen())STRICT_THROW("�t�@�C�����J���܂���ł���");
		//���b�V���ǂݍ���
		MeshLoad(fc);
		//�X�L���ǂݍ���
		SkinLoad(fc);
		//�I��
		fc->Close();
	}
	DxdImporter::~DxdImporter() = default;
	void DxdImporter::MeshLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//���b�V�����擾
		fc->Read(&meshCount, sizeof(int), sizeof(int), 1);
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			VertexLoad(file);
		}
	}
	void DxdImporter::VertexLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		Mesh mesh = {};
		//�C���f�b�N�X���擾
		fc->Read(&mesh.indexCount, sizeof(int), sizeof(int), 1);
		//uv���擾
		fc->Read(&mesh.uvCount, sizeof(int), sizeof(int), 1);
		mesh.vertices.resize(mesh.indexCount);
		//Vertex�擾
		fc->Read(mesh.vertices.data(), sizeof(Vertex)*mesh.indexCount, sizeof(Vertex), mesh.indexCount);
		//�}�e���A�����擾
		fc->Read(&mesh.materialNameLength, sizeof(int), sizeof(int), 1);
		//�}�e���A����
		char* temp = new char[mesh.materialNameLength];
		fc->Read(temp, sizeof(char)*mesh.materialNameLength, sizeof(char), mesh.materialNameLength);
		mesh.materialName = temp;
		delete[] temp;
		//���b�V���ǉ�
		meshes.push_back(mesh);
	}
	void DxdImporter::SkinLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		clusterCount.resize(meshCount);
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			bool isEntity = false;
			//�{�[���������Ă��邩�ۂ�
			fc->Read(&isEntity, sizeof(bool), sizeof(bool), 1);
			//�{�[�������Ă��Ȃ���ΏI��
			if (!isEntity) {
				meshsBoneInfo.push_back({});
				continue;
			}
			ClusterLoad(file, meshIndex);
		}
	}
	void DxdImporter::ClusterLoad(std::weak_ptr<Utility::FileController> file, int mesh_index) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		Bone bone = {};
		bone.isEntity = true;
		int indexCount = 0;
		//���_���擾
		fc->Read(&indexCount, sizeof(int), sizeof(int), 1);
		//���_���������o�b�t�@�m��
		bone.infos.resize(indexCount);
		for (int i = 0; i < indexCount; i++) {
			int impactSize = 0;
			//�e�����ۑ�
			fc->Read(&impactSize, sizeof(int), sizeof(int), 1);
			bone.infos[i].resize(impactSize);
			//�e���x�ۑ�
			fc->Read(bone.infos[i].data(), sizeof(Bone::Info)*impactSize, sizeof(Bone::Info), impactSize);
		}
		//�N���X�^�[���擾
		fc->Read(&clusterCount[mesh_index], sizeof(int), sizeof(int), 1);
		//�N���X�^�[���������o�b�t�@�m��
		bone.initPoseMatrices.resize(clusterCount[mesh_index]);
		std::vector<DirectX::XMFLOAT4X4> matrices(clusterCount[mesh_index]);
		//�����p���s��擾
		fc->Read(matrices.data(), sizeof(DirectX::XMFLOAT4X4)*clusterCount[mesh_index], sizeof(DirectX::XMFLOAT4X4), clusterCount[mesh_index]);
		for (int i = 0; i < clusterCount[mesh_index]; i++) {
			bone.initPoseMatrices[i] = DirectX::XMLoadFloat4x4(&matrices[i]);
		}
		//�{�[���ǉ�
		meshsBoneInfo.push_back(bone);
	}
	int DxdImporter::GetMeshCount() { return meshCount; }
	const std::vector<DxdImporter::Mesh>& DxdImporter::GetMeshes() { return meshes; }
	DxdImporter::Mesh& DxdImporter::GetMesh(int index) { return meshes[index]; }
	int DxdImporter::GetBoneCount(int mesh_index) { return clusterCount[mesh_index]; }
	const std::vector<DxdImporter::Bone>& DxdImporter::GetMeshsBoneInfos() { return meshsBoneInfo; }
	const DxdImporter::Bone& DxdImporter::GetMeshBoneInfo(int index) { return meshsBoneInfo[index]; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// material
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	MaterialImporter::MaterialImporter(const char* file_path) {
		std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		if (!fc->IsOpen())STRICT_THROW("�t�@�C�����J���܂���ł���");
		Load(fc);
		fc->Close();
	}
	MaterialImporter::~MaterialImporter() = default;
	void MaterialImporter::Load(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//�}�e���A�����擾
		fc->Read(&materialCount, sizeof(int), sizeof(int), 1);
		materials.resize(materialCount);
		auto StringLoad = [&](int* count, std::string* str) {
			fc->Read(count, sizeof(int), sizeof(int), 1);
			//�o�b�t�@�m��
			char* temp = new char[*count];
			//�}�e���A�����擾
			fc->Read(temp, sizeof(char)*(*count), sizeof(char), *count);
			*str = temp;
			//�o�b�t�@���
			delete[] temp;
		};
		for (int i = 0; i < materialCount; i++) {
			Material material = {};
			//�}�e���A�����擾
			StringLoad(&material.nameLength, &material.name);
			//�e�N�X�`�����擾
			StringLoad(&material.textureNameLength, &material.textureName);
			//�}�e���A���ǉ�
			materials[i] = material;
		}
	}
	int MaterialImporter::GetMaterialCount() { return materialCount; }
	const std::vector<MaterialImporter::Material>&  MaterialImporter::GetMaterials() { return materials; }
	const MaterialImporter::Material& MaterialImporter::GetMaterial(int index) { return materials[index]; }
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// anm
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AnimationImporter::AnimationImporter(const char* file_path) {
		std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		if (!fc->IsOpen())STRICT_THROW("�A�j���[�V�����t�@�C�����J���܂���ł���");
		//���O�擾
		LoadName(fc);
		//��{���擾
		SettingLoad(fc);
		//�L�[�t���[���擾
		KeyFramesLoad(fc);
		fc->Close();
	}
	AnimationImporter::~AnimationImporter() = default;
	void AnimationImporter::LoadName(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//�}�e���A�����擾
		fc->Read(&nameLength, sizeof(int), sizeof(int), 1);
		//�o�b�t�@�m��
		char* temp = new char[nameLength];
		//�}�e���A����
		fc->Read(temp, sizeof(char)*nameLength, sizeof(char), nameLength);
		name = temp;
		delete[] temp;
	}
	void AnimationImporter::SettingLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//1�b������̃t���[�����擾
		fc->Read(&framePerSecond, sizeof(int), sizeof(int), 1);
		//���t���[�����擾
		fc->Read(&keyFrameCount, sizeof(int), sizeof(int), 1);
		//���b�V�����擾
		fc->Read(&meshCount, sizeof(int), sizeof(int), 1);
	}
	void AnimationImporter::KeyFramesLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		infos.resize(meshCount);
		for (int i = 0; i < meshCount; i++) {
			Info info = {};
			//�N���X�^�[���擾
			fc->Read(&info.clusetCount, sizeof(int), sizeof(int), 1);
			//�o�b�t�@�m��
			info.clusterFrames.resize(info.clusetCount);
			for (int clusterIndex = 0; clusterIndex < info.clusetCount; clusterIndex++) {
				info.clusterFrames[clusterIndex].keyFrames.resize(keyFrameCount);
				//�L�[�t���[���擾
				fc->Read(info.clusterFrames[clusterIndex].keyFrames.data(), sizeof(DirectX::XMFLOAT4X4)*keyFrameCount, sizeof(DirectX::XMFLOAT4X4), keyFrameCount);
			}
			//�L�[�t���[�����擾
			infos[i] = info;
		}
	}
	const std::string& AnimationImporter::GetName() { return name; }
	int AnimationImporter::GetSampleFramePerSecond() { return framePerSecond; }
	int AnimationImporter::GetKeyFrameCount() { return keyFrameCount; }
	int AnimationImporter::GetMeshCount() { return meshCount; }
	const std::vector<AnimationImporter::Info>& AnimationImporter::GetInfos() { return infos; }
	const AnimationImporter::Info& AnimationImporter::GetInfo(int index) { return infos[index]; }
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Animation
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Animation::Animation(const char* file_path) :constantBuffer(std::make_unique<ConstantBuffer<Constant>>(3, ShaderStageList::VS)), time(0.0f) {
		std::shared_ptr<Lobelia::Graphics::AnimationImporter> importer = std::make_unique<Lobelia::Graphics::AnimationImporter>(file_path);
		//�A�j���[�V�������擾
		name = importer->GetName();
		//���b�V�����擾
		meshCount = importer->GetMeshCount();
		//1�b������̃T���v���t���[�����擾
		framePerCount = importer->GetSampleFramePerSecond();
		//�t���[�����擾
		frameCount = importer->GetKeyFrameCount();
		//�o�b�t�@�m��
		clusterCount.resize(meshCount);
		keyFrames.resize(meshCount);
		//�L�[�t���[���擾�J�n
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			//���b�V�����Ƃ̃N���X�^�[���擾
			clusterCount[meshIndex] = importer->GetInfo(meshIndex).clusetCount;
			//�o�b�t�@�m��
			keyFrames[meshIndex].resize(clusterCount[meshIndex]);
			for (int clusterIndex = 0; clusterIndex < clusterCount[meshIndex]; clusterIndex++) {
				//�o�b�t�@�m��
				keyFrames[meshIndex][clusterIndex].resize(frameCount);
				for (int frameIndex = 0; frameIndex < frameCount; frameIndex++) {
					//�e���b�V���̊e�N���X�^�[�ɂ���L�[�t���[�����擾
					keyFrames[meshIndex][clusterIndex][frameIndex] = importer->GetInfo(meshIndex).clusterFrames[clusterIndex].keyFrames[frameIndex];
				}
			}
		}
	}
	Animation::~Animation() = default;
	void Animation::AddElapsedTime(float time) {
		this->time += time;
		//�A�j���[�V�����̍ő�l���擾
		int animationMax = (frameCount - 1)*(1000 / framePerCount);
		while (this->time >= animationMax)this->time -= animationMax;
	}
	void Animation::ResetTime() {
		this->time = 0.0f;
	}
	void Animation::Update(int mesh_index) {
		//��ԓ������Ⴒ���Ⴕ�Ȃ��Ƃ����Ȃ�
		for (int i = 0; i < clusterCount[mesh_index]; i++) {
			//�{���͂����ŕ��
			DirectX::XMMATRIX renderTransform = DirectX::XMLoadFloat4x4(&keyFrames[mesh_index][i][static_cast<int>(time / (1000 / framePerCount))]);
			renderTransform = DirectX::XMMatrixTranspose(renderTransform);
			DirectX::XMStoreFloat4x4(&buffer.keyFrame[i], renderTransform);
		}
		constantBuffer->Activate(buffer);
	}
	const std::string& Animation::GetName() { return name; }
	float Animation::GetMaxTime() {
		return f_cast(frameCount - 1)*(1000 / framePerCount);
	}
	void Animation::CalcAnimationMatrix(int bone_index, DirectX::XMFLOAT4X4* anim) {
		int boneCount = 0, meshIndex = -1;
		//�ǂ̃��b�V���ɑ����Ă��邩���ׂ�
		for (int i = 0; i < meshCount; i++) {
			if (bone_index < boneCount + clusterCount[i]) {
				meshIndex = i;
				break;
			}
			boneCount += clusterCount[i];
		}
		//���̃��b�V������̃C���f�b�N�X�֕ϊ�
		bone_index -= boneCount;
		*anim = keyFrames[meshIndex][bone_index][i_cast(time / (1000 / framePerCount))];
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	ModelData
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ModelData::ModelData(const char* dxd_path, const char* mt_path) :allMeshVertexCountSum(0) {
		std::shared_ptr<DxdImporter> dxd = std::make_unique<DxdImporter>(dxd_path);
		std::shared_ptr<MaterialImporter> mt = std::make_unique<MaterialImporter>(mt_path);
		//���_���Z�o
		for (int i = 0; i < dxd->GetMeshCount(); i++) {
			//�e���b�V���̒��_���𑫂����킹��
			allMeshVertexCountSum += dxd->GetMesh(i).indexCount;
		}
		//�o�b�t�@�m��
		vertices.resize(allMeshVertexCountSum);
		//���_�\��
		ConfigureVertex(dxd);
		//�}�e���A���\��
		std::string directory = Utility::FilePathControl::GetParentDirectory(mt_path);
		if (!directory.empty())directory += "/";
		ConfigureMaterial(mt, directory);

	}
	ModelData::~ModelData() = default;
	void ModelData::ConfigureVertex(std::weak_ptr<DxdImporter> dxd) {
		std::shared_ptr<DxdImporter> importer = dxd.lock();
		if (!importer)STRICT_THROW("�C���|�[�^�[���擾�ł��܂���ł���");
		for (int meshIndex = 0, index = 0, boneIndex = 0, log = 0; meshIndex < importer->GetMeshCount(); meshIndex++) {
			auto& dxdMesh = importer->GetMesh(meshIndex);
			//���_���擾
			int vertexCount = dxdMesh.indexCount;
			//���_���(�ꕔ)�擾
			for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++, index++) {
				vertices[index].pos = dxdMesh.vertices[vertexIndex].pos;
				vertices[index].normal = dxdMesh.vertices[vertexIndex].normal;
				vertices[index].tex = dxdMesh.vertices[vertexIndex].tex;
			}
			//�{�[�����\��
			ConfigureBones(dxd, meshIndex, &boneIndex);
			//�T�u�Z�b�g�\�z
			Subset subset = { meshIndex, log, importer->GetMesh(meshIndex).indexCount ,importer->GetMesh(meshIndex).materialName };
			//���b�V���J�n�n�_�����ւ��炷
			log += importer->GetMesh(meshIndex).indexCount;
			//�T�u�Z�b�g�ǉ�
			subsets.push_back(subset);
		}
	}
	void ModelData::ConfigureBones(std::weak_ptr<DxdImporter> dxd, int mesh_index, int* vertex_index) {
		std::shared_ptr<DxdImporter> importer = dxd.lock();
		if (!importer)STRICT_THROW("�C���|�[�^�[���擾�ł��܂���ł���");
		//���b�V�����Ƃ̃{�[���Q�擾
		auto& meshBones = importer->GetMeshBoneInfo(mesh_index);
		int indexCount = importer->GetMesh(mesh_index).indexCount;
		//���̎��{�[���͐�������Ȃ��̂ŁA�������ӁB�����v��ʌ�쓮�����邩���H
		if (!meshBones.isEntity) {
			(*vertex_index) += indexCount;
			return;
		}
		//���_�\��(�c��)
		for (int i = 0; i < indexCount; i++) {
			int impactCount = i_cast(meshBones.infos[i].size());
			for (int j = 0; j < 4; j++) {
				if (j < impactCount) {
					vertices[*vertex_index].clusteIndex[j] = meshBones.infos[i][j].clusterIndex;
					vertices[*vertex_index].weights.v[j] = meshBones.infos[i][j].weight;
				}
				else {
					vertices[*vertex_index].clusteIndex[j] = 0UL;
					vertices[*vertex_index].weights.v[j] = 0.0f;
				}
			}
			(*vertex_index)++;
		}
		Bone bone = {};
		//�N���X�^�[���擾
		bone.clusterCount = importer->GetBoneCount(mesh_index);
		//�����p���s��擾
		for (int i = 0; i < bone.clusterCount; i++) {
			bone.initPoseMatrices.push_back(meshBones.initPoseMatrices[i]);
		}
		bones.push_back(bone);
	}
	void ModelData::ConfigureMaterial(std::weak_ptr<MaterialImporter> mt, const std::string& directory) {
		std::shared_ptr<MaterialImporter> importer = mt.lock();
		materialCount = importer->GetMaterialCount();
		materials.resize(materialCount);
		for (int i = 0; i < materialCount; i++) {
			auto& material = importer->GetMaterial(i);
			materials[i] = { material.name,material.textureName };
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AnimationData::AnimationData(const char* file_path) {
		std::shared_ptr<Lobelia::Graphics::AnimationImporter> importer = std::make_unique<Lobelia::Graphics::AnimationImporter>(file_path);
		//�A�j���[�V�������擾
		name = importer->GetName();
		//���b�V�����擾
		meshCount = importer->GetMeshCount();
		//1�b������̃T���v���t���[�����擾
		framePerCount = importer->GetSampleFramePerSecond();
		//�t���[�����擾
		frameCount = importer->GetKeyFrameCount();
		//�o�b�t�@�m��
		clusterCount.resize(meshCount);
		keyFrames.resize(meshCount);
		//�L�[�t���[���擾�J�n
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			//���b�V�����Ƃ̃N���X�^�[���擾
			clusterCount[meshIndex] = importer->GetInfo(meshIndex).clusetCount;
			//�o�b�t�@�m��
			keyFrames[meshIndex].resize(clusterCount[meshIndex]);
			for (int clusterIndex = 0; clusterIndex < clusterCount[meshIndex]; clusterIndex++) {
				//�o�b�t�@�m��
				keyFrames[meshIndex][clusterIndex].resize(frameCount);
				for (int frameIndex = 0; frameIndex < frameCount; frameIndex++) {
					//�e���b�V���̊e�N���X�^�[�ɂ���L�[�t���[�����擾
					keyFrames[meshIndex][clusterIndex][frameIndex] = importer->GetInfo(meshIndex).clusterFrames[clusterIndex].keyFrames[frameIndex];
				}
			}
		}
	}
	AnimationData::~AnimationData() = default;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Model
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Model::Model(const char* dxd_path, const char* mt_path) :animationCount(0), allMeshVertexCountSum(0), world(), boneCount(0) {
		if (!blend)blend = std::make_shared<BlendState>(Graphics::BLEND_PRESET::COPY, true, true);
		if (!sampler)sampler = std::make_shared<SamplerState>(Graphics::SAMPLER_PRESET::POINT, 16);
		if (!rasterizer) rasterizer = std::make_shared<RasterizerState>(Graphics::RASTERIZER_PRESET::FRONT);
		if (!depthStencil)depthStencil = std::make_shared<DepthStencilState>(Graphics::DEPTH_PRESET::ALWAYS, true, Graphics::StencilDesc(), false);
		if (!vs)vs = std::make_shared<VertexShader>("Data/Shaderfile/3D/VS.hlsl", "Main3DNoSkin", Graphics::VertexShader::Model::VS_4_0);
		vsAnim = std::make_shared<VertexShader>("Data/Shaderfile/3D/VS.hlsl", "Main3D", Graphics::VertexShader::Model::VS_4_0);
		if (!ps) {
			ps = std::make_shared<PixelShader>("Data/Shaderfile/3D/PS.hlsl", "Main3D", Graphics::PixelShader::Model::PS_5_0, true);
			ps->GetLinkage()->CreateInstance("Lambert");
			ps->GetLinkage()->CreateInstance("Diffuse");
			ps->GetLinkage()->CreateInstance("Fog");
			ps->GetLinkage()->CreateInstance("Phong");
			ps->SetLinkage(0);
		}
		StateInitialize();
		std::shared_ptr<DxdImporter> dxd = std::make_unique<DxdImporter>(dxd_path);
		std::shared_ptr<MaterialImporter> mt = std::make_unique<MaterialImporter>(mt_path);
		//���_���Z�o
		for (int i = 0; i < dxd->GetMeshCount(); i++) {
			//�e���b�V���̒��_���𑫂����킹��
			allMeshVertexCountSum += dxd->GetMesh(i).indexCount;
		}
		//���b�V���o�b�t�@�m��
		mesh = std::make_unique<Mesh<Vertex>>(allMeshVertexCountSum);
		//���_�\��
		ConfigureVertex(dxd);
		//�}�e���A���\��
		std::string directory = Utility::FilePathControl::GetParentDirectory(mt_path);
		if (!directory.empty())directory += "/";
		ConfigureMaterial(mt, directory);
		//�o�b�t�@�m��
		renderIndexMaterial.resize(dxd->GetMeshCount());
		//�`�揇�Ƀ}�e���A���ւ̃|�C���^����
		for (int i = 0; i < dxd->GetMeshCount(); i++) {
			renderIndexMaterial[i] = materials[dxd->GetMesh(i).materialName].get();
		}
		SetTransformAndCalcMatrix(transform);
		CalcWorldMatrix();
		int boneSize = bones.size();
		for (int i = 0; i < boneSize; i++) {
			boneCount += bones[i].clusterCount;
		}
	}
	Model::~Model() = default;
	void Model::StateInitialize() {
		//�A�N�e�B�u�ȃA�j���[�V�����͖���(-1)
		activeAnimation = -1;
		//���t���N�V�����J�n
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs.get());
		//���̓��C�A�E�g�쐬
		inputLayout = std::make_unique<InputLayout>(vs.get(), reflector.get());
		//�R���X�^���g�o�b�t�@�쐬
		constantBuffer = std::make_unique<ConstantBuffer<DirectX::XMMATRIX>>(1, Config::GetRefPreference().systemCBActiveStage);
		//�e�͐ݒ肳��Ă��Ȃ�
		parent = nullptr;
		transform = {};
		transform.scale = Math::Vector3(1.0f, 1.0f, 1.0f);
		animationCount = 0;
	}
	void Model::ConfigureVertex(std::weak_ptr<DxdImporter> dxd) {
		std::shared_ptr<DxdImporter> importer = dxd.lock();
		if (!importer)STRICT_THROW("�C���|�[�^�[���擾�ł��܂���ł���");
		for (int meshIndex = 0, index = 0, boneIndex = 0, log = 0; meshIndex < importer->GetMeshCount(); meshIndex++) {
			auto& dxdMesh = importer->GetMesh(meshIndex);
			//���_���擾
			int vertexCount = dxdMesh.indexCount;
			//���_���(�ꕔ)�擾
			for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++, index++) {
				mesh->GetBuffer()[index].pos = dxdMesh.vertices[vertexIndex].pos;
				mesh->GetBuffer()[index].normal = dxdMesh.vertices[vertexIndex].normal;
				mesh->GetBuffer()[index].tex = dxdMesh.vertices[vertexIndex].tex;
			}
			//�{�[�����\��
			ConfigureBones(dxd, meshIndex, &boneIndex);
			//�T�u�Z�b�g�\�z
			Subset subset = { meshIndex, log, importer->GetMesh(meshIndex).indexCount };
			//���b�V���J�n�n�_�����ւ��炷
			log += importer->GetMesh(meshIndex).indexCount;
			//�T�u�Z�b�g�ǉ�
			subsets.push_back(subset);
		}
	}
	void Model::ConfigureBones(std::weak_ptr<DxdImporter> dxd, int mesh_index, int* vertex_index) {
		std::shared_ptr<DxdImporter> importer = dxd.lock();
		if (!importer)STRICT_THROW("�C���|�[�^�[���擾�ł��܂���ł���");
		//���b�V�����Ƃ̃{�[���Q�擾
		auto& meshBones = importer->GetMeshBoneInfo(mesh_index);
		int indexCount = importer->GetMesh(mesh_index).indexCount;
		//���̎��{�[���͐�������Ȃ��̂ŁA�������ӁB�����v��ʌ�쓮�����邩���H
		if (!meshBones.isEntity) {
			(*vertex_index) += indexCount;
			return;
		}
		//���_�\��(�c��)
		for (int i = 0; i < indexCount; i++) {
			int impactCount = i_cast(meshBones.infos[i].size());
			for (int j = 0; j < 4; j++) {
				if (j < impactCount) {
					mesh->GetBuffer()[*vertex_index].clusteIndex[j] = meshBones.infos[i][j].clusterIndex;
					mesh->GetBuffer()[*vertex_index].weights.v[j] = meshBones.infos[i][j].weight;
				}
				else {
					mesh->GetBuffer()[*vertex_index].clusteIndex[j] = 0UL;
					mesh->GetBuffer()[*vertex_index].weights.v[j] = 0.0f;
				}
			}
			(*vertex_index)++;
		}
		Bone bone = {};
		//�N���X�^�[���擾
		bone.clusterCount = importer->GetBoneCount(mesh_index);
		//�����p���s��擾
		for (int i = 0; i < bone.clusterCount; i++) {
			bone.initPoseMatrices.push_back(meshBones.initPoseMatrices[i]);
		}
		bones.push_back(bone);
	}
	void Model::ConfigureMaterial(std::weak_ptr<MaterialImporter> mt, const std::string& directory) {
		std::shared_ptr<MaterialImporter> importer = mt.lock();
		int materialCount = importer->GetMaterialCount();
		for (int i = 0; i < materialCount; i++) {
			auto& material = importer->GetMaterial(i);
			materials[material.name] = std::make_shared<Material>(material.name.c_str(), (directory + material.textureName).c_str());
		}
	}
	void Model::CalcTranslateMatrix() { translate = DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z); }
	void Model::CalcScallingMatrix() {
		scalling = DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z);
		scalling.m[0][0] *= -1;
	}
	void Model::SetTransformAndCalcMatrix(const Transform3D& transform) {
		this->transform = transform;
		Translation(transform.position);
		RotationRollPitchYow(transform.rotation);
		Scalling(transform.scale);
		CalcWorldMatrix();
	}
	const Transform3D& Model::GetTransform() { return transform; }
	void Model::LinkParent(Model* model) { parent = model; }
	void Model::UnLinkParent() { parent = nullptr; }
	//�ړ�
	void Model::Translation(const Math::Vector3& pos) {
		transform.position = pos;
		CalcTranslateMatrix();
	}
	void Model::Translation(float x, float y, float z) {
		transform.position.x = x; transform.position.y = y; transform.position.z = z;
		CalcTranslateMatrix();
	}
	void Model::TranslationMove(const Math::Vector3& move) {
		transform.position += move;
		CalcTranslateMatrix();
	}
	void Model::TranslationMove(float x, float y, float z) {
		transform.position.x += x; transform.position.y += y; transform.position.z += z;
		CalcTranslateMatrix();
	}
	//��]
	void Model::RotationQuaternion(const DirectX::XMVECTOR& quaternion) {
		rotation = DirectX::XMMatrixRotationQuaternion(quaternion);
		//�����Ńg�����X�t�H�[���̉�]��RPY�̉�]�ʎZ�o
	}
	void Model::RotationAxis(const Math::Vector3& axis, float rad) {
		rotation = DirectX::XMMatrixRotationAxis(DirectX::XMVECTOR{ axis.x,axis.y,axis.z,1.0f }, rad);
		//�����Ńg�����X�t�H�[���̉�]��RPY�̉�]�ʎZ�o
	}
	void Model::RotationRollPitchYow(const Math::Vector3& rpy) {
		transform.rotation = rpy;
		rotation = DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z);
	}
	void Model::RotationYAxis(float rad) {
		transform.rotation.x = 0.0f;	transform.rotation.y = rad; transform.rotation.z = 0.0f;
		rotation = DirectX::XMMatrixRotationY(transform.rotation.y);
	}
	//�g�k
	void Model::Scalling(const Math::Vector3& scale) {
		transform.scale = scale;
		CalcScallingMatrix();
	}
	void Model::Scalling(float x, float y, float z) {
		transform.scale.x = x; transform.scale.y = y; transform.scale.z = z;
		CalcScallingMatrix();
	}
	void Model::Scalling(float scale) {
		transform.scale.x = scale; transform.scale.y = scale; transform.scale.z = scale;
		CalcScallingMatrix();
	}
	//�X�V����
	void Model::CalcWorldMatrix() {
		//�e�q�֌W���邳���͎�����transform�͐e���猩�����̂ɂȂ邪�A���[���h�̏�Ԃł��~�������ȁH
		world = scalling;
		world *= rotation;
		//�����͏����R�c���K�v
		world *= translate;
		//�e�q�֌W����
		if (parent)world *= parent->world;
		//���ʒux�𔽓]������(FBX������)
		//�e������Ƃ��̍s��ł��ł�-1�|������Ă���̂ł����ł͕K�v���Ȃ�(?)
	}
	void Model::GetTranslateMatrix(DirectX::XMMATRIX* translate) {
		if (!translate)STRICT_THROW("translate��nullptr�ł�");
		*translate = this->translate;
	}
	void Model::CalcInverseTranslateMatrix(DirectX::XMMATRIX* inv_translate) {
		if (!inv_translate)STRICT_THROW("inv_world��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_translate = DirectX::XMMatrixInverse(&arg, translate);
	}
	void Model::GetScallingMatrix(DirectX::XMMATRIX* scalling) {
		if (!scalling)STRICT_THROW("scalling��nullptr�ł�");
		*scalling = this->scalling;
	}
	void Model::CalcInverseScallingMatrix(DirectX::XMMATRIX* inv_scalling) {
		if (!inv_scalling)STRICT_THROW("inv_world��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_scalling = DirectX::XMMatrixInverse(&arg, scalling);

	}
	void Model::GetRotationMatrix(DirectX::XMMATRIX* rotation) {
		if (!rotation)STRICT_THROW("rotation��nullptr�ł�");
		*rotation = this->rotation;
	}
	void Model::CalcInverseRotationMatrix(DirectX::XMMATRIX* inv_rotation) {
		if (!inv_rotation)STRICT_THROW("inv_rotation��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_rotation = DirectX::XMMatrixInverse(&arg, rotation);
	}
	void Model::GetWorldMatrix(DirectX::XMMATRIX* world) {
		if (!world)STRICT_THROW("world��nullptr�ł�");
		*world = this->world;
	}
	void Model::CalcInverseWorldMatrix(DirectX::XMMATRIX* inv_world) {
		if (!inv_world)STRICT_THROW("inv_world��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_world = DirectX::XMMatrixInverse(&arg, world);
	}
	int Model::GetBoneCount() { return boneCount; }
	const DirectX::XMMATRIX& Model::GetBone(int index) {
		if (index >= boneCount)STRICT_THROW("�C���f�b�N�X���{�[���̐��𒴂��Ă��܂�");
		int counter = 0;
		int boneSize = bones.size();
		for (int i = 0; i < boneSize; i++) {
			int nowLocation = counter;
			if (index >= counter && index < (counter += bones[i].clusterCount)) {
				int accessIndex = index - nowLocation;
				return bones[i].initPoseMatrices[accessIndex];
			}
		}
	}
	AnimationNo Model::AnimationLoad(const char* file_path) {
		animations.push_back(std::make_unique<Animation>(file_path));
		return animationCount++;
	}
	void Model::AnimationActivate(AnimationNo index) {
		if (index >= animationCount)STRICT_THROW("���݂��Ȃ��A�j���[�V�����ł�");
		if (activeAnimation != -1 && activeAnimation != index)animations[activeAnimation]->ResetTime();
		activeAnimation = index;
	}
	void Model::AnimationUnActive() {
		if (activeAnimation == -1)return;
		animations[activeAnimation]->ResetTime();
		activeAnimation = -1;
	}
	const std::string& Model::GetAnimationName(AnimationNo index) { return animations[index]->GetName(); }
	std::string Model::GetMaterialName(int poly_index) {
		for each(auto&& subset in subsets) {
			if (subset.ThisIsMyVertex(poly_index)) return renderIndexMaterial[subset.index]->GetName();
		}
		return "none material";
	}
	float Model::GetAnimationTime(AnimationNo index) { return animations[index]->GetMaxTime(); }
	void Model::CalcAnimationMatrix(AnimationNo index, int bone_index, DirectX::XMFLOAT4X4* anim) { return animations[index]->CalcAnimationMatrix(bone_index, anim); }
	void Model::AnimationUpdate(float elapsed_time) { if (activeAnimation != -1)animations[activeAnimation]->AddElapsedTime(elapsed_time); }
	Material* Model::GetMaterial(const char* mt_name) { return materials[mt_name].get(); }
	void Model::Render(D3D_PRIMITIVE_TOPOLOGY topology, bool no_set) {
		blend->Set(true);
		sampler->Set(true);
		rasterizer->Set(true);
		depthStencil->Set(true);
		ps->Set();
		mesh->Set(); inputLayout->Set(); constantBuffer->Activate(DirectX::XMMatrixTranspose(world));
		Device::GetContext()->IASetPrimitiveTopology(topology);
		if (!no_set) {
			//�X�L�j���O���邩�ۂ�
			if (activeAnimation > -1) vsAnim->Set();
			else vs->Set();
		}
		int meshIndex = 0;
		for each(auto subset in subsets) {
			if (activeAnimation > -1)animations[activeAnimation]->Update(meshIndex);
			subset.Render(this);
			meshIndex++;
		}
	}
	//���̂������f���f�[�^�Ƃ̋������l����
	int Model::RayPickWorld(Math::Vector3* out_pos, Math::Vector3* out_normal, const Math::Vector3& ray_pos, const Math::Vector3& ray_vec, float dist) {
#ifdef _DEBUG
		Math::Vector3 debugDeirection = ray_vec; debugDeirection.Normalize();
		Graphics::DebugRenderer::GetInstance()->SetLine(ray_pos, ray_pos + debugDeirection * dist, 0xFFFFFFFF);
#endif
		DirectX::XMMATRIX inverseWorld = {};
		//���[���h�ϊ��s��̋t�s��擾
		CalcInverseWorldMatrix(&inverseWorld);
		//�J�����ʒu
		DirectX::XMVECTOR eye = DirectX::XMVectorSet(ray_pos.x, ray_pos.y, ray_pos.z, 1.0f);
		//���C�̐i�s�x�N�g��
		DirectX::XMVECTOR ray = DirectX::XMVectorSet(ray_vec.x, ray_vec.y, ray_vec.z, 0.0f);
		//���f���̃��[�J����ԂɘA��Ă���
		eye = DirectX::XMVector4Transform(eye, inverseWorld);
		ray = DirectX::XMVector4Transform(ray, inverseWorld);
		//Math::Vector3�֕ϊ�
		Math::Vector3 rayPos(DirectX::XMVectorGetX(eye), DirectX::XMVectorGetY(eye), DirectX::XMVectorGetZ(eye));
		Math::Vector3 rayVec(DirectX::XMVectorGetX(ray), DirectX::XMVectorGetY(ray), DirectX::XMVectorGetZ(ray));
		//���C�s�b�N
		int ret = RayPickLocal(out_pos, out_normal, rayPos, rayVec, dist);
		//�������Ă��Ȃ��Ȃ炱�̐�̌v�Z�͕K�v�Ȃ��̂ŕԂ�
		if (ret < 0)return ret;
		//DirectX::XMVECTOR�֕ϊ�
		DirectX::XMVECTOR outPos = DirectX::XMVectorSet(out_pos->x, out_pos->y, out_pos->z, 1.0f);
		DirectX::XMVECTOR outNormal = DirectX::XMVectorSet(out_normal->x, out_normal->y, out_normal->z, 0.0f);
		//���[���h���W�֖߂�
		outPos = DirectX::XMVector3Transform(outPos, world);
		outNormal = DirectX::XMVector3Transform(outNormal, world);
		//Math::Vector3�֕ϊ�
		out_pos->x = DirectX::XMVectorGetX(outPos); out_pos->y = DirectX::XMVectorGetY(outPos); out_pos->z = DirectX::XMVectorGetZ(outPos);
		out_normal->x = DirectX::XMVectorGetX(outNormal); out_normal->y = DirectX::XMVectorGetY(outNormal); out_normal->z = DirectX::XMVectorGetZ(outNormal);
#ifdef _DEBUG
		Graphics::DebugRenderer::GetInstance()->SetLine(*out_pos, *out_pos + *out_normal, 0xFF00FFFF);
#endif
		return ret;
	}
	int Model::RayPickLocal(Math::Vector3* out_pos, Math::Vector3* out_normal, const Math::Vector3& ray_pos, const Math::Vector3& ray_vec, float dist) {
		//�O�p�`�̐��Z�o
		int faceSum = allMeshVertexCountSum / 3;
		for (int face = 0; face < faceSum; face++) {
			//�O�p�`�̒��_�擾
			Math::Vector3 pos[3] = {};
			for (int tri = 0; tri < 3; tri++) {
				pos[tri] = mesh->GetBuffer()[face * 3 + tri].pos.xyz;
			}
			//�ӎZ�o
			Math::Vector3 edge[3] = {};
			for (int tri = 0; tri < 3; tri++) {
				int temp = tri + 1 > 2 ? 0 : tri + 1;
				edge[tri] = pos[temp] - pos[tri];
			}
			//�@���Z�o
			Math::Vector3 normal = {};
			normal = Math::Vector3::Cross(edge[0], edge[1]);
			//�x�N�g���̌����ɂ��\������
			float dot = Math::Vector3::Dot(ray_vec, normal);
			if (dot >= 0.0f)continue;
			//�|���S���܂ł̍ŒZ�����Z�o
			Math::Vector3 posFromRay = pos[0] - ray_pos;
			float nearLength = Math::Vector3::Dot(normal, posFromRay) / dot;
			//�������}�C�i�X�ł���Δ��΂̌����ɂ��邽�ߒe���A�w�肵����������������Γ�����Ȃ��̂Œe��
			if (nearLength < 0.0f || nearLength > dist)continue;
			//���C�̌����Ƀ|���S���܂ł̍ŒZ�ňړ������_���Z�o(��_)
			Math::Vector3 rayPoint = ray_vec * nearLength + ray_pos;
			//���_����J�n
			bool isInside = true;
			for (int i = 0; i < 3; i++) {
				Math::Vector3 pointVec = pos[i] - rayPoint;
				//���̃x�N�g���̌������d�v
				Math::Vector3 testVec = Math::Vector3::Cross(pointVec, edge[i]);
				//�x�N�g���̌������@�������Ɣ��΂������ꍇ�A�O��
				if (Math::Vector3::Dot(testVec, normal) < 0.0f) {
					isInside = false;
					break;
				}
			}
			//�|���S���̓����ɓ_���Ȃ������ꍇ����
			if (!isInside)continue;
			//���������I
			*out_pos = rayPoint;
			*out_normal = normal;
			return face;
		}
		return -1;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Transformer::CalcTranslateMatrix() { translate = DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z); }
	void Transformer::CalcScallingMatrix() {
		scalling = DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z);
		scalling.m[0][0] *= -1;
	}
	Transformer::Transformer() {
		world = scalling = rotation = translate = DirectX::XMMatrixIdentity();
	}
	//�ړ�
	void Transformer::Translation(const Math::Vector3& pos) {
		transform.position = pos;
		CalcTranslateMatrix();
	}
	void Transformer::Translation(float x, float y, float z) {
		transform.position.x = x; transform.position.y = y; transform.position.z = z;
		CalcTranslateMatrix();
	}
	void Transformer::TranslationMove(const Math::Vector3& move) {
		transform.position += move;
		CalcTranslateMatrix();
	}
	void Transformer::TranslationMove(float x, float y, float z) {
		transform.position.x += x; transform.position.y += y; transform.position.z += z;
		CalcTranslateMatrix();
	}
	//��]
	void Transformer::RotationQuaternion(const DirectX::XMVECTOR& quaternion) {
		rotation = DirectX::XMMatrixRotationQuaternion(quaternion);
		//�����Ńg�����X�t�H�[���̉�]��RPY�̉�]�ʎZ�o
	}
	void Transformer::RotationAxis(const Math::Vector3& axis, float rad) {
		rotation = DirectX::XMMatrixRotationAxis(DirectX::XMVECTOR{ axis.x,axis.y,axis.z,1.0f }, rad);
		//�����Ńg�����X�t�H�[���̉�]��RPY�̉�]�ʎZ�o
	}
	void Transformer::RotationRollPitchYow(const Math::Vector3& rpy) {
		transform.rotation = rpy;
		rotation = DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z);
	}
	void Transformer::RotationYAxis(float rad) {
		transform.rotation.x = 0.0f;	transform.rotation.y = rad; transform.rotation.z = 0.0f;
		rotation = DirectX::XMMatrixRotationY(transform.rotation.y);
	}
	//�g�k
	void Transformer::Scalling(const Math::Vector3& scale) {
		transform.scale = scale;
		CalcScallingMatrix();
	}
	void Transformer::Scalling(float x, float y, float z) {
		transform.scale.x = x; transform.scale.y = y; transform.scale.z = z;
		CalcScallingMatrix();
	}
	void Transformer::Scalling(float scale) {
		transform.scale.x = scale; transform.scale.y = scale; transform.scale.z = scale;
		CalcScallingMatrix();
	}
	//�X�V����
	void Transformer::CalcWorldMatrix() {
		//�e�q�֌W���邳���͎�����transform�͐e���猩�����̂ɂȂ邪�A���[���h�̏�Ԃł��~�������ȁH
		world = scalling;
		world *= rotation;
		//�����͏����R�c���K�v
		world *= translate;
		//���ʒux�𔽓]������(FBX������)
	}
	void Transformer::GetTranslateMatrix(DirectX::XMMATRIX* translate) {
		if (!translate)STRICT_THROW("translate��nullptr�ł�");
		*translate = this->translate;
	}
	void Transformer::CalcInverseTranslateMatrix(DirectX::XMMATRIX* inv_translate) {
		if (!inv_translate)STRICT_THROW("inv_world��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_translate = DirectX::XMMatrixInverse(&arg, translate);
	}
	void Transformer::GetScallingMatrix(DirectX::XMMATRIX* scalling) {
		if (!scalling)STRICT_THROW("scalling��nullptr�ł�");
		*scalling = this->scalling;
	}
	void Transformer::CalcInverseScallingMatrix(DirectX::XMMATRIX* inv_scalling) {
		if (!inv_scalling)STRICT_THROW("inv_world��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_scalling = DirectX::XMMatrixInverse(&arg, scalling);

	}
	void Transformer::GetRotationMatrix(DirectX::XMMATRIX* rotation) {
		if (!rotation)STRICT_THROW("rotation��nullptr�ł�");
		*rotation = this->rotation;
	}
	void Transformer::CalcInverseRotationMatrix(DirectX::XMMATRIX* inv_rotation) {
		if (!inv_rotation)STRICT_THROW("inv_rotation��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_rotation = DirectX::XMMatrixInverse(&arg, rotation);
	}
	void Transformer::GetWorldMatrixTranspose(DirectX::XMMATRIX* t_world) {
		if (!t_world)STRICT_THROW("t_world��nullptr�ł�");
		*t_world = DirectX::XMMatrixTranspose(world);
	}
	void Transformer::GetWorldMatrix(DirectX::XMMATRIX* world) {
		if (!world)STRICT_THROW("world��nullptr�ł�");
		*world = this->world;
	}
	void Transformer::CalcInverseWorldMatrix(DirectX::XMMATRIX* inv_world) {
		if (!inv_world)STRICT_THROW("inv_world��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_world = DirectX::XMMatrixInverse(&arg, world);
	}
	const Transform3D& Transformer::GetTransform() { return transform; }
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	InstancingEngine
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	InstancingEngine::InstancingEngine(const int instance_count) :INSTANCE_MAX(instance_count) {
		BufferCreator::Create(buffer.GetAddressOf(), nullptr, sizeof(InstancedData)*INSTANCE_MAX, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE, D3D11_CPU_ACCESS_WRITE, sizeof(float), 0);
	};
	InstancingEngine::~InstancingEngine() = default;
	InstancedData* InstancingEngine::Map() {
		D3D11_MAPPED_SUBRESOURCE resource;
		HRESULT hr = Graphics::Device::GetContext()->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		if (FAILED(hr))STRICT_THROW("Map�Ɏ��s���܂���");
		return static_cast<InstancedData*>(resource.pData);
	}
	void InstancingEngine::Unmap() {
		Graphics::Device::GetContext()->Unmap(buffer.Get(), 0);
	}
	ComPtr<ID3D11Buffer>& InstancingEngine::GetBuffer() { return buffer; };
	const int InstancingEngine::GetInstanceMax() { return INSTANCE_MAX; }
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	ModelInstanced
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ModelInstanced::ModelInstanced(const char* dxd_path, const char* mt_path, const int instance_count) :mapBuffer(nullptr), renderCount(0) {
		if (!blend)blend = std::make_shared<BlendState>(Graphics::BLEND_PRESET::COPY, true, true);
		if (!sampler)sampler = std::make_shared<SamplerState>(Graphics::SAMPLER_PRESET::POINT, 16);
		if (!rasterizer) rasterizer = std::make_shared<RasterizerState>(Graphics::RASTERIZER_PRESET::FRONT);
		if (!depthStencil)depthStencil = std::make_shared<DepthStencilState>(Graphics::DEPTH_PRESET::ALWAYS, true, Graphics::StencilDesc(), false);
		if (!vs)vs = std::make_shared<VertexShader>("Data/Shaderfile/3D/VS.hlsl", "Main3DInstancingNoSkin", Graphics::VertexShader::Model::VS_4_0);
		if (!ps) {
			ps = std::make_shared<PixelShader>("Data/Shaderfile/3D/PS.hlsl", "Main3D", Graphics::PixelShader::Model::PS_5_0, true);
			ps->GetLinkage()->CreateInstance("Lambert");
			ps->GetLinkage()->CreateInstance("Fog");
			ps->GetLinkage()->CreateInstance("Phong");
			ps->SetLinkage(0);
		}
		instancingEngine = std::make_unique<InstancingEngine>(instance_count);
		modelData = ResourceBank<ModelData>::Factory(std::string(dxd_path) + mt_path, dxd_path, mt_path);
		mesh = std::make_unique<Mesh<Vertex>>(modelData->allMeshVertexCountSum);
		for (int i = 0; i < modelData->allMeshVertexCountSum; i++) {
			mesh->GetBuffer()[i].pos = modelData->vertices[i].pos;
			mesh->GetBuffer()[i].normal = modelData->vertices[i].normal;
			mesh->GetBuffer()[i].tex = modelData->vertices[i].tex;
		}
		mesh->Update();
		int meshCount = i_cast(modelData->subsets.size());
		std::string directory = Utility::FilePathControl::GetParentDirectory(dxd_path);
		if (!directory.empty()) directory += "/";
		for (int i = 0; i < modelData->materialCount; i++) {
			materials[modelData->materials[i].name] = std::make_shared<Material>(modelData->materials[i].name.c_str(), (directory + modelData->materials[i].texturePath).c_str());
		}
		//�`�揇�Ƀ}�e���A���ւ̃|�C���^����
		renderIndexMaterial.resize(meshCount);
		subsets.resize(meshCount);
		//renderIndexMaterial[i] = materials[dxd->GetMesh(i).materialName].get();
		for (int i = 0; i < meshCount; i++) {
			renderIndexMaterial[i] = materials[modelData->subsets[i].materialName].get();
			subsets[i] = { modelData->subsets[i].index,modelData->subsets[i].start,modelData->subsets[i].sum };
		}
		//���t���N�V�����J�n
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs.get());
		//���̓��C�A�E�g�쐬
		inputLayout = std::make_unique<InputLayout>(vs.get(), reflector.get());
	}
	ModelInstanced::~ModelInstanced() = default;
	void ModelInstanced::Begin() {
		mapBuffer = instancingEngine->Map();
		renderCount = 0;
	}
	void ModelInstanced::Set(const InstancedData& data) {
		if (!mapBuffer)STRICT_THROW("Buffer��nullptr�ł� Map���s���Ă�������");
		if (renderCount >= instancingEngine->GetInstanceMax())STRICT_THROW("�o�b�t�@�̃T�C�Y���I�[�o�[���܂���");
		mapBuffer[renderCount++] = data;
	}
	void ModelInstanced::End() {
		mapBuffer = nullptr;
		instancingEngine->Unmap();
	}
	void ModelInstanced::Render() {
		if (mapBuffer)STRICT_THROW("Unmap���s���Ă�������");
		inputLayout->Set();
		Device::GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		blend->Set(true);
		sampler->Set(true);
		rasterizer->Set(true);
		depthStencil->Set(true);
		inputLayout->Set();
		vs->Set();
		ps->Set();
		ID3D11Buffer* buffers[2] = { mesh->GetVertexBuffer().Get() , instancingEngine->GetBuffer().Get() };
		UINT stride[2] = { sizeof(Vertex), sizeof(InstancedData) };
		UINT offset[2] = { 0, 0 };
		Device::GetContext()->IASetVertexBuffers(0, 2, buffers, stride, offset);

		for each(auto subset in subsets) {
			subset.Render(this);
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	InstancingAnimationEngine
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	InstancingAnimationEngine::InstancingAnimationEngine(const int instance_max) :INSTANCE_MAX(instance_max), times(instance_max), animationIndex(instance_max), animationCount(0), clusterMax(0) {

	}
	InstancingAnimationEngine::~InstancingAnimationEngine() = default;
	AnimationNo InstancingAnimationEngine::LoadAnimation(const char* anm_path) {
		animationData.push_back(ResourceBank<AnimationData>::Factory(anm_path, anm_path));
		for (int i = 0; i < animationData[animationCount]->meshCount; i++) {
			if (clusterMax < animationData[animationCount]->clusterCount[i])clusterMax = animationData[animationCount]->clusterCount[i];
		}
		return animationCount++;
	}
	float InstancingAnimationEngine::AnimationTimeMax(AnimationNo animation_index) { return f_cast(animationData[animationCount]->frameCount - 1)*(1000 / animationData[animationCount]->framePerCount); }
	void InstancingAnimationEngine::CreateTexture() {
		//�S�A�j���[�V�����^�C�~���O�ł��N���X�^�[������؂�悤��
		//�S���f��������؂�悤��
		Math::Vector2 size(f_cast(clusterMax * 4), f_cast(INSTANCE_MAX));
		DXGI_SAMPLE_DESC desc = { 1,0 };
		animationFrames = std::make_unique<Texture>(size, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_SHADER_RESOURCE, desc, Texture::ACCESS_FLAG::DYNAMIC, Texture::CPU_ACCESS_FLAG::WRITE);
	}
#include <omp.h>
	void InstancingAnimationEngine::SetData(int instance_index, int animation_index, float* times) {
		this->times[instance_index] = times;
		animationIndex[instance_index] = animation_index;
		AnimationData* data = animationData[animation_index];
		//�A�j���[�V�����̍ő�l���擾
		int animationMax = (data->frameCount - 1)*(1000 / data->framePerCount);
		while (*this->times[instance_index] >= animationMax) *this->times[instance_index] -= animationMax;
	}
	void InstancingAnimationEngine::TextureWriteFrames(int mesh_index, int render_count) {
		D3D11_MAPPED_SUBRESOURCE resource;
		Device::GetContext()->Map(animationFrames->Get().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		Math::Vector4* writeTex = static_cast<Math::Vector4*>(resource.pData);
		//int x = animationFrames->GetSize().x;
		int x = resource.RowPitch / 16;
		//openmp�̃I�v�V�����K�{
#pragma omp parallel for
		for (int instanceIndex = 0; instanceIndex < render_count; instanceIndex++) {
			AnimationData* data = animationData[animationIndex[instanceIndex]];
			auto* frame = data->keyFrames[mesh_index].data();
			float time = *times[instanceIndex];
			int count = data->framePerCount;
#pragma omp parallel for
			for (int i = 0; i < data->clusterCount[mesh_index]; i++) {
				Math::Vector4* p = writeTex + x * instanceIndex + i * 4;
				DirectX::XMMATRIX pose = DirectX::XMLoadFloat4x4(&frame[i][static_cast<int>(time / (1000 / count))]);
				pose = DirectX::XMMatrixTranspose(pose);
				//�A�j���[�V�������e�N�X�`���ɏ�������
				memcpy_s(p, sizeof(DirectX::XMMATRIX), &pose, sizeof(DirectX::XMMATRIX));
			}
		}
		Device::GetContext()->Unmap(animationFrames->Get().Get(), 0);
	}
	void InstancingAnimationEngine::CalcAnimationMatrix(AnimationNo animation_index, float time, int bone_index, DirectX::XMFLOAT4X4* anim) {
		int boneCount = 0, meshIndex = -1;
		//�ǂ̃��b�V���ɑ����Ă��邩���ׂ�
		for (int i = 0; i < animationData[animation_index]->meshCount; i++) {
			if (bone_index < boneCount + animationData[animation_index]->clusterCount[i]) {
				meshIndex = i;
				break;
			}
			boneCount += animationData[animation_index]->clusterCount[i];
		}
		//���̃��b�V������̃C���f�b�N�X�֕ϊ�
		bone_index -= boneCount;
		*anim = animationData[animation_index]->keyFrames[meshIndex][bone_index][i_cast(time / (1000 / animationData[animation_index]->framePerCount))];
	}
	void InstancingAnimationEngine::Activate() {
		animationFrames->Set(3, ShaderStageList::VS);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// ModelInstancedAnimation
	///////////////////////////////////////////////////////////////////////////////////////////////////
	ModelInstancedAnimation::ModelInstancedAnimation(const char* dxd_path, const char* mt_path, const int instance_count) :mapBuffer(nullptr),renderCount(0) {
		if (!blend)blend = std::make_shared<BlendState>(Graphics::BLEND_PRESET::COPY, true, true);
		if (!sampler)sampler = std::make_shared<SamplerState>(Graphics::SAMPLER_PRESET::POINT, 16);
		if (!rasterizer) rasterizer = std::make_shared<RasterizerState>(Graphics::RASTERIZER_PRESET::FRONT);
		if (!depthStencil)depthStencil = std::make_shared<DepthStencilState>(Graphics::DEPTH_PRESET::ALWAYS, true, Graphics::StencilDesc(), false);
		if (!vs)vs = std::make_shared<VertexShader>("Data/Shaderfile/3D/VS.hlsl", "Main3DInstancing", Graphics::VertexShader::Model::VS_4_0);
		if (!ps) {
			ps = std::make_shared<PixelShader>("Data/Shaderfile/3D/PS.hlsl", "Main3D", Graphics::PixelShader::Model::PS_5_0, true);
			ps->GetLinkage()->CreateInstance("Lambert");
			ps->GetLinkage()->CreateInstance("Fog");
			ps->GetLinkage()->CreateInstance("Phong");
			ps->SetLinkage(0);
		}
		//��������������̕��@�Ńt���b�V���o����悤��
		ModelData* modelData = ResourceBank<ModelData>::Factory(std::string(dxd_path) + mt_path, dxd_path, mt_path);
		instancingEngine = std::make_unique<InstancingEngine>(instance_count);
		animationEngine = std::make_unique<InstancingAnimationEngine>(instance_count);
		mesh = std::make_unique<Mesh<Vertex>>(modelData->allMeshVertexCountSum);
		for (int i = 0; i < modelData->allMeshVertexCountSum; i++) {
			mesh->GetBuffer()[i].pos = modelData->vertices[i].pos;
			mesh->GetBuffer()[i].normal = modelData->vertices[i].normal;
			mesh->GetBuffer()[i].tex = modelData->vertices[i].tex;
			for (int j = 0; j < 4; j++) {
				mesh->GetBuffer()[i].weights[j] = modelData->vertices[i].weights.v[j];
				mesh->GetBuffer()[i].clusteIndex[j] = modelData->vertices[i].clusteIndex[j];
			}
		}
		mesh->Update();
		int meshCount = i_cast(modelData->subsets.size());
		std::string directory = Utility::FilePathControl::GetParentDirectory(dxd_path);
		if (!directory.empty()) directory += "/";
		for (int i = 0; i < modelData->materialCount; i++) {
			materials[modelData->subsets[i].materialName] = std::make_shared<Material>(modelData->materials[i].name.c_str(), (directory + modelData->materials[i].texturePath).c_str());
		}
		//�`�揇�Ƀ}�e���A���ւ̃|�C���^����
		renderIndexMaterial.resize(meshCount);
		subsets.resize(meshCount);
		for (int i = 0; i < meshCount; i++) {
			renderIndexMaterial[i] = materials[modelData->subsets[i].materialName].get();
			subsets[i] = { modelData->subsets[i].index,modelData->subsets[i].start,modelData->subsets[i].sum };
		}
		//���t���N�V�����J�n
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs.get());
		//���̓��C�A�E�g�쐬
		inputLayout = std::make_unique<InputLayout>(vs.get(), reflector.get());

	}
	ModelInstancedAnimation::~ModelInstancedAnimation() = default;
	AnimationNo ModelInstancedAnimation::LoadAnimation(const char* anm_path) {
		return animationEngine->LoadAnimation(anm_path);
	}
	void ModelInstancedAnimation::CreateAnimationFramesTexture() {
		animationEngine->CreateTexture();
	}
	float ModelInstancedAnimation::GetAnimationTimeMax(AnimationNo animation_index) { return animationEngine->AnimationTimeMax(animation_index); }
	void ModelInstancedAnimation::CalcAnimationMatrix(AnimationNo animation_index, float time, int bone_index, DirectX::XMFLOAT4X4* anim) { animationEngine->CalcAnimationMatrix(animation_index, time, bone_index, anim); }
	void ModelInstancedAnimation::Begin() {
		mapBuffer = instancingEngine->Map();
		renderCount = 0;
	}
	void ModelInstancedAnimation::Set(const InstancedData& data, int animation_index, float* time) {
		if (!mapBuffer)STRICT_THROW("Buffer��nullptr�ł� Map���s���Ă�������");
		if (renderCount >= instancingEngine->GetInstanceMax())STRICT_THROW("�o�b�t�@�̃T�C�Y���I�[�o�[���܂���");
		mapBuffer[renderCount] = data;
		animationEngine->SetData(renderCount++, animation_index, time);
	}
	void ModelInstancedAnimation::End() {
		mapBuffer = nullptr;
		instancingEngine->Unmap();
	}
	void ModelInstancedAnimation::Render() {
		if (mapBuffer)STRICT_THROW("Unmap���s���Ă�������");
		inputLayout->Set();
		Device::GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Activate();
		ID3D11Buffer* buffers[2] = { mesh->GetVertexBuffer().Get() , instancingEngine->GetBuffer().Get() };
		UINT stride[2] = { sizeof(Vertex), sizeof(InstancedData) };
		UINT offset[2] = { 0, 0 };
		Device::GetContext()->IASetVertexBuffers(0, 2, buffers, stride, offset);
		animationEngine->Activate();
		for each(auto subset in subsets) {
			subset.Render(this);
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	MultiTextureModel::MultiTextureModel(const char* dxd_path, const char* mt_path, const int instance_count) :ModelInstancedAnimation(dxd_path, mt_path, instance_count) {
		//Texture(const Math::Vector2& size, DXGI_FORMAT format, UINT bind_flags, const DXGI_SAMPLE_DESC& sample, ACCESS_FLAG access_flag = ACCESS_FLAG::DEFAULT, CPU_ACCESS_FLAG cpu_flag = CPU_ACCESS_FLAG::NONE, int array_count = 1);
		//std::make_unique<Texture>(size, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_SHADER_RESOURCE, desc, Texture::ACCESS_FLAG::DYNAMIC, Texture::CPU_ACCESS_FLAG::WRITE);
		DXGI_SAMPLE_DESC desc = { 1,0 };
		texture = std::make_unique<Texture>(Math::Vector2(instance_count, 1), DXGI_FORMAT_R32_UINT, D3D11_BIND_SHADER_RESOURCE, desc, Texture::ACCESS_FLAG::DYNAMIC, Texture::CPU_ACCESS_FLAG::WRITE);
		buffer = nullptr;
	}
	void MultiTextureModel::SetMultiTexture(const char* texture_path, int tex_slot)  {
		Lobelia::Graphics::TextureFileAccessor::Load(texture_path, &subTexture);
		/*TexInfo texInfo;
		Lobelia::Graphics::TextureFileAccessor::Load(texture_path, &texInfo.texture);
		texInfo.slot = tex_slot;
		textures.push_back(texInfo);*/
	}
	void MultiTextureModel::Begin() {
		ModelInstancedAnimation::Begin();
		renderCount = 0;
		HRESULT hr = Device::GetContext()->Map(texture->Get().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		buffer = static_cast<UINT*>(resource.pData);
		if (FAILED(hr)) STRICT_THROW("�݂����Ă�[�[�[�[");
	}
	void MultiTextureModel::Set(const InstancedData& data, int animationIndex, float* time, int type) {
		ModelInstancedAnimation::Set(data, animationIndex, time);
		buffer[renderCount] = type;
		renderCount++;
	}
	void MultiTextureModel::End() {
		ModelInstancedAnimation::End();
		Device::GetContext()->Unmap(texture->Get().Get(), 0);
		buffer = nullptr;
	}
	void MultiTextureModel::Render() {
	/*	for (auto&& textureInfo : textures) {
			textureInfo.texture->Set(textureInfo.slot,Lobelia::Graphics::ShaderStageList::PS);
		}*/
		subTexture->Set(7, ShaderStageList::PS);
		texture->Set(17, ShaderStageList::VS);

		ModelInstancedAnimation::Render();
	}

}

