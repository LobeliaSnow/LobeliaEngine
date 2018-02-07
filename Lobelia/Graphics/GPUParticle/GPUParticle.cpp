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
		//バッファ作成
		CreateRWByteAddressBuffer(buffer, init_buffer, element_size, element_count, is_vertex_buffer, is_index_buffer, is_indirect_args);
		//UAV作成
		CreateUAV(uav, buffer);
	}
	GPUParticleSystem::RWByteAddressBuffer::~RWByteAddressBuffer() = default;
	void GPUParticleSystem::RWByteAddressBuffer::CreateRWByteAddressBuffer(ComPtr<ID3D11Buffer>& buffer, void* init_buffer, UINT element_size, UINT element_count, bool is_vertex_buffer, bool is_index_buffer, bool is_indirect_args) {
		UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		//頂点バッファとしてバインドするか否か
		if (is_vertex_buffer)bindFlags |= D3D11_BIND_VERTEX_BUFFER;
		//インデックスバッファとしてバインドするか否か
		if (is_index_buffer)bindFlags |= D3D11_BIND_INDEX_BUFFER;
		UINT miscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		//Indirect系の引数として使用するか否か
		if (is_indirect_args)miscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		//バッファ作成
		BufferCreator::Create(buffer.GetAddressOf(), init_buffer, element_size*element_count, D3D11_USAGE_DEFAULT, bindFlags, 0, element_size, miscFlags);
	}
	void GPUParticleSystem::RWByteAddressBuffer::CreateUAV(ComPtr<ID3D11UnorderedAccessView>& uav, const ComPtr<ID3D11Buffer>& buffer) {
		D3D11_BUFFER_DESC bufferDesc;
		//バッファの情報取得
		buffer->GetDesc(&bufferDesc);
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		if (bufferDesc.MiscFlags == D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) {
			//構造化バッファの場合
			//要素数
			uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		}
		else if (bufferDesc.MiscFlags == D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS || bufferDesc.MiscFlags&D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS) {
			//要素数(1要素はfloat又はunsigned intなので4で割る)
			uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / 4UL;
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		}
		//上の情報をもとにUAV作成
		Device::Get()->CreateUnorderedAccessView(buffer.Get(), &uavDesc, uav.GetAddressOf());
	}
	void GPUParticleSystem::RWByteAddressBuffer::ResourceUpdate(void* data_buffer, UINT element_size, UINT element_count) {
		//まだボックスの部分分かっていないので調査必要
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
	//ブレンドプリセット
	GPUParticleSystem::GPUParticleSystem() :textureList{} {
		//バッファ作成
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
		//ComputeShader作成
		appendCS = std::make_unique<ComputeShader>("Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "AppendParticle");
		sortCS = std::make_unique<ComputeShader>("Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "SortParticle");
		updateCS = std::make_unique<ComputeShader>("Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "UpdateParticle");
		//GeometryShader作成
		gs = std::make_unique<GeometryShader>("Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "GPUParticleGS");
		//VertexShader,PixelShader作成
		ShaderBank::Register<VertexShader>("GPUParticle", "Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "GPUParticleVS", VertexShader::Model::VS_4_0);
		VertexShader* vs = ShaderBank::Get<VertexShader>("GPUParticle");
		//リフレクション開始
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs);
		//入力レイアウト作成
		inputLayout = std::make_unique<InputLayout>(vs, reflector.get());

		ShaderBank::Register<PixelShader>("GPUParticle", "Data/ShaderFile/GPUParticle/GPUParticle.hlsl", "GPUParticlePS", PixelShader::Model::PS_5_0);
		//パイプライン構築
		pipeline = std::make_unique<Pipeline>("Add", "Point", D_SOFF_ZOFF, "Cull None", "GPUParticle", 0, nullptr, "GPUParticle", 0, nullptr);
	}
	GPUParticleSystem::~GPUParticleSystem() = default;
	void GPUParticleSystem::SetTexture(int slot, Texture* texture) {
		if (slot >= TEXTURE_COUNT)STRICT_THROW("セット可能なテクスチャの最大数を超えました");
		textureList[slot] = texture;
	}
	void GPUParticleSystem::Append(const Particle& particle) {
		if (info.appendCount >= APPEND_PARTICLE_MAX)STRICT_THROW("1ループで追加できるパーティクルの限界値を超えました");
		appendParticles[info.appendCount++] = particle;
	}
	void GPUParticleSystem::RunAppend() {
		//GPUのバッファにデータを移す
		appendData->ResourceUpdate(&appendParticles, sizeof(Particle), info.appendCount);
		//uavの設定
		ID3D11UnorderedAccessView* uavs[4] = { appendData->uav.Get(),indexBuffer->uav.Get(),dataBuffer->uav.Get() ,indirectArgs->uav.Get() };
		ComputeShader::SetUnorderedAccessView(0, 4, uavs);
		//GPUへのパーティクルの追加を実行
		appendCS->Run(1, 1, 1);
		info.appendCount = 0;
		ID3D11UnorderedAccessView* resetUavs[4] = {};
		ComputeShader::SetUnorderedAccessView(0, 4, resetUavs);
	}
	void GPUParticleSystem::RunSort() {
		//uavの設定
		ID3D11UnorderedAccessView* uavs[4] = { appendData->uav.Get(),indexBuffer->uav.Get(),dataBuffer->uav.Get() ,indirectArgs->uav.Get() };
		ComputeShader::SetUnorderedAccessView(0, 4, uavs);
		//分割要素数がパーティクル総数超えるまで回す
		for (UINT divideLevel = 2; divideLevel <= GPU_PARTICLE_MAX; divideLevel <<= 1) {
			for (UINT subDivision = divideLevel; 1 < subDivision; subDivision >>= 1) {
				info.compareInterval = subDivision >> 1;
				info.divideLevel = divideLevel;
				info.isBitonicFinal = (divideLevel == GPU_PARTICLE_MAX && subDivision == 2);
				//定数バッファの更新
				infoCBuffer->Activate(info);
				//ソート実行
				sortCS->Run(GPU_PARTICLE_MAX / LOCAL_THREAD_COUNT, 1, 1);
			}
		}
		ID3D11UnorderedAccessView* resetUavs[4] = {};
		ComputeShader::SetUnorderedAccessView(0, 4, resetUavs);
	}
	void GPUParticleSystem::RunUpdate() {
		ID3D11UnorderedAccessView* uavs[4] = { appendData->uav.Get(),indexBuffer->uav.Get(),dataBuffer->uav.Get() ,indirectArgs->uav.Get() };
		ComputeShader::SetUnorderedAccessView(0, 4, uavs);
		//パーティクル更新処理
		updateCS->Run(GPU_PARTICLE_MAX / (LOCAL_THREAD_COUNT*THREAD_PER_COUNT), 1, 1);
		ID3D11UnorderedAccessView* resetUavs[4] = {};
		ComputeShader::SetUnorderedAccessView(0, 4, resetUavs);
	}
	void GPUParticleSystem::Update(float time_scale) {
		//経過時間更新
		info.elapsedTime = Application::GetInstance()->GetProcessTime()*time_scale / 1000.0f;
		//定数バッファ更新
		infoCBuffer->Activate(info);
		//パーティクル追加されていない場合はスルー GPUへのパーティクル追加実行
		if (info.appendCount > 0)RunAppend();
		//ソート実行
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
		//重要
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
		//GeometryShaderのクリア
		Device::GetContext()->GSSetShader(nullptr, nullptr, 0);
	}

	//----------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------
}