#include "Lobelia.hpp"

namespace Lobelia::Graphics {
	//----------------------------------------------------------------------------------------------------
	//	GPUParticle::ElapsedTime
	//----------------------------------------------------------------------------------------------------
	void GPUParticleSystem::Info::Update(float elapsed_time) {
		elapsedTime = elapsed_time;
	}
	//----------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------
	//	GPUParticle::RWByteAddressBuffer
	//----------------------------------------------------------------------------------------------------
	GPUParticleSystem::RWByteAddressBuffer::RWByteAddressBuffer(void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args) {
		//�o�b�t�@�쐬
		CreateRWByteAddressBuffer(buffer, init_buffer, element_size, element_count, is_vertex_buffer, is_index_buffer, is_indirect_args);
		//UAV�쐬
		CreateUAV(uav, buffer);
	}
	GPUParticleSystem::RWByteAddressBuffer::~RWByteAddressBuffer() = default;
	void GPUParticleSystem::RWByteAddressBuffer::CreateRWByteAddressBuffer(ComPtr<ID3D11Buffer>& buffer, void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args) {
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		//���_�o�b�t�@�Ƃ��ăo�C���h���邩�ۂ�
		if (is_vertex_buffer)bindFlags |= D3D11_BIND_VERTEX_BUFFER;
		//�C���f�b�N�X�o�b�t�@�Ƃ��ăo�C���h���邩�ۂ�
		if (is_index_buffer)bindFlags |= D3D11_BIND_INDEX_BUFFER;
		UINT miscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		//Indirect�n�̈����Ƃ��Ďg�p���邩�ۂ�
		if (is_indirect_args)miscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		//�o�b�t�@�쐬
		BufferCreator::Create(buffer.GetAddressOf(), init_buffer, element_size*element_count, D3D11_USAGE_DEFAULT, bindFlags, 0, element_size, miscFlags);
	}
	void GPUParticleSystem::RWByteAddressBuffer::CreateUAV(ComPtr<ID3D11UnorderedAccessView>& uav, const ComPtr<ID3D11Buffer>& buffer) {
		D3D11_BUFFER_DESC bufferDesc;
		//�o�b�t�@�̏��擾
		buffer->GetDesc(&bufferDesc);
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		if (bufferDesc.MiscFlags == D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) {
			//�\�����o�b�t�@�̏ꍇ
			//�v�f��
			uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		}
		else if (bufferDesc.MiscFlags == D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS || bufferDesc.MiscFlags&D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS) {
			//�v�f��(1�v�f��float����unsigned int�Ȃ̂�4�Ŋ���)
			uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / 4UL;
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		}
		//��̏������Ƃ�UAV�쐬
		Device::Get()->CreateUnorderedAccessView(buffer.Get(), &uavDesc, uav.GetAddressOf());
	}
	void GPUParticleSystem::RWByteAddressBuffer::ResourceUpdate(void* data_buffer, UINT element_size, UINT element_count) {
		//�܂��{�b�N�X�̕����������Ă��Ȃ��̂Œ����K�v
		D3D11_BOX copyRange = { 0,0,0,element_size*element_count,1,1 };
		Device::GetContext()->UpdateSubresource(buffer.Get(), 0, &copyRange, data_buffer, element_size, element_count);
	}
	//----------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------
	//	GPUParticle::Particle
	//----------------------------------------------------------------------------------------------------
	GPUParticleSystem::Particle::Particle(const Math::Vector3& pos, const Math::Vector3& move, const Math::Vector3& power, int texture_index, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, float alive_time, float fade_in_time, float fade_out_time, float start_scale, float end_scale, float start_rad, float end_rad, Utility::Color color) :
		pos(pos), move(move), power(power), textureIndex(texture_index), uvPos(uv_pos), uvSize(uv_size), aliveTime(alive_time), elapsedTime(alive_time), fadeInTime(fade_in_time), fadeOutTime(fade_out_time), startScale(start_scale), endScale(end_scale), startRad(start_rad), endRad(end_rad), color(color.r, color.g, color.b) {}
	GPUParticleSystem::Particle::Particle() : Particle({}, {}, {}, -1, {}, {}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xFFFFFF) {	}
	GPUParticleSystem::Particle::~Particle() = default;
	//----------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------
	//	GPUParticle
	//----------------------------------------------------------------------------------------------------
	const std::string GPUParticleSystem::BLEND_LIST[4] = { "Copy" ,"Add" ,"Sub" ,"Screen" };
	//�u�����h�v���Z�b�g
	GPUParticleSystem::GPUParticleSystem() :textureList{} {
		//�o�b�t�@�쐬
		infoCBuffer = std::make_unique<ConstantBuffer<Info>>(0, ShaderStageList::CS);
		appendData = std::make_unique<RWByteAddressBuffer>(appendParticles, s_cast<UINT>(sizeof(Particle)), APPEND_PARTICLE_MAX, false, false, false);
		UINT initIndexBuffer[GPU_PARTICLE_MAX] = {};
		for (int i = 0; i < GPU_PARTICLE_MAX; i++) {
			initIndexBuffer[i] = i;
		}
		indexBuffer = std::make_unique<RWByteAddressBuffer>(initIndexBuffer, s_cast<UINT>(sizeof(UINT)), GPU_PARTICLE_MAX, false, true, false);
		dataBuffer = std::make_unique<RWByteAddressBuffer>(nullptr, s_cast<UINT>(sizeof(Particle)), GPU_PARTICLE_MAX, true, false, false);
		UINT initArgsBuffer[5] = {};
		indirectArgs = std::make_unique<RWByteAddressBuffer>(initArgsBuffer, 4UL, 5UL, false, false, true);
		//ComputeShader�쐬
		appendCS = std::make_unique<ComputeShader>("Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "AppendParticle");
		sortCS = std::make_unique<ComputeShader>("Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "SortParticle");
		updateCS = std::make_unique<ComputeShader>("Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "UpdateParticle");
		//GeometryShader�쐬
		gs = std::make_unique<GeometryShader>("Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "GPUParticleGS");
		//VertexShader,PixelShader�쐬
		ShaderBank::Register<VertexShader>("GPUParticle", "Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "GPUParticleVS", VertexShader::Model::VS_4_0);
		VertexShader* vs = ShaderBank::Get<VertexShader>("GPUParticle");
		//���t���N�V�����J�n
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs);
		//���̓��C�A�E�g�쐬
		inputLayout = std::make_unique<InputLayout>(vs, reflector.get());

		ShaderBank::Register<PixelShader>("GPUParticle", "Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "GPUParticlePS", PixelShader::Model::PS_5_0);
		//�p�C�v���C���\�z
		pipeline = std::make_unique<Pipeline>("Add", "Point", D_SOFF_ZOFF, "Cull None", "GPUParticle", 0, nullptr, "GPUParticle", 0, nullptr);
	}
	GPUParticleSystem::~GPUParticleSystem() = default;
	void GPUParticleSystem::SetTexture(int slot, Texture* texture) {
		if (slot >= TEXTURE_COUNT)STRICT_THROW("�Z�b�g�\�ȃe�N�X�`���̍ő吔�𒴂��܂���");
		textureList[slot] = texture;
	}
	void GPUParticleSystem::Append(const Particle& particle) {
		if (info.appendCount >= APPEND_PARTICLE_MAX)STRICT_THROW("1���[�v�Œǉ��ł���p�[�e�B�N���̌��E�l�𒴂��܂���");
		appendParticles[info.appendCount++] = particle;
	}
	void GPUParticleSystem::RunAppend() {
		//GPU�̃o�b�t�@�Ƀf�[�^���ڂ�
		appendData->ResourceUpdate(&appendParticles, sizeof(Particle), info.appendCount);
		//uav�̐ݒ�
		ID3D11UnorderedAccessView* uavs[4] = { appendData->uav.Get(),indexBuffer->uav.Get(),dataBuffer->uav.Get() ,indirectArgs->uav.Get() };
		ComputeShader::SetUnorderedAccessView(0, 4, uavs);
		//GPU�ւ̃p�[�e�B�N���̒ǉ������s
		appendCS->Run(1, 1, 1);
		info.appendCount = 0;
		ID3D11UnorderedAccessView* resetUavs[4] = {};
		ComputeShader::SetUnorderedAccessView(0, 4, resetUavs);
	}
	void GPUParticleSystem::RunSort() {
		//uav�̐ݒ�
		ID3D11UnorderedAccessView* uavs[4] = { appendData->uav.Get(),indexBuffer->uav.Get(),dataBuffer->uav.Get() ,indirectArgs->uav.Get() };
		ComputeShader::SetUnorderedAccessView(0, 4, uavs);
		//�����v�f�����p�[�e�B�N������������܂ŉ�
		for (UINT divideLevel = 2; divideLevel <= GPU_PARTICLE_MAX; divideLevel <<= 1) {
			for (UINT subDivision = divideLevel; 1 < subDivision; subDivision >>= 1) {
				info.compareInterval = subDivision >> 1;
				info.divideLevel = divideLevel;
				info.isBitonicFinal = (divideLevel == GPU_PARTICLE_MAX && subDivision == 2);
				//�萔�o�b�t�@�̍X�V
				infoCBuffer->Activate(info);
				//�\�[�g���s
				sortCS->Run(GPU_PARTICLE_MAX / LOCAL_THREAD_COUNT, 1, 1);
			}
		}
		ID3D11UnorderedAccessView* resetUavs[4] = {};
		ComputeShader::SetUnorderedAccessView(0, 4, resetUavs);
	}
	void GPUParticleSystem::RunUpdate() {
		ID3D11UnorderedAccessView* uavs[4] = { appendData->uav.Get(),indexBuffer->uav.Get(),dataBuffer->uav.Get() ,indirectArgs->uav.Get() };
		ComputeShader::SetUnorderedAccessView(0, 4, uavs);
		//�p�[�e�B�N���X�V����
		updateCS->Run(GPU_PARTICLE_MAX / (LOCAL_THREAD_COUNT*THREAD_PER_COUNT), 1, 1);
		ID3D11UnorderedAccessView* resetUavs[4] = {};
		ComputeShader::SetUnorderedAccessView(0, 4, resetUavs);
	}
	void GPUParticleSystem::Update(float time_scale) {
		//�o�ߎ��ԍX�V
		info.elapsedTime = Application::GetInstance()->GetProcessTime()*time_scale / 1000.0f;
		//�萔�o�b�t�@�X�V
		infoCBuffer->Activate(info);
		//�p�[�e�B�N���ǉ�����Ă��Ȃ��ꍇ�̓X���[ GPU�ւ̃p�[�e�B�N���ǉ����s
		if (info.appendCount > 0)RunAppend();
		//�\�[�g���s
		RunSort();
		RunUpdate();
	}
	void GPUParticleSystem::Render(BlendMode mode) {
		for (int i = 0; i < TEXTURE_COUNT; i++) {
			if (textureList[i])textureList[i]->Set(i + 10, ShaderStageList::PS);
		}
		UINT stride = sizeof(Particle);
		UINT offset = 0;
		pipeline->BlendSet(BLEND_LIST[i_cast(mode)]);
		//�d�v
		Device::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		Device::GetContext()->IASetVertexBuffers(0, 1, dataBuffer->buffer.GetAddressOf(), &stride, &offset);
		Device::GetContext()->IASetIndexBuffer(indexBuffer->buffer.Get(), DXGI_FORMAT_R32_UINT, offset);
		pipeline->Activate(true);
		gs->Set();
		inputLayout->Set();
		Device::GetContext()->DrawIndexedInstancedIndirect(indirectArgs->buffer.Get(), 0);
		Device::GetContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, offset);
		ID3D11Buffer* resetBuffer = nullptr;
		Device::GetContext()->IASetVertexBuffers(0, 1, &resetBuffer, &stride, &offset);
		//GeometryShader�̃N���A
		Device::GetContext()->GSSetShader(nullptr, nullptr, 0);
	}

	//----------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------
}