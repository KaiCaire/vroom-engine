#include "Application.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"
#include "FileSystem.h"
#include "ResourceMesh.h"
#include "SceneManager.h"
#include "RenderMeshComponent.h"
#include "ResourceManager.h"
#include "Camera.h"
#include "OpenGL.h"
#include "Octree.h"
#include "GUIManager.h"
#include "Input.h"
#include "CameraComponent.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//forward declaration
AABB GetGameObjectAABB(const std::shared_ptr<GameObject>& obj);

Render::Render() : Module()
{
	name = "render";
	
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render()
{
}

// Called before render is available
bool Render::Awake()
{
	LOG("Create SDL rendering context");
	bool ret = true;

	int scale = Application::GetInstance().window->GetScale();
	SDL_Window* window = Application::GetInstance().window->window;

	// SDL3: no flags; create default renderer and set vsync separately
	renderer = SDL_CreateRenderer(window, nullptr);

	if (renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		if (vsync)
		{
			if (!SDL_SetRenderVSync(renderer, 1))
			{
				LOG("Warning: could not enable vsync: %s", SDL_GetError());
			}
			else
			{
				LOG("Using vsync");
			}
		}

		camera.w = Application::GetInstance().window->width * scale;
		camera.h = Application::GetInstance().window->height * scale;
		camera.x = 0;
		camera.y = 0;
	}

	return ret;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// back background
	if (!SDL_GetRenderViewport(renderer, &viewport))
	{
		LOG("SDL_GetRenderViewport failed: %s", SDL_GetError());
	}
	InitSceneFBO(Application::GetInstance().window->width, Application::GetInstance().window->height);
	InitGameFBO(Application::GetInstance().window->width, Application::GetInstance().window->height);

	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);
	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	SDL_RenderClear(renderer);

	return true;
}

bool Render::Update(float dt)
{
	
	
	return true;
}

bool Render::PostUpdate()
{
	//scene rendering to fbo
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	glViewport(0, 0, Application::GetInstance().window->width, Application::GetInstance().window->height);
	glClearColor(background.r / 255.0f, background.g / 255.0f, background.b / 255.0f, 1.0f);
	//glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render scene content
	Shader* renderShader = Application::GetInstance().openGL.get()->GetActiveShader();
	if (renderShader) RenderFrame(*renderShader);

	if (Application::GetInstance().openGL.get()->drawZbuffer) {
		Shader* depthBufferShader = Application::GetInstance().openGL.get()->depthBufferShader;
		glDepthFunc(GL_ALWAYS);
		depthBufferShader->Use();
		depthBufferShader->setFloat("near", Application::GetInstance().camera->nearPlane);
		depthBufferShader->setFloat("far", Application::GetInstance().camera->farPlane / 5);
		RenderFrame(*depthBufferShader);
		glDepthFunc(GL_LESS);
	}

	//draw game view
	if (renderShader) {
		DrawGameView(*renderShader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Application::GetInstance().window->width, Application::GetInstance().window->height);

	//clear screen
	glClear(GL_COLOR_BUFFER_BIT);

	//imgui rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(Application::GetInstance().window->window);
	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	if (sceneFBO) glDeleteFramebuffers(1, &sceneFBO);
	if (sceneTextureID) glDeleteTextures(1, &sceneTextureID);
	if (sceneRBO) glDeleteRenderbuffers(1, &sceneRBO);

	if (gameFBO) glDeleteFramebuffers(1, &gameFBO);
	if (gameTextureID) glDeleteTextures(1, &gameTextureID);
	if (gameRBO) glDeleteRenderbuffers(1, &gameRBO);
	
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::InitSceneFBO(int w, int h) {
	//create fbo
	if (sceneFBO) glDeleteFramebuffers(1, &sceneFBO);
	glGenFramebuffers(1, &sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

	//create texture attatchment
	if (sceneTextureID) glDeleteTextures(1, &sceneTextureID);
	glGenTextures(1, &sceneTextureID);
	glBindTexture(GL_TEXTURE_2D, sceneTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTextureID, 0);

	//renderbuffer
	if (sceneRBO) glDeleteRenderbuffers(1, &sceneRBO);
	glGenRenderbuffers(1, &sceneRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, sceneRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneRBO);

	//unbind
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG("ERROR: Scene Framebuffer is not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	LOG("Scene FBO initialized/resized to %dx%d.", w, h);
}

void Render::InitGameFBO(int w, int h) {
	//create fbo
	if (gameFBO) glDeleteFramebuffers(1, &gameFBO);
	glGenFramebuffers(1, &gameFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gameFBO);

	//create texture attatchment
	if (gameTextureID) glDeleteTextures(1, &gameTextureID);
	glGenTextures(1, &gameTextureID);
	glBindTexture(GL_TEXTURE_2D, gameTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gameTextureID, 0);

	//renderbuffer
	if (gameRBO) glDeleteRenderbuffers(1, &gameRBO);
	glGenRenderbuffers(1, &gameRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, gameRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gameRBO);

	//safety check
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG("ERROR: Game Framebuffer is not complete!");
	}

	//unbind
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	LOG("Game FBO initialized/resized to %dx%d.", w, h);
}

void Render::DrawGameView(Shader& shader) {
	auto scene = Application::GetInstance().sceneManager->GetActiveScene();
	if (!scene) return;

	std::shared_ptr<CameraComponent> mainCam = nullptr;

	//find the camera component
	for (auto& go : scene->GetAllGameObjects()) {
		if (!go) continue;
		auto cam = std::dynamic_pointer_cast<CameraComponent>(go->GetComponent(ComponentType::CAMERA));
		if (cam && cam->isPrimary) {
			mainCam = cam;
			break;
		}
	}

	if (mainCam) {
		glBindFramebuffer(GL_FRAMEBUFFER, gameFBO);
		glViewport(0, 0, gameWidth, gameHeight);

		//clear
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//update shader matrices
		shader.Use();

		float aspect = gameWidth / gameHeight;

		shader.setMat4("view", mainCam->GetViewMatrix());
		shader.setMat4("projection", mainCam->GetProjectionMatrix(aspect));

		DrawActiveScene(shader);

		//unbind
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}



void Render::DrawGrid() {

	Shader* gridShader = Application::GetInstance().openGL.get()->texCoordsShader;
	gridShader->Use();

	auto camera = Application::GetInstance().camera.get();
	gridShader->setMat4("view", camera->viewMat);
	gridShader->setMat4("projection", camera->projectionMat);

	glm::mat4 identityMat = glm::mat4(1.0f);
	gridShader->setMat4("model", identityMat);

	// Enable line color mode
	glUniform1i(glGetUniformLocation(gridShader->ID, "useLineColor"), true);
	glUniform4f(glGetUniformLocation(gridShader->ID, "lineColor"), 1.0f, 1.0f, 1.0f, 0.5f);

	float lineX = -100.0f;
	float lineZ = -100.0f;

	float lineLength = 100.0f;
	for (int i = 0; i < 1000; i++) {
		
		glLineWidth(1.0f);
		
		glBegin(GL_LINES);

		//X AXIS LINES
		glVertex3f(-lineLength, 0.0f, lineZ);
		glVertex3f(lineLength, 0.0f, lineZ);

		//Z AXIS LINES
		glVertex3f(lineX, 0.f, -lineLength);
		glVertex3f(lineX, 0.f, lineLength);

		glEnd();
		

		lineX++;
		lineZ++;
	}

	//glClearColor;

	// Restore texture mode
	glUniform1i(glGetUniformLocation(gridShader->ID, "useLineColor"), false);
}

void Render::DrawAABB(const AABB& bounds, const glm::vec4& color) {
	Shader* debugShader = Application::GetInstance().openGL.get()->texCoordsShader;
	if (!debugShader) return;

	debugShader->Use();

	//set matrices
	auto camera = Application::GetInstance().camera.get();
	debugShader->setMat4("view", camera->viewMat);
	debugShader->setMat4("projection", camera->projectionMat);

	glm::mat4 identityMat = glm::mat4(1.0f);
	debugShader->setMat4("model", identityMat); 

	//set color uniform
	glUniform1i(glGetUniformLocation(debugShader->ID, "useLineColor"), true);
	glUniform4f(glGetUniformLocation(debugShader->ID, "lineColor"), color.r, color.g, color.b, color.a);

	glLineWidth(2.0f); //make AABB lines thicker

	glm::vec3 min = bounds.min;
	glm::vec3 max = bounds.max;

	glBegin(GL_LINES);

	//bottom Face
	glVertex3f(min.x, min.y, min.z); glVertex3f(max.x, min.y, min.z);
	glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, min.y, max.z); glVertex3f(min.x, min.y, max.z);
	glVertex3f(min.x, min.y, max.z); glVertex3f(min.x, min.y, min.z);

	//top Face
	glVertex3f(min.x, max.y, min.z); glVertex3f(max.x, max.y, min.z);
	glVertex3f(max.x, max.y, min.z); glVertex3f(max.x, max.y, max.z);
	glVertex3f(max.x, max.y, max.z); glVertex3f(min.x, max.y, max.z);
	glVertex3f(min.x, max.y, max.z); glVertex3f(min.x, max.y, min.z);

	//vertical Edges
	glVertex3f(min.x, min.y, min.z); glVertex3f(min.x, max.y, min.z);
	glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, max.y, min.z);
	glVertex3f(max.x, min.y, max.z); glVertex3f(max.x, max.y, max.z);
	glVertex3f(min.x, min.y, max.z); glVertex3f(min.x, max.y, max.z);

	glEnd();

	//restore state
	glUniform1i(glGetUniformLocation(debugShader->ID, "useLineColor"), false);
	glLineWidth(1.0f);
}

void Render::DrawRay(const glm::vec3& origin, const glm::vec3& direction, const glm::vec4& color) {
	Shader* debugShader = Application::GetInstance().openGL.get()->texCoordsShader;
	if (!debugShader) return;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	debugShader->Use();

	//set matrices
	auto camera = Application::GetInstance().camera.get();
	debugShader->setMat4("view", camera->viewMat);
	debugShader->setMat4("projection", camera->projectionMat);
	glm::mat4 identityMat = glm::mat4(1.0f);
	debugShader->setMat4("model", identityMat);

	//set color
	glUniform1i(glGetUniformLocation(debugShader->ID, "useLineColor"), true);
	glUniform4f(glGetUniformLocation(debugShader->ID, "lineColor"), color.r, color.g, color.b, color.a);

	glLineWidth(3.0f); //make ray thicker

	//start ray 0.1 units away so its more visible
	glm::vec3 startPoint = origin + direction * 0.1f;

	//draw ray 5000 units long
	glm::vec3 endPoint = origin + direction * 5000.0f;

	glBegin(GL_LINES);
	glVertex3f(origin.x, origin.y, origin.z);
	glVertex3f(endPoint.x, endPoint.y, endPoint.z);
	glEnd();

	//restore state
	glUniform1i(glGetUniformLocation(debugShader->ID, "useLineColor"), false);
	glLineWidth(1.0f);

	glEnable(GL_DEPTH_TEST);
}

void Render::UpdateShaderMatrices(Shader& shader) {
	auto camera = Application::GetInstance().camera.get();

	shader.Use();
	//All objects share the SAME camera(view + projection)
	shader.setMat4("view", camera->viewMat); 
	shader.setMat4("projection", camera->projectionMat);
	// Model matrix is set per object in DrawActiveScene


}

void Render::RenderFrame(Shader& shader) {
	// Setup shader matrices
	UpdateShaderMatrices(shader);

	// Draw grid
	DrawGrid();

	//draw raycast
	auto camera = Application::GetInstance().camera.get();
	auto input = Application::GetInstance().input.get();
	auto guiManager = Application::GetInstance().guiManager.get();

	if (guiManager->drawRaycast) {
		//calculate current mouse ray
		auto mousePos = input->GetMousePosition();
		glm::vec3 rayDir = input->MouseRay(mousePos.x, mousePos.y, camera->projectionMat, camera->viewMat);

		//draw the ray
		DrawRay(camera->GetWorldPosition(), rayDir, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	}

	shader.Use();

	// Draw scene
	DrawActiveScene(shader);
}

void Render::DrawActiveScene(Shader& shader) {
	auto sceneManager = Application::GetInstance().sceneManager.get();
	auto scene = sceneManager->GetActiveScene();
	auto guiManager = Application::GetInstance().guiManager.get();
	auto camera = Application::GetInstance().camera.get();

	if (!scene) {
		LOG("WARNING: No active scene to render");
		return;
	}

	std::vector<std::shared_ptr<GameObject>> visibleObjects;
	//get total object count in order to log rendered objects (to test frustum culling)
	int totalObjects = (int)scene->GetAllGameObjects().size();

	if (scene->GetOctree()) {
		// Query the Octree using the camera's pre-calculated frustum
		scene->GetOctree()->Query(camera->frustum, visibleObjects);
	}
	else {
		// Fallback: If no Octree, render all objects
		visibleObjects = scene->GetAllGameObjects();
		LOG("WARNING: Octree not active. Rendering all %d GameObjects.", (int)visibleObjects.size());
	}

	int drawnObjects = (int)visibleObjects.size();

	//log culling to check (commented to not flood the console)
	/*if (totalObjects > 1) {
		LOG("Culling Stats: Drawn/Total = %d / %d. Culled: %d", drawnObjects, totalObjects, totalObjects - drawnObjects);
	}*/

	// Iterate through all GameObjects in the scene
	for (auto& gameObject : visibleObjects) {
		if (!gameObject || !gameObject->IsActive() || gameObject->IsMarkedForDestroy()) {
			continue;
		}

		DrawGameObject(gameObject, shader);

		if (guiManager->drawAABBs) {
			AABB bounds = GetGameObjectAABB(gameObject);
			if (bounds.min != bounds.max) {
				DrawAABB(bounds, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			}
			shader.Use();
			//Uniforms are local to a specific shader program! If you're using the depth buffer shader, you must reset them!
		}
	}
}

void Render::DrawGameObject(std::shared_ptr<GameObject> go, Shader& shader) {
	//transform comp's get global transform must happen always!!
	auto transformComp = go->GetComponent(ComponentType::TRANSFORM);
	if (transformComp) {
		auto transform = std::dynamic_pointer_cast<TransformComponent>(transformComp);
		glm::mat4 modelMat = transform->GetGlobalTransform();
		shader.setMat4("model", modelMat);
	}

	auto rendererComp = go->GetComponent(ComponentType::MESH_RENDERER);
	if (!rendererComp) return;

	auto renderer = std::dynamic_pointer_cast<RenderMeshComponent>(rendererComp);
	if (!renderer) return;

	// 3. Draw the mesh
	renderer->Render(&shader);
}




















//UNUSED (from 2D Plarformer Template)


//void Render::SetViewPort(const SDL_Rect& rect)
//{
//	SDL_SetRenderViewport(renderer, &rect);
//}
//
//void Render::ResetViewPort()
//{
//	SDL_SetRenderViewport(renderer, &viewport);
//}
//
//// Blit to screen
//bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY) const
//{
//	bool ret = true;
//	int scale = Application::GetInstance().window->GetScale();
//
//	// SDL3 uses float rects for rendering
//	SDL_FRect rect;
//	rect.x = (float)((int)(camera.x * speed) + x * scale);
//	rect.y = (float)((int)(camera.y * speed) + y * scale);
//
//	if (section != NULL)
//	{
//		rect.w = (float)(section->w * scale);
//		rect.h = (float)(section->h * scale);
//	}
//	else
//	{
//		float tw = 0.0f, th = 0.0f;
//		if (!SDL_GetTextureSize(texture, &tw, &th))
//		{
//			LOG("SDL_GetTextureSize failed: %s", SDL_GetError());
//			return false;
//		}
//		rect.w = tw * scale;
//		rect.h = th * scale;
//	}
//
//	const SDL_FRect* src = NULL;
//	SDL_FRect srcRect;
//	if (section != NULL)
//	{
//		srcRect.x = (float)section->x;
//		srcRect.y = (float)section->y;
//		srcRect.w = (float)section->w;
//		srcRect.h = (float)section->h;
//		src = &srcRect;
//	}
//
//	SDL_FPoint* p = NULL;
//	SDL_FPoint pivot;
//	if (pivotX != INT_MAX && pivotY != INT_MAX)
//	{
//		pivot.x = (float)pivotX;
//		pivot.y = (float)pivotY;
//		p = &pivot;
//	}
//
//	// SDL3: returns bool; map to int-style check
//	int rc = SDL_RenderTextureRotated(renderer, texture, src, &rect, angle, p, SDL_FLIP_NONE) ? 0 : -1;
//	if (rc != 0)
//	{
//		LOG("Cannot blit to screen. SDL_RenderTextureRotated error: %s", SDL_GetError());
//		ret = false;
//	}
//
//	return ret;
//}
//
//bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
//{
//	bool ret = true;
//	int scale = Application::GetInstance().window->GetScale();
//
//	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
//	SDL_SetRenderDrawColor(renderer, r, g, b, a);
//
//	SDL_FRect rec;
//	if (use_camera)
//	{
//		rec.x = (float)((int)(camera.x + rect.x * scale));
//		rec.y = (float)((int)(camera.y + rect.y * scale));
//		rec.w = (float)(rect.w * scale);
//		rec.h = (float)(rect.h * scale);
//	}
//	else
//	{
//		rec.x = (float)(rect.x * scale);
//		rec.y = (float)(rect.y * scale);
//		rec.w = (float)(rect.w * scale);
//		rec.h = (float)(rect.h * scale);
//	}
//
//	int result = (filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderRect(renderer, &rec)) ? 0 : -1;
//
//	if (result != 0)
//	{
//		LOG("Cannot draw quad to screen. SDL_RenderFillRect/SDL_RenderRect error: %s", SDL_GetError());
//		ret = false;
//	}
//
//	return ret;
//}
//
//bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
//{
//	bool ret = true;
//	int scale = Application::GetInstance().window->GetScale();
//
//	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
//	SDL_SetRenderDrawColor(renderer, r, g, b, a);
//
//	float X1, Y1, X2, Y2;
//
//	if (use_camera)
//	{
//		X1 = (float)(camera.x + x1 * scale);
//		Y1 = (float)(camera.y + y1 * scale);
//		X2 = (float)(camera.x + x2 * scale);
//		Y2 = (float)(camera.y + y2 * scale);
//	}
//	else
//	{
//		X1 = (float)(x1 * scale);
//		Y1 = (float)(y1 * scale);
//		X2 = (float)(x2 * scale);
//		Y2 = (float)(y2 * scale);
//	}
//
//	int result = SDL_RenderLine(renderer, X1, Y1, X2, Y2) ? 0 : -1;
//
//	if (result != 0)
//	{
//		LOG("Cannot draw quad to screen. SDL_RenderLine error: %s", SDL_GetError());
//		ret = false;
//	}
//
//	return ret;
//}
//
//bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
//{
//	bool ret = true;
//	int scale = Application::GetInstance().window->GetScale();
//
//	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
//	SDL_SetRenderDrawColor(renderer, r, g, b, a);
//
//	int result = -1;
//	SDL_FPoint points[360];
//
//	float factor = (float)M_PI / 180.0f;
//
//	float cx = (float)((use_camera ? camera.x : 0) + x * scale);
//	float cy = (float)((use_camera ? camera.y : 0) + y * scale);
//
//	for (int i = 0; i < 360; ++i)
//	{
//		points[i].x = cx + (float)(radius * cos(i * factor));
//		points[i].y = cy + (float)(radius * sin(i * factor));
//	}
//
//	result = SDL_RenderPoints(renderer, points, 360) ? 0 : -1;
//
//	if (result != 0)
//	{
//		LOG("Cannot draw quad to screen. SDL_RenderPoints error: %s", SDL_GetError());
//		ret = false;
//	}
//
//	return ret;
//}

//void Render::AddModel(Model* model) {
//	modelsToDraw.push_back(model);
//}

//bool Render::DrawMesh(Mesh mesh, unsigned int shaderProgram, unsigned int VAO) const {
//
//	//glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // dark bluish background
//	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//
//	return true;
//}