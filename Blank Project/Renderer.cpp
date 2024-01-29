#include "Renderer.h";
#include "../nclgl/Camera.h";
#include "../nclgl/HeightMap.h";
#include "../nclgl/Light.h";
#include "../nclgl/MeshAnimation.h";
#include "../nclgl/MeshMaterial.h";
#include "../nclgl/SceneNode.h";
#include <algorithm>;
#include <math.h>;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	asteroidMesh = Mesh::LoadFromMeshFile("asteroidMesh.fbx");

	heightMap = new HeightMap(TEXTUREDIR"noise.png");
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	planetTex = SOIL_load_OGL_texture(TEXTUREDIR"planet_tex.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	bumpmap = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg",
		TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	moonTex = SOIL_load_OGL_texture(TEXTUREDIR"moonTex.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	asteroidTex = SOIL_load_OGL_texture(TEXTUREDIR"asteroidTex.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	shadowSceneShader = new Shader("ShadowSceneVert.glsl", "ShadowSceneFrag.glsl");
	sceneShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	bumpShader = new Shader("BumpVertex.glsl", "BumpFragment.glsl");
	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	reflectShader = new Shader("ReflectVertex.glsl", "ReflectFragment.glsl");
	skinningShader = new Shader("SkinningVertex.glsl", "texturedFragment.glsl");

	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	if (!sceneShader->LoadSuccess() || !bumpShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !reflectShader->LoadSuccess() || !skinningShader->LoadSuccess()) {
		return;
	}
	if (!bumpmap || !cubeMap || !heightMap) {
		return;
	}
	if (!earthTex || !waterTex || !moonTex) {
		return;
	}
	Vector3 hSize = heightMap->GetHeightMapSize();
	light = new Light(hSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 1), hSize.x * 0.5f);
	camera = new Camera(0.0f, 200.0f, hSize * Vector3(0.5f, 1.7f, 0.35f));


	troopMesh = Mesh::LoadFromMeshFile("Role_T.msh");
	troopAnim = new MeshAnimation("Role_T.anm");
	troopMaterial = new MeshMaterial("Role_T.mat");
	for (int i = 0; i < troopMesh->GetSubMeshCount(); i++) {
		const MeshMaterialEntry* matEntry = troopMaterial->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
	}
	currentFrame = 0;
	frameTime = 0.0f;
	totalTime = 0.0f;

	root = new SceneNode();

	heightMapNode = new SceneNode(heightMap);

	waterNode = new SceneNode(quad);

	troopNode = new SceneNode(troopMesh);
	troopNode->SetTransform(Matrix4::Translation(Vector3(hSize.x *0.55,hSize.y*0.9,hSize.z * 0.5f)));
	troopNode->SetModelScale(Vector3(50, 50, 50));

	moonSpinner = new SceneNode(sphere);
	moonSpinner->SetTransform(Matrix4::Translation(Vector3((hSize.x)*0.5f, hSize.y, hSize.z*0.5f)));
	moonSpinner->SetModelScale(Vector3(500, 500, 500));

	moonNode = new SceneNode(sphere);
	moonNode->SetTransform(Matrix4::Translation(Vector3(8000.0f, 1000.0f, 0.0f)));
	moonNode->SetModelScale(Vector3(300, 300, 300));

	planet = new SceneNode(sphere);
	planet->SetTransform(Matrix4::Translation(Vector3(hSize)));
	planet->SetModelScale(Vector3(800, 800, 800));
	asteroid = new SceneNode(asteroidMesh);
	moonNode->SetTransform(Matrix4::Translation(Vector3(2500.0f, 500.0f, 0.0f)));
	moonNode->SetModelScale(Vector3(50, 50, 50));
	asteroidSpinner = new SceneNode(sphere);
	asteroidSpinner->SetTransform(Matrix4::Translation(Vector3(hSize)));
	asteroidSpinner->SetModelScale(Vector3(500, 500, 500));

	root->AddChild(heightMapNode);
	root->AddChild(waterNode);
	root->AddChild(troopNode);
	root->AddChild(moonSpinner);
	root->AddChild(planet);
	root->AddChild(asteroidSpinner);
	asteroidSpinner->AddChild(asteroid);
	moonSpinner->AddChild(moonNode);

	DrawSkybox();

	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(bumpmap, true);
	SetTextureRepeating(moonTex, true);
	SetTextureRepeating(asteroidTex, true);

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterCycle = 0.0f;
	waterRotate = 0.0f;

	angle = 0.0f;
	troopSpeed = 2.0f;

	freeCamChoice = 0;

	inSpace = false;
	freeCam = false;

	init = true;
}

Renderer::~Renderer(void) {
	delete root;
	delete quad;
	delete camera;
	delete cube;
	delete sceneShader;
	delete bumpShader;
	delete skyboxShader;
	delete reflectShader;
	delete skinningShader;
	delete shadowSceneShader;
	delete light;
	delete heightMap;
	delete troopMesh;
	delete troopAnim;
	delete troopMaterial;
	glDeleteTextures(1, &earthTex);
	glDeleteTextures(1, &waterTex);
	glDeleteTextures(1, &bumpmap);
	glDeleteTextures(1, &moonTex);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	angle = dt * 10.0f;
	if (angle > 360.0f) {
		angle = 0.0f;
	}
	waterRotate += dt * 0.5f;
	waterCycle += dt * 0.25f;

	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % troopAnim->GetFrameCount();
		frameTime += 1.0f / troopAnim->GetFrameRate();
		
	}
	
	totalTime += dt;
	root->Update(dt);
}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from) && from != heightMapNode && from != waterNode) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));
		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}
	else {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));
		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); i++) {
		BuildNodeLists((*i));
	}
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::DrawNodes() {
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		if (n == heightMapNode && inSpace == false) {
			DrawHeightMap(n);
		}
		else if (n == waterNode && inSpace == false) {
			DrawWater(n);
		}
		else if (n->GetMesh() == troopMesh && inSpace == false) {
			DrawTroops(n);
		}
		else if (n == moonNode) {
			DrawMoon(n);
		}
		else if (n == moonSpinner) {
			MoonSpin(n);
		}
		else if (n == planet && inSpace == true) {
			DrawPlanet(n);
		}
		else if (n == asteroidSpinner && inSpace == true) {
			AsteroidSpin(n);
		}
		else if (n == asteroid && inSpace == true) {
			DrawAsteroid(n);
		}
	}
}

void Renderer::DrawHeightMap(SceneNode* n) {
	BindShader(bumpShader);

	glUniform1i(glGetUniformLocation(bumpShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(bumpShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bumpmap);

	glUniform3fv(glGetUniformLocation(bumpShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	UpdateShaderMatrices();
	SetShaderLight(*light);
	n->Draw(*this);
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawWater(SceneNode* n) {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightMapSize();
	modelMatrix = Matrix4::Translation(Vector3(hSize.x * 0.5f,hSize.y * 0.42f,hSize.z * 0.5f)) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	n->Draw(*this);
	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();
}

void Renderer::DrawTroops(SceneNode* n) {
	BindShader(skinningShader);
	glUniform3fv(glGetUniformLocation(skinningShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(skinningShader->GetProgram(), "diffuseTex"), 0);

	Vector3 hSize = heightMap->GetHeightMapSize();
	if (n->GetTransform().GetPositionVector().z == (hSize.z * 0.5f) + 300.0f) {
		n->SetTransform(n->GetTransform() * Matrix4::Rotation(180, Vector3(0, 1, 0)));
	}
	else if(n->GetTransform().GetPositionVector().z == (hSize.z * 0.5f) - 100.0f){
		n->SetTransform(n->GetTransform() * Matrix4::Rotation(180, Vector3(0, 1, 0)));
	}
	n->SetTransform(n->GetTransform() * Matrix4::Translation(Vector3(0.0f, 0.0f, troopSpeed)));
	Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(skinningShader->GetProgram(), "modelMatrix"), 1, false, model.values);
	glUniform4fv(glGetUniformLocation(skinningShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
	modelMatrix = model;

	UpdateShaderMatrices();
	vector<Matrix4> frameMatrices;

	const Matrix4* invBindPose = n->GetMesh()->GetInverseBindPose();
	const Matrix4* frameData = troopAnim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < n->GetMesh()->GetJointCount(); i++) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(skinningShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < n->GetMesh()->GetSubMeshCount(); i++) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);
		n->GetMesh()->DrawSubMesh(i);
	}
	modelMatrix.ToIdentity();
}

void Renderer::MoonSpin(SceneNode* n) {
	BindShader(reflectShader);

	n->SetTransform(n->GetTransform() * Matrix4::Rotation(angle, Vector3(0, 1, 0)));
	Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(reflectShader->GetProgram(), "modelMatrix"), 1, false, model.values);
	glUniform4fv(glGetUniformLocation(reflectShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
	modelMatrix = model;
	UpdateShaderMatrices();
	modelMatrix.ToIdentity();
}

void Renderer::DrawMoon(SceneNode* n) {
	BindShader(reflectShader);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, moonTex);

	n->SetTransform(n->GetTransform() * Matrix4::Rotation(angle, Vector3(0, 1, 0)));
	Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(reflectShader->GetProgram(), "modelMatrix"), 1, false, model.values);
	glUniform4fv(glGetUniformLocation(reflectShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

	modelMatrix = model;
	UpdateShaderMatrices();
	n->Draw(*this);
	modelMatrix.ToIdentity();
}

void Renderer::DrawPlanet(SceneNode* n) {
	BindShader(sceneShader);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);

	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planetTex);

	n->SetTransform(n->GetTransform() * Matrix4::Rotation(-angle, Vector3(0, 1, 0)));
	Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(sceneShader->GetProgram(), "modelMatrix"), 1, false, model.values);
	glUniform4fv(glGetUniformLocation(sceneShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

	modelMatrix = model;
	UpdateShaderMatrices();
	n->Draw(*this);
	modelMatrix.ToIdentity();
}

void Renderer::AsteroidSpin(SceneNode* n) {
	BindShader(sceneShader);

	n->SetTransform(n->GetTransform() * Matrix4::Rotation(angle, Vector3(1, 0, 0)));
	Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(sceneShader->GetProgram(), "modelMatrix"), 1, false, model.values);
	glUniform4fv(glGetUniformLocation(sceneShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
	modelMatrix = model;
	UpdateShaderMatrices();
	modelMatrix.ToIdentity();
}

void Renderer::DrawAsteroid(SceneNode* n) {
	BindShader(sceneShader);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);

	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, asteroidTex);

	n->SetTransform(n->GetTransform() * Matrix4::Rotation(-angle, Vector3(0, 1, 0)));
	Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(sceneShader->GetProgram(), "modelMatrix"), 1, false, model.values);
	glUniform4fv(glGetUniformLocation(sceneShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

	modelMatrix = model;
	UpdateShaderMatrices();
	n->Draw(*this);
	modelMatrix.ToIdentity();
}

void Renderer::CameraMovement() {
	Vector3 hSize = heightMap->GetHeightMapSize();
	if (camera->GetPosition().z == (hSize.z * 0.35f) + 500.0f) {
		freeCamChoice = 1;
		camera->setPosition(hSize * Vector3(0.1f, 1.7f, 0.5f));
	}
	else if (camera->GetPosition().x == (hSize.x * 0.1f) + 800.0f) {
		freeCamChoice = 2;
		camera->setPosition(hSize * Vector3(0.11f, 1.7f, 0.1f));
	}
	else if (camera->GetPosition().x == (hSize.x * 0.11f) + 800.0f && (camera->GetPosition().z == (hSize.x * 0.1f) + 800.0f)){
		freeCamChoice = 0;
		camera->setPosition(hSize * Vector3(0.5f, 1.7f, 0.35f));
	}
	if (!freeCam) {
		switch (freeCamChoice) {
		case 0:
			camera->SetYaw(180.0f);
			camera->setPosition(Vector3(hSize.x * 0.5f, hSize.y * 1.7f, camera->GetPosition().z + troopSpeed));
			break;
		case 1:
			camera->SetYaw(270.0f);
			camera->setPosition(Vector3(camera->GetPosition().x + troopSpeed, hSize.y * 1.7f, hSize.z * 0.5f));
			break;
		case 2:
			camera->SetYaw(225.0f);
			camera->setPosition(Vector3(camera->GetPosition().x + troopSpeed, hSize.y * 1.7f, camera->GetPosition().z + troopSpeed));
			break;
		default:
			break;
		}
		
	}
	
}

void Renderer::ToggleFreeCam() {
	Vector3 hSize = heightMap->GetHeightMapSize();
	freeCam = !freeCam;
	freeCamChoice = 0;
	camera->setPosition(hSize * Vector3(0.5f, 1.7f, 0.35f));
}

void Renderer::SwitchToSpace() {
	glClear(GL_DEPTH_BUFFER_BIT);
	inSpace = true;
	Vector3 hSize = heightMap->GetHeightMapSize();
	camera->SetPitch(0.0f);
	camera->SetYaw(-125.0f);
	camera->setPosition(hSize * Vector3(0.25f, 1.7f, 0.5f));

	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"space_right.PNG", TEXTUREDIR"space_left.PNG", TEXTUREDIR"space_top.PNG",
		TEXTUREDIR"space_bottom.PNG", TEXTUREDIR"space_front.PNG", TEXTUREDIR"space_back.PNG", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	moonSpinner->SetTransform(Matrix4::Translation(Vector3(hSize.x, hSize.y, hSize.z)));
	moonNode->SetTransform(Matrix4::Translation(Vector3(1800.0f, 500.0f, 0.0f)));
	moonNode->SetModelScale(Vector3(100, 100, 100));

	totalTime = 0.0f;
}

void Renderer::SwitchToLand() {
	inSpace = false;
	glClear(GL_DEPTH_BUFFER_BIT);
	Vector3 hSize = heightMap->GetHeightMapSize();
	camera->SetPitch(0.0f);
	camera->SetYaw(200.0f);
	camera->setPosition(hSize * Vector3(0.5f, 1.7f, 0.5f));
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg",
		TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	moonSpinner->SetTransform(Matrix4::Translation(Vector3((hSize.x), hSize.y * 8, hSize.z)));
	moonNode->SetTransform(Matrix4::Translation(Vector3(8000.0f, 1000.0f, 0.0f)));
	moonNode->SetModelScale(Vector3(300, 300, 300));

	SetShaderLight(*light);

	totalTime = 0.0f;
}

void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	UpdateShaderMatrices();
	DrawSkybox();
	DrawNodes();

	if (!freeCam) {
		CameraMovement();
	}

	ClearNodeLists();
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}

