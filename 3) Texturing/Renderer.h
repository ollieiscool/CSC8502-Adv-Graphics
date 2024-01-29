#pragma once
#include "../nclgl/OGLRenderer.h";

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	virtual ~Renderer(void);

	virtual void RenderScene();

	void UpdateTextureMatrix(float rotation);
	void ToggleRepeating();
	void ToggleFiltering();

protected:
	Shader* shader;
	Mesh* triangle;
	GLuint texture;
	bool filtering;
	bool repeating;
};