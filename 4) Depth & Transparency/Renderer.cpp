#include "Renderer.h";

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	meshes[0] = Mesh::GenerateQuad();
	meshes[1] = Mesh::GenerateTriangle();

	textures[0] = SOIL_load_OGL_texture(TEXTUREDIR"brick.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	textures[1] = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	if (!textures[0] || !textures[1]) {
		return;
	}

	positions[0] = Vector3(0, 0, -5);
	positions[1] = Vector3(0, 0, -5);

	shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	if (!shader->LoadSuccess()) {
		return;
	}

	usingDepth = false;
	usingAlpha = false;
	blendMode = 0;
	modifyObject = true;

	projMatrix = Matrix4::Perspective(1.0f, 100.0f, (float)width / (float)height, 45.0f);
	init = true;
}

Renderer::~Renderer(void) {
	delete meshes[0];
	delete meshes[1];
	delete shader;
	glDeleteTextures(2, textures);
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	for (unsigned int i = 0; i < 2; i++) {
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, (float*)&Matrix4::Translation(positions[i]));
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		meshes[i]->Draw();
	}
}

void Renderer::ToggleObject() {
	modifyObject = !modifyObject;
}

void Renderer::MoveObject(float by) {
	positions[(int)modifyObject].z += by;
}

void Renderer::ToggleBlendMode() {
	blendMode = (blendMode + 1) % 4;
	switch (blendMode) {
	case(0): glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
	case(1): glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR); break;
	case(2): glBlendFunc(GL_ONE, GL_ZERO); break;
	case(3): glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
	};
}

void Renderer::ToggleDepth() {
	usingDepth = !usingDepth;
	usingDepth ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void Renderer::ToggleAlphaBlend() {
	usingAlpha = !usingAlpha;
	usingAlpha ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
}