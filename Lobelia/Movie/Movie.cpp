#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Exception/Exception.hpp"
#include "Graphics/Transform/Transform.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Config/Config.hpp"
#include "Graphics/DisplayInfo/DisplayInfo.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/RenderState/RenderState.hpp"
#include "Graphics/View/View.hpp"
#include "Graphics/RenderTarget/RenderTarget.hpp"
#include "Graphics/Texture/Texture.hpp"
#include "Movie.hpp"
#include "Exception/Exception.hpp"
namespace Lobelia::Movie {
	//とりあえず適当、一度うって全貌をつかむ
	//http://yuhr.tumblr.com/post/126355174601/mediafoundation%E3%81%A7id3d11texture2d%E3%81%AB%E5%8B%95%E7%94%BB%E3%81%AE%E3%83%95%E3%83%AC%E3%83%BC%E3%83%A0%E3%82%92%E8%AA%AD%E3%81%BF%E8%BE%BC%E3%82%80-%E8%A6%9A%E6%9B%B8%CE%B2
	//音声取得できそうなサイト
	//http://any-programming.hatenablog.com/entry/2017/03/11/223603
	//再生はできているけど設計考えずとりあえずテスト用に上から下まで書いた、
	//現状動きが後半怪しい
	//他にも情報が全く取れていなかったりとかなのでとりあえず凍結
	void MovieSystem::Bootup() { MFStartup(MF_VERSION); }
	void MovieSystem::Shutdown() { MFShutdown(); }
	Loader::Loader(const char* file_path) {
		HRESULT hr = S_OK;
		//読み込み
		hr = MFCreateSourceReaderFromURL(Utility::ConverteWString(file_path).c_str(), nullptr, reader.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("動画ファイルの読み込みに失敗"); 
		//IYUV
		hr = MFCreateMediaType(iyuv.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("メディアタイプ作成に失敗");
		hr = iyuv->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		if (FAILED(hr))STRICT_THROW("GUIDの設定に失敗");
		hr = iyuv->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_IYUV);
		if (FAILED(hr))STRICT_THROW("GUIDの設定に失敗");
		hr = iyuv->SetUINT32(MF_MT_DEFAULT_STRIDE, 1280);
		if (FAILED(hr))STRICT_THROW("ストライドの設定に失敗");
		hr = MFSetAttributeRatio((IMFAttributes*)iyuv.Get(), MF_MT_FRAME_RATE, 24, 1);
		if (FAILED(hr))STRICT_THROW("レート設定に失敗");
		hr = MFSetAttributeSize((IMFAttributes*)iyuv.Get(), MF_MT_FRAME_SIZE, 1280, 720);
		if (FAILED(hr))STRICT_THROW("サイズ設定に失敗");
		hr = iyuv->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
		if (FAILED(hr))STRICT_THROW("インターレース設定に失敗");
		hr = iyuv->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
		if (FAILED(hr))STRICT_THROW("サンプル設定に失敗");
		hr = MFSetAttributeRatio((IMFAttributes*)iyuv.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
		if (FAILED(hr))STRICT_THROW("アスペクト比設定に失敗");
		//ARGB
		hr = MFCreateMediaType(argb32.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("メディアタイプ作成に失敗");
		hr = argb32->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		if (FAILED(hr))STRICT_THROW("GUIDの設定に失敗");
		hr = argb32->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
		if (FAILED(hr))STRICT_THROW("GUIDの設定に失敗");
		hr = argb32->SetUINT32(MF_MT_DEFAULT_STRIDE, 1280 * 4);
		if (FAILED(hr))STRICT_THROW("ストライドの設定に失敗");
		hr = MFSetAttributeRatio((IMFAttributes*)argb32.Get(), MF_MT_FRAME_RATE, 24, 1);
		if (FAILED(hr))STRICT_THROW("レート設定に失敗");
		hr = MFSetAttributeSize((IMFAttributes*)argb32.Get(), MF_MT_FRAME_SIZE, 1280, 720);
		if (FAILED(hr))STRICT_THROW("サイズ設定に失敗");
		hr = argb32->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
		if (FAILED(hr))STRICT_THROW("インターレース設定に失敗");
		hr = argb32->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
		if (FAILED(hr))STRICT_THROW("サンプル設定に失敗");
		hr = MFSetAttributeRatio((IMFAttributes*)argb32.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
		if (FAILED(hr))STRICT_THROW("アスペクト比設定に失敗");
		hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, iyuv.Get());
		if (FAILED(hr))STRICT_THROW("メディアタイプの設定に失敗");

		IMFActivate** activates = nullptr;
		UINT32 activateCount = 0; // will be 1
		hr = MFTEnumEx(MFT_CATEGORY_VIDEO_PROCESSOR, MFT_ENUM_FLAG_ALL, nullptr, nullptr, &activates, &activateCount);
		if (FAILED(hr) || activateCount <= 0)STRICT_THROW("MFTの列挙に失敗");
		//本当はここでループ回して適切なもの比較しないといけないみたいだが、ちょっとよくわかんない
		hr = activates[0]->ActivateObject(__uuidof(IMFTransform), (void**)&mft);
		if (FAILED(hr))STRICT_THROW("MFT有効化に失敗");
		activates[0]->Release();
		activates[0] = nullptr;
		CoTaskMemFree(activates);
		//変換設定 iyuv->argb
		hr = mft->SetInputType(0, iyuv.Get(), 0);
		if (FAILED(hr))STRICT_THROW("MFT有効化に失敗");
		hr = mft->SetOutputType(0, argb32.Get(), 0);
		if (FAILED(hr))STRICT_THROW("MFT有効化に失敗");
		texture = std::make_unique<Graphics::Texture>(Math::Vector2(1280, 720), DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, DXGI_SAMPLE_DESC{ 1,0 });
	}
	void Loader::Update() {
		HRESULT hr = S_OK;
		//よくわかんない
		DWORD streamIndex, rsflags;
		LONGLONG llTimeStamp;
		IMFSample* sample = nullptr;
		//IYUVのデータを毎フレーム取得
		hr = reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &rsflags, &llTimeStamp, &sample);
		if (FAILED(hr))STRICT_THROW("サンプル読み込みに失敗");
		//ARGBに変換するためにデータを流す
		hr = mft->ProcessInput(0, sample, 0);
		//if (FAILED(hr))STRICT_THROW("ARGB変換(Input)に失敗");
		DWORD status = 0;
		//この先出力
		hr = mft->GetOutputStatus(&status);
		if (FAILED(hr))STRICT_THROW("ARGB変換(Output)の続行が不可能です");
		if (status & MFT_OUTPUT_STATUS_SAMPLE_READY) {
			//2Dバッファ作成
			IMFMediaBuffer* mb;
			const DWORD D3DFMT_A8R8G8B8 = 21;
			hr = MFCreate2DMediaBuffer(1280, 720, D3DFMT_A8R8G8B8, false, &mb);
			if (FAILED(hr))STRICT_THROW("2Dバッファの作成に失敗");
			//変換結果の出力先
			MFT_OUTPUT_DATA_BUFFER argb32Buffer;
			ZeroMemory(&argb32Buffer, sizeof(MFT_OUTPUT_DATA_BUFFER));
			hr = MFCreateSample(&argb32Buffer.pSample);
			if (FAILED(hr))STRICT_THROW("サンプル作成に失敗");
			hr = argb32Buffer.pSample->AddBuffer(mb);
			if (FAILED(hr))STRICT_THROW("バッファの追加に失敗");
			//変換実行
			hr = mft->ProcessOutput(0, 1, &argb32Buffer, &status);
			if (FAILED(hr))return;/*STRICT_THROW("ARGB変換(Output)に失敗");*/
			//デコード後の画像を得るためにクエリ
			IMF2DBuffer* buf2d;
			hr = mb->QueryInterface(__uuidof(IMF2DBuffer), (void**)&buf2d);
			if (FAILED(hr))STRICT_THROW("クエリに失敗");
			//ここからおなじみ
			unsigned char* framedata = nullptr;
			LONG pitch;
			buf2d->Lock2D(&framedata, &pitch);
			Graphics::Device::GetContext()->UpdateSubresource(texture->Get().Get(), 0, nullptr, framedata, pitch, 0);
			buf2d->Unlock2D();
			buf2d->Release();
			mb->Release();
			if(sample)sample->Release();
		}
	}
}

