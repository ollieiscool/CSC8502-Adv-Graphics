#pragma once
#include "../nclgl/OGLRenderer.h";
#include "../nclgl/SceneNode.h";
#include "../nclgl/Frustum.h"

class Camera;
class SceneNode;
class Mesh;
class Shader;
class HeightMap;
class Light;
class MeshAnimation;
class MeshMaterial;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void UpdateScene(float msec) override;
	void RenderScene() override;
	void SwitchToSpace();
	void SwitchToLand();
	void ToggleFreeCam();

protected:
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void DrawHeightMap(SceneNode* n);
	void DrawSkybox();
	void DrawWater(SceneNode* n);
	void DrawTroops(SceneNode* n);
	void DrawMoon(SceneNode* n);
	void MoonSpin(SceneNode* n);
	void DrawPlanet(SceneNode* n);
	void DrawAsteroid(SceneNode* n);
	void AsteroidSpin(SceneNode* n);
	void CameraMovement();

	SceneNode* root;
	SceneNode* heightMapNode;
	SceneNode* waterNode;
	SceneNode* troopNode;
	SceneNode* moonNode;
	SceneNode* moonSpinner;
	SceneNode* spaceRoot;
	SceneNode* planet;
	SceneNode* asteroid;
	SceneNode* asteroidSpinner;
	Camera* camera;
	Mesh* quad;
	Mesh* cube;
	Mesh* sphere;
	Mesh* asteroidMesh;

	Shader* sceneShader;
	Shader* bumpShader;
	Shader* skyboxShader;
	Shader* reflectShader;
	Shader* skinningShader;
	Shader* shadowSceneShader;

	GLuint earthTex;
	GLuint waterTex;
	GLuint moonTex;
	GLuint planetTex;
	GLuint asteroidTex;
	Frustum frameFrustum;

	HeightMap* heightMap;
	GLuint bumpmap;
	GLuint cubeMap;
	Light* light;

	Mesh* troopMesh;
	MeshAnimation* troopAnim;
	MeshMaterial* troopMaterial;
	vector<GLuint> matTextures;
	int currentFrame;
	float frameTime;
	float totalTime;

	float waterCycle;
	float waterRotate;

	float angle;

	float troopSpeed;

	bool inSpace;
	bool freeCam;
	int freeCamChoice;

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
};
