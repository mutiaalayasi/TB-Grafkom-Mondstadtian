#pragma once
#include "RenderEngine.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <SOIL/SOIL.h>

class Demo :
	public RenderEngine
{
public:
	Demo();
	~Demo();
private:
	GLuint depthmapShader, shadowmapShader, stexture, stexture2, depthMapFBO, depthMap,
		RuangVBO, RuangVAO, RuangEBO, Ruang_texture,
		KasurVBO, KasurVAO, KasurEBO, Kasur_texture,
		planeVBO, planeVAO, planeEBO, plane_texture,
		LemariVBO, LemariVAO, LemariEBO, LemariTexture,
		RakVBO, RakVAO, RakEBO, RakTexture;
	float viewCamX, viewCamY, viewCamZ, upCamX, upCamY, upCamZ, posCamX, posCamY, posCamZ, CAMERA_SPEED, fovy;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	virtual void Init();
	virtual void DeInit();
	virtual void Update(double deltaTime);
	virtual void Render();
	virtual void ProcessInput(GLFWwindow *window);
	void BuildRuang();
	void DrawRuang(GLuint shader);
	void BuildKasur();
	void DrawKasur(GLuint shader);
	void BuildLemari();
	void DrawLemari(GLuint shader);
	void BuildRak();
	void DrawRak(GLuint shader);
	void BuildTexturedPlane();	
	void DrawTexturedPlane(GLuint shader);
	void BuildDepthMap();
	void BuildShaders();
	void MoveCamera(float speed);
	void StrafeCamera(float speed);
	void RotateCamera(float speed);
	void InitCamera();
};

