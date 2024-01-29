#pragma once
#include "../nclgl/OGLRenderer.h";

class Camera;
class Mesh;
class HeightMap;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void FillBuffers(); //G-Buffer Fill render pass
	void DrawPointLights(); //Lighting render pass
	void CombineBuffers(); //Combination render pass

	void GenerateScreenTexture(GLuint& into, bool depth = false);

	Shader* sceneShader; //GBuffer shader
	Shader* pointlightShader; //Lighting Shader
	Shader* combineShader; 

	GLuint bufferFBO; //Gbuffer pass
	GLuint bufferColourTex;
	GLuint bufferNormalTex;
	GLuint bufferDepthTex;

	GLuint pointLightFBO;
	GLuint lightDiffuseTex;
	GLuint lightSpecularTex;

	HeightMap* heightMap;
	Light* pointLights;
	Mesh* sphere;
	Mesh* quad;
	Camera* camera;
	GLuint earthTex;
	GLuint earthBump;
};