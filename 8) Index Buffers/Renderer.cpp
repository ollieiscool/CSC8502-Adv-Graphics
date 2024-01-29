#include "Renderer.h";
#include "../nclgl/Camera.h";
#include "../nclgl/HeightMap.h";

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	heightMap = new HeightMap(TEXTUREDIR"noise.png");
	camera = new Camera(-40, 270, Vector3());

	Vector3 dimensions = heightMap->GetHeightMapSize();
	camera->setPosition(dimensions * Vector3(0.5, 2, 0.5));

	shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	if (!shader->LoadSuccess()) {
		return;
	}

	terrainTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!terrainTex) {
		return;
	}
	SetTextureRepeating(terrainTex, true);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	init = true;
}

Renderer::~Renderer(void) {
	delete heightMap;
	delete camera;
	delete shader;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTex);
	heightMap->Draw();
}