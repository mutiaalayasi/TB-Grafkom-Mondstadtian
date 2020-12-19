#include "Demo.h"



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	BuildShaders();
	BuildDepthMap();
	BuildRuang();
	BuildKasur();
	BuildLemari();
	BuildRak();
	BuildTexturedPlane();
	InitCamera();

}

void Demo::DeInit() {
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	//ruang
	glDeleteVertexArrays(1, &RuangVAO);
	glDeleteBuffers(1, &RuangVBO);
	glDeleteBuffers(1, &RuangEBO);
	//kasur
	glDeleteVertexArrays(1, &KasurVAO);
	glDeleteBuffers(1, &KasurVBO);
	glDeleteBuffers(1, &KasurEBO);

	//lantai
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &planeEBO);

	glDeleteBuffers(1, &depthMapFBO);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	// zoom camera
	// -----------
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (fovy < 90) {
			fovy += 0.0001f;
		}
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (fovy > 0) {
			fovy -= 0.0001f;
		}
	}

	// update camera movement 
	// -------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		MoveCamera(CAMERA_SPEED);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		MoveCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		StrafeCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		StrafeCamera(CAMERA_SPEED);
	}

	// update camera rotation
	// ----------------------
	double mouseX, mouseY;
	double midX = screenWidth / 2;
	double midY = screenHeight / 2;
	float angleY = 0.0f;
	float angleZ = 0.0f;

	// Get mouse position
	glfwGetCursorPos(window, &mouseX, &mouseY);
	if ((mouseX == midX) && (mouseY == midY)) {
		return;
	}

	// Set mouse position
	glfwSetCursorPos(window, midX, midY);

	// Get the direction from the mouse cursor, set a resonable maneuvering speed
	angleY = (float)((midX - mouseX)) / 1000;
	angleZ = (float)((midY - mouseY)) / 1000;

	// The higher the value is the faster the camera looks around.
	viewCamY += angleZ * 2;

	// limit the rotation around the x-axis
	if ((viewCamY - posCamY) > 8) {
		viewCamY = posCamY + 8;
	}
	if ((viewCamY - posCamY) < -8) {
		viewCamY = posCamY - 8;
	}
	RotateCamera(-angleY);
}
void Demo::InitCamera()
{
	posCamX = 0.0f;
	posCamY = 25.0f;
	posCamZ = 8.0f;
	viewCamX = 0.0f;
	viewCamY = 1.0f;
	viewCamZ = 0.0f;
	upCamX = 0.0f;
	upCamY = 1.0f;
	upCamZ = 0.0f;
	CAMERA_SPEED = 0.01f;
	fovy = 45.0f;
	glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Demo::MoveCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	// forward positive cameraspeed and backward negative -cameraspeed.
	posCamX = posCamX + x * speed;
	posCamZ = posCamZ + z * speed;
	viewCamX = viewCamX + x * speed;
	viewCamZ = viewCamZ + z * speed;
}

void Demo::StrafeCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	float orthoX = -z;
	float orthoZ = x;

	// left positive cameraspeed and right negative -cameraspeed.
	posCamX = posCamX + orthoX * speed;
	posCamZ = posCamZ + orthoZ * speed;
	viewCamX = viewCamX + orthoX * speed;
	viewCamZ = viewCamZ + orthoZ * speed;
}

void Demo::RotateCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	viewCamZ = (float)(posCamZ + glm::sin(speed) * x + glm::cos(speed) * z);
	viewCamX = (float)(posCamX + glm::cos(speed) * x - glm::sin(speed) * z);
}

void Demo::Update(double deltaTime) {
}

void Demo::Render() {
	
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	
	// Step 1 Render depth of scene to texture
	// ----------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(glm::vec3(10.0f, 4.0f, 30.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	UseShader(this->depthmapShader);
	glUniformMatrix4fv(glGetUniformLocation(this->depthmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glViewport(0, 0, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawRuang(this->depthmapShader); //ini buat render objeknya, bikin dulu nama variablenya di Demo.h
	DrawKasur(this->depthmapShader);
	DrawLemari(this->depthmapShader);
	DrawRak(this->depthmapShader);

	DrawTexturedPlane(this->depthmapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// Step 2 Render scene normally using generated depth map
	// ------------------------------------------------------
	glViewport(0, 0, this->screenWidth, this->screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Pass perspective projection matrix
	UseShader(this->shadowmapShader);
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	// LookAt camera (position, target/direction, up)
	glm::vec3 cameraPos = glm::vec3(0, 100, 2);
	glm::vec3 cameraFront = glm::vec3(0, 0, 0);
	glm::mat4 view = glm::lookAt(glm::vec3(posCamX, posCamY, posCamZ), glm::vec3(viewCamX, viewCamY, viewCamZ), glm::vec3(upCamX, upCamY, upCamZ));
	GLint viewLoc = glGetUniformLocation(this->shadowmapShader, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	// Setting Light Attributes
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "lightPos"), -2.0f, 4.0f, -1.0f);

	// Configure Shaders
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "shadowMap"), 1);

	// Render floor
	glActiveTexture(GL_TEXTURE0); //ntar dibikin begini, masing2 objek punya render sendiri
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedPlane(this->shadowmapShader);
	
	// Render ruang
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Ruang_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawRuang(this->shadowmapShader);

	//render Kasur
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Kasur_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawKasur(this->shadowmapShader);

	// Render Lemari
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, LemariTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawLemari(this->shadowmapShader);

	// Render Rak
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, RakTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawRak(this->shadowmapShader);

	glDisable(GL_DEPTH_TEST);
}



void Demo::BuildRuang() //ini buat bikin ruang
{
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &Ruang_texture);
	glBindTexture(GL_TEXTURE_2D, Ruang_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("dinding.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = { 
		// format position, tex coords, normal
		// front
		 0.0,  0.0, 0.0, 0, 0, 0.0f,  0.0f,  1.0f, // 0
		96.0,  0.0, 0.0, 1, 0,  0.0f,  0.0f,  1.0f, // 1
		96.0, 64.0, 0.0, 1, 1,  0.0f,  0.0f,  1.0f, // 2
		 0.0, 64.0, 0.0, 0, 1, 0.0f,  0.0f,  1.0f, // 3

		// right
		96.0,  0.0,  0.0, 0, 0, 1.0f,  0.0f,  0.0f, // 4
		96.0,  0.0, 96.0, 1, 0, 1.0f,  0.0f,  0.0f, // 5
		96.0, 64.0, 96.0, 1, 1, 1.0f,  0.0f,  0.0f, // 6
		96.0, 64.0,  0.0, 0, 1, 1.0f,  0.0f,  0.0f, // 7

		// back
		 0.0,  0.0, 96.0, 0, 0, 0.0f,  0.0f,  -1.0f, // 8 
		96.0,  0.0, 96.0, 1, 0, 0.0f,  0.0f,  -1.0f, // 9
		96.0, 64.0, 96.0, 1, 1, 0.0f,  0.0f,  -1.0f, // 10
		 0.0, 64.0, 96.0, 0, 1, 0.0f,  0.0f,  -1.0f, // 11

		 // left
		 0.0,  0.0, 96.0, 0, 0, -1.0f,  0.0f,  0.0f, // 12
		 0.0,  0.0,  0.0, 1, 0, -1.0f,  0.0f,  0.0f, // 13
		 0.0, 64.0,  0.0, 1, 1, -1.0f,  0.0f,  0.0f, // 14
		 0.0, 64.0, 96.0, 0, 1, -1.0f,  0.0f,  0.0f, // 15

		//// upper
		 0.0, 64.0,  0.0, 0, 0,   0.0f,  1.0f,  0.0f, // 16
		96.0, 64.0,  0.0, 1, 0,   0.0f,  1.0f,  0.0f, // 17
		96.0, 64.0, 96.0, 1, 1,  0.0f,  1.0f,  0.0f, // 18
		 0.0, 64.0, 96.0, 0, 1,   0.0f,  1.0f,  0.0f, // 19
	};

	unsigned int indices[] = {
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // back
		8,  9,  10, 8,  10, 11,  // left
		12, 14, 13, 12, 15, 14,  // right
		16, 18, 17, 16, 19, 18,  // upper
	};

	glGenVertexArrays(1, &RuangVAO);
	glGenBuffers(1, &RuangVBO);
	glGenBuffers(1, &RuangEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(RuangVAO);

	glBindBuffer(GL_ARRAY_BUFFER, RuangVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RuangEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void Demo::DrawRuang(GLuint shader) //ini buat bikin texture
{
	UseShader(shader);
	glBindVertexArray(RuangVAO);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.5f, 0));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildKasur()
{
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &Kasur_texture);
	glBindTexture(GL_TEXTURE_2D, Kasur_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("yaaa.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords, normal
		// format position, tex coords, normal
		// front
		 0.0,  0.0, 80.0, 0, 0, 0.0f,  0.0f,  1.0f, // 0
		32.0,  0.0, 80.0, 1, 0,  0.0f,  0.0f,  1.0f, // 1
		32.0,  8.0, 80.0, 1, 1,  0.0f,  0.0f,  1.0f, // 2
		 0.0,  8.0, 80.0, 0, 1, 0.0f,  0.0f,  1.0f, // 3

		 // right
		32.0, 0.0, 96.0, 0, 0, 1.0f,  0.0f,  0.0f, // 4
		32.0, 0.0, 80.0, 1, 0, 1.0f,  0.0f,  0.0f, // 5
		32.0, 8.0, 80.0, 1, 1, 1.0f,  0.0f,  0.0f, // 6
		32.0, 8.0, 96.0, 0, 1, 1.0f,  0.0f,  0.0f, // 7

		// back
		32.0, 0.0, 96.0, 0, 0, 0.0f,  0.0f,  -1.0f, // 8 
		 0.0, 0.0, 96.0, 1, 0, 0.0f,  0.0f,  -1.0f, // 9
		 0.0, 8.0, 96.0, 1, 1, 0.0f,  0.0f,  -1.0f, // 10
		32.0, 8.0, 96.0, 0, 1, 0.0f,  0.0f,  -1.0f, // 11

		 // left
		 0.0, 0.0, 96.0, 0, 0, -1.0f,  0.0f,  0.0f, // 12
		 0.0, 0.0, 80.0, 1, 0, -1.0f,  0.0f,  0.0f, // 13
		 0.0, 8.0, 80.0, 1, 1, -1.0f,  0.0f,  0.0f, // 14
		 0.0, 8.0, 96.0, 0, 1, -1.0f,  0.0f,  0.0f, // 15

		// upper
		 0.0, 8.0, 96.0, 0, 0,   0.0f,  1.0f,  0.0f, // 16
		32.0, 8.0, 96.0, 1, 0,   0.0f,  1.0f,  0.0f, // 17
		32.0, 8.0, 80.0, 1, 1,  0.0f,  1.0f,  0.0f, // 18
		 0.0, 8.0, 80.0, 0, 1,   0.0f,  1.0f,  0.0f, // 19
	};

	unsigned int indices[] = {
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // right
		8,  9,  10, 8,  10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		//20, 22, 21, 20, 23, 22   // bottom
	};

	glGenVertexArrays(1, &KasurVAO);
	glGenBuffers(1, &KasurVBO);
	glGenBuffers(1, &KasurEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(KasurVAO);

	glBindBuffer(GL_ARRAY_BUFFER, KasurVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, KasurEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void Demo::DrawKasur(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(KasurVAO);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.5f, 0));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildLemari()
{
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &LemariTexture);
	glBindTexture(GL_TEXTURE_2D, LemariTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("Lemari.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords, normal
		// right
		0.0,   0.0,  8.0, 0, 0, 0.0f,  0.0f,  1.0f, // 0
		0.0,   0.0,  0.0, 1, 0, 0.0f,  0.0f,  1.0f, // 1
		0.0,   16.0,  0.0, 1, 1,  0.0f,  0.0f,  1.0f, // 2
		0.0,   16.0,  8.0, 0, 1, 0.0f,  0.0f,  1.0f, // 3

		 // front
		 0.0,   0.0,  0.0, 0, 0, 1.0f,  0.0f,  0.0f, // 4
		 16.0,  0.0,  0.0, 1, 0, 1.0f,  0.0f,  0.0f, // 5
		 16.0,  16.0,  0.0, 1, 1, 1.0f,  0.0f,  0.0f, // 6
		 0.0,   16.0,  0.0, 0, 1, 1.0f,  0.0f,  0.0f, // 7

		// left
		16.0,   0.0, 8.0, 0, 0, 0.0f,  0.0f,  -1.0f, // 8 
		16.0,   0.0, 0.0, 1, 0, 0.0f,  0.0f,  -1.0f, // 9
		16.0,   16.0, 0.0, 1, 1, 0.0f,  0.0f,  -1.0f, // 10
		16.0,   16.0, 8.0, 0, 1, 0.0f,  0.0f,  -1.0f, // 11

		 // back
		 16.0,   0.0, 8.0, 0, 0, -1.0f,  0.0f,  0.0f, // 12
		 0.0,    0.0, 8.0, 1, 0, -1.0f,  0.0f,  0.0f, // 13
		 0.0,    16.0, 8.0, 1, 1, -1.0f,  0.0f,  0.0f, // 14
		 16.0,   16.0, 8.0, 0, 1, -1.0f,  0.0f,  0.0f, // 15

		// upper
		0.0,   16.0, 8.0, 0, 0,   0.0f,  1.0f,  0.0f, // 16
		0.0,   16.0, 0.0, 1, 0,   0.0f,  1.0f,  0.0f, // 17
		16.0,  16.0, 0.0, 1, 1,  0.0f,  1.0f,  0.0f, // 18
		16.0,  16.0, 8.0, 0, 1,   0.0f,  1.0f,  0.0f, // 19

	};

	unsigned int indices[] = {
		0,  1,  2,  0,  2,  3,   // right
		4,  5,  6,  4,  6,  7,   // front
		8,  9,  10, 8,  10, 11,  // left
		12, 14, 13, 12, 15, 14,  // back
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22   // bottom
	};

	glGenVertexArrays(1, &LemariVAO);
	glGenBuffers(1, &LemariVBO);
	glGenBuffers(1, &LemariEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(LemariVAO);

	glBindBuffer(GL_ARRAY_BUFFER, LemariVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LemariEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void Demo::DrawLemari(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(LemariVAO);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.5f, 0));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildRak()
{
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &RakTexture);
	glBindTexture(GL_TEXTURE_2D, RakTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("Rak.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords, normal
		// right
		0.0,   0.0,  80.0, 0, 0, 0.0f,  0.0f,  1.0f, // 0
		0.0,   0.0,  70.0, 1, 0, 0.0f,  0.0f,  1.0f, // 1
		0.0,   8.0,  70.0, 1, 1,  0.0f,  0.0f,  1.0f, // 2
		0.0,   8.0,  80.0, 0, 1, 0.0f,  0.0f,  1.0f, // 3

		 // front
		 0.0,   0.0,  70.0, 0, 0, 1.0f,  0.0f,  0.0f, // 4
		 8.0,  0.0,  70.0, 1, 0, 1.0f,  0.0f,  0.0f, // 5
		 8.0,  8.0,  70.0, 1, 1, 1.0f,  0.0f,  0.0f, // 6
		 0.0,   8.0,  70.0, 0, 1, 1.0f,  0.0f,  0.0f, // 7

		// left
		8.0,   0.0, 80.0, 0, 0, 0.0f,  0.0f,  -1.0f, // 8 
		8.0,   0.0, 70.0, 1, 0, 0.0f,  0.0f,  -1.0f, // 9
		8.0,   8.0, 70.0, 1, 1, 0.0f,  0.0f,  -1.0f, // 10
		8.0,   8.0, 80.0, 0, 1, 0.0f,  0.0f,  -1.0f, // 11

		 // back
		 8.0,   0.0, 80.0, 0, 0, -1.0f,  0.0f,  0.0f, // 12
		 0.0,    0.0, 80.0, 1, 0, -1.0f,  0.0f,  0.0f, // 13
		 0.0,    8.0, 80.0, 1, 1, -1.0f,  0.0f,  0.0f, // 14
		 8.0,   8.0, 80.0, 0, 1, -1.0f,  0.0f,  0.0f, // 15

		// upper
		0.0,   8.0, 80.0, 0, 0,   0.0f,  1.0f,  0.0f, // 16
		0.0,   8.0, 70.0, 1, 0,   0.0f,  1.0f,  0.0f, // 17
		8.0,  8.0, 70.0, 1, 1,  0.0f,  1.0f,  0.0f, // 18
		8.0,  8.0, 80.0, 0, 1,   0.0f,  1.0f,  0.0f, // 19

	};

	unsigned int indices[] = {
		0,  1,  2,  0,  2,  3,   // right
		4,  5,  6,  4,  6,  7,   // front
		8,  9,  10, 8,  10, 11,  // left
		12, 14, 13, 12, 15, 14,  // back
		16, 18, 17, 16, 19, 18,  // upper
	};

	glGenVertexArrays(1, &RakVAO);
	glGenBuffers(1, &RakVBO);
	glGenBuffers(1, &RakEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(RakVAO);

	glBindBuffer(GL_ARRAY_BUFFER, RakVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RakEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void Demo::DrawRak(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(RakVAO);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.5f, 0));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedPlane(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(planeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glm::mat4 model;
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}
void Demo::BuildTexturedPlane()
{
	// Load and create a texture 
	glGenTextures(1, &plane_texture);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("lantai.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-100.0, 0.0, -100.0, 0, 0, 0.0f,  -1.0f,  0.0f,
		268.0, 0.0, -100.0, 1, 0,  0.0f,  -1.0f,  0.0f,
		268.0, 0.0, 180.0, 1, 1,  0.0f,  -1.0f,  0.0f,
		-100.0, 0.0,  180, 0, 1, 0.0f,  -1.0f,  0.0f,
	};

	GLuint indices[] = { 0,  2,  1,  0,  3,  2 };

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glGenBuffers(1, &planeEBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO
}

void Demo::BuildDepthMap() {
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->SHADOW_WIDTH, this->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Demo::BuildShaders()
{
	// build and compile our shader program
	// ------------------------------------
	shadowmapShader = BuildShader("shadowMapping.vert", "shadowMapping.frag", nullptr);
	depthmapShader = BuildShader("depthMap.vert", "depthMap.frag", nullptr);
}


int main(int argc, char** argv) {
	RenderEngine &app = Demo();
	app.Start("Shadow Mapping Demo", 800, 600, false, false);
}