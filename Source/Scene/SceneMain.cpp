#include "Lobelia.hpp"
#include "SceneMain.hpp"

//個人製作 第二弾
//第三弾はグラフィック的に良いものを作ろうと画策中

namespace Lobelia::Game {
	//研究スペース
	//namespace Develop {
	//	Material::Material(const char* file_path, const char* instance_name) {

	//	}

	//	void Renderer::SetMaterial(std::shared_ptr<Material>& material) { this->material = material; }
	//	std::shared_ptr<Material>& Renderer::GetMaterial() { return material; }
	//	void Renderer::SetVertexShader(std::shared_ptr<Graphics::VertexShader>& vs) { this->vs = vs; }
	//	std::shared_ptr<Graphics::VertexShader>&  Renderer::GetVertexShader() { return vs; }
	//	void Renderer::SetPixelShader(std::shared_ptr<Graphics::PixelShader>& ps) { this->ps = ps; }
	//	std::shared_ptr<Graphics::PixelShader>&  Renderer::GetPixelShader() { return ps; }

	//	std::shared_ptr<Graphics::VertexShader> ShaderBufferObject::CreateVertexShader(const char* include, const char* vs) {

	//	}
	//	std::shared_ptr<Graphics::PixelShader> ShaderBufferObject::CreatePixelShader(const char* include, const char* ps) {

	//	}

	//	MeshRenderer::MeshRenderer() {

	//	}
	//	void MeshRenderer::SetFilter(std::shared_ptr<MeshFilter> mesh) {

	//	}
	//	void MeshRenderer::Render() {

	//	}

	//	SkinMeshRenderer::SkinMeshRenderer() {

	//	}
	//	void SkinMeshRenderer::Render() {

	//	}
	//}

	SceneMain::SceneMain() :view(std::make_unique<Graphics::View>(Math::Vector2(), Application::GetInstance()->GetWindow()->GetSize())) {
	}
	SceneMain::~SceneMain() {
	}
	void SceneMain::Initialize() {
	}
	void SceneMain::AlwaysUpdate() {
	}
	void SceneMain::AlwaysRender() {
		view->Activate();
	}
}
