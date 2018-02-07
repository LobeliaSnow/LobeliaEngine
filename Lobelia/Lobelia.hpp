#pragma once
#ifndef LOBELIA_ENGINE

//friendは人によって毛嫌いされるかもしれないですが、
//あくまでも関係のないところから触られたくないという観点からfriend指定の部分はしています。
//ここであえてゲッターセッターを作ってしまうと、ユーザーの手によりこちらの予期せぬ動作をするかもしれないと考えたからです。

//TODO : abstarct interface final const noexcept等、指定子を付けていく。
#include "Network/Network.hpp"
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
//#include "Movie/Movie.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/ShaderBank.hpp"
#include "Graphics/GPUParticle/GPUParticle.hpp"
#include "Graphics/Shader/Reflection/Reflection.hpp"
#include "Graphics/InputLayout/InputLayout.hpp"
#include "Graphics/Material/Material.hpp"
#include "Graphics/Mesh/Mesh.hpp"
#include "Graphics/Transform/Transform.hpp"
#include "Graphics/Sprite/Sprite.hpp"
#include "Graphics/Model/Model.hpp"
#include "Graphics/Environment/Environment.hpp"
#include "Graphics/SwapChain/SwapChain.hpp"
#include "Graphics/SwapChain/ToneCurve/ToneCurve.hpp"
#include "Graphics/RenderableObject/RenderableObject.hpp"
#include "Graphics/Renderer/Renderer.hpp"
#include "Console/Console.hpp"
#include "Graphics/Pipeline/Pipeline.hpp"
#include "Graphics/Direct2D/Direct2DSystem.hpp"
#include "Input/DeviceList/Device.hpp"
#include "Input/Data/Data.hpp"
#include "Input/Keyboard/Keyboard.hpp"
#include "Input/Controller/DS4/DS4.hpp"
#include "Input/Mouse/Mouse.hpp"
#include "Input/DeviceList/DeviceManager.hpp"
#include "Scene/Scene.hpp"
#include "Application/Application.hpp"
#include "Graphics/Experimental/Effect.hpp"
#include "Audio/Device/Device.hpp"
#include "Audio/Voice/Voice.hpp"
#include "Audio/Loader/Loader.hpp"
#include "Audio/Bank/Bank.hpp"
#include "XML/XML.hpp"

#include "Game/SelectModule.hpp"
#include "Game/Ranking.hpp"
#include "Game/Collision/Collision.hpp"
#include "Game/GameObject/GameObject.hpp"
#include "Game/StringView.hpp"

namespace Lobelia {
	void CreateStencilStatePreset();
	void CreateDefaultPipeline();
	void Bootup();
	void Shutdown();
}
#endif

