// #include <glad/glad.h>
// #include <GLFW/glfw3.h>

// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>

// #include <learnopengl/filesystem.h>
// #include <learnopengl/camera.h>
// #include <learnopengl/shader.h>
// #include <learnopengl/camera.h>
// #include <learnopengl/model.h>
// #include "../headers/Animation.h"
// #include "../headers/Bone.h"
// #include "../headers/Skeleton.h"
// #include "../headers/Pose.h"

// #include <iostream>

// #define ROOT_DIR std::filesystem::current_path().string()

// void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
// void processInput(GLFWwindow *window);

// // settings
// const unsigned int SCR_WIDTH = 800;
// const unsigned int SCR_HEIGHT = 600;

// // camera
// Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
// float lastX = SCR_WIDTH / 2.0f;
// float lastY = SCR_HEIGHT / 2.0f;
// bool firstMouse = true;

// // timing
// float deltaTime = 0.0f;
// float lastFrame = 0.0f;

// // lighting
// glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// int main()
// {
// 	// glfw: initialize and configure
// 	// ------------------------------
// 	glfwInit();
// 	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
// 	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
// 	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// #ifdef __APPLE__
// 	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
// #endif

// 	// glfw window creation
// 	// --------------------
// 	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
// 	if (window == NULL)
// 	{
// 		std::cout << "Failed to create GLFW window" << std::endl;
// 		glfwTerminate();
// 		return -1;
// 	}
// 	glfwMakeContextCurrent(window);
// 	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
// 	glfwSetCursorPosCallback(window, mouse_callback);
// 	glfwSetScrollCallback(window, scroll_callback);

// 	// tell GLFW to capture our mouse
// 	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

// 	// glad: load all OpenGL function pointers
// 	// ---------------------------------------
// 	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
// 	{
// 		std::cout << "Failed to initialize GLAD" << std::endl;
// 		return -1;
// 	}

// 	// configure global opengl state
// 	// -----------------------------
// 	glEnable(GL_DEPTH_TEST);

// 	// build and compile shaders
// 	// -------------------------
// 	Shader ourShader("/home/maxx/workspace/LearnOpenGL/shaders/basic_lighting.vs", "/home/maxx/workspace/LearnOpenGL/shaders/basic_lighting.fs");
// 	Shader lampShader("/home/maxx/workspace/LearnOpenGL/shaders/lamp.vs", "/home/maxx/workspace/LearnOpenGL/shaders/lamp.fs");

// 	// load models
// 	// -----------
// 	/*string s = std::filesystem::current_path().string().append("\\res\\monkey\\monkey.obj");
// 	Model ourModel(s);*/
//     Model sphere(FileSystem::getPath("resources/objects/sphere/sphere.obj"));
// 	Model cylinder(FileSystem::getPath("resources/objects/cylinder/cylinder.obj"));
// 	Model plane(FileSystem::getPath("resources/objects/plane/plane.obj"));
// 	Model monkey(FileSystem::getPath("resources/objects/monkey/monkey.obj"));

// 		// set up vertex data (and buffer(s)) and configure vertex attributes
// 	// ------------------------------------------------------------------
// 	float vertices[] = {
// 		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
// 		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,-
// 		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
// 		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
// 		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
// 		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

// 		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
// 		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
// 		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
// 		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
// 		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
// 		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

// 		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
// 		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
// 		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
// 		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
// 		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
// 		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

// 		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
// 		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
// 		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
// 		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
// 		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
// 		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

// 		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
// 		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
// 		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
// 		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
// 		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
// 		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

// 		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
// 		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
// 		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
// 		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
// 		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
// 		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
// 	};
// 	// first, configure the cube's VAO (and VBO)
// 	unsigned int VBO, cubeVAO;
// 	glGenVertexArrays(1, &cubeVAO);
// 	glGenBuffers(1, &VBO);

// 	glBindBuffer(GL_ARRAY_BUFFER, VBO);
// 	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// 	glBindVertexArray(cubeVAO);

// 	// position attribute
// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
// 	glEnableVertexAttribArray(0);
// 	// normal attribute
// 	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
// 	glEnableVertexAttribArray(1);


// 	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
// 	unsigned int lightVAO;
// 	glGenVertexArrays(1, &lightVAO);
// 	glBindVertexArray(lightVAO);

// 	glBindBuffer(GL_ARRAY_BUFFER, VBO);
// 	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
// 	glEnableVertexAttribArray(0);


// 	// draw in wireframe
// 	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

// 	// render loop
// 	// -----------
// 	while (!glfwWindowShouldClose(window))
// 	{
// 		// per-frame time logic
// 		// --------------------
// 		float currentFrame = glfwGetTime();
// 		deltaTime = currentFrame - lastFrame;
// 		lastFrame = currentFrame;

// 		// input
// 		// -----
// 		processInput(window);

// 		// render
// 		// ------
// 		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
// 		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// 		// don't forget to enable shader before setting uniforms
// 		ourShader.use();
// 		// be sure to activate shader when setting uniforms/drawing objects
// 		ourShader.use();
// 		ourShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
// 		ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
// 		ourShader.setVec3("lightPos", lightPos);
// 		ourShader.setVec3("viewPos", camera.Position);

// 		// view/projection transformations
// 		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
// 		glm::mat4 view = camera.GetViewMatrix();
// 		ourShader.setMat4("projection", projection);
// 		ourShader.setMat4("view", view);

// 		// render the loaded model
// 		glm::mat4 model = glm::mat4(1.0f);
// 		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
// 		model = glm::scale(model, glm::vec3(5.5f, 5.5f, 5.5f));	// it's a bit too big for our scene, so scale it down
// 		ourShader.setMat4("model", model);
// 		monkey.Draw(ourShader);

// 		// also draw the lamp object
// 		lampShader.use();
// 		lampShader.setMat4("projection", projection);
// 		lampShader.setMat4("view", view);
// 		model = glm::mat4(1.0f);
// 		model = glm::translate(model, lightPos);
// 		model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
// 		lampShader.setMat4("model", model);

// 		glBindVertexArray(lightVAO);
// 		glDrawArrays(GL_TRIANGLES, 0, 36);


// 		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
// 		// -------------------------------------------------------------------------------
// 		glfwSwapBuffers(window);
// 		glfwPollEvents();
// 	}

// 	// glfw: terminate, clearing all previously allocated GLFW resources.
// 	// ------------------------------------------------------------------
// 	glfwTerminate();
// 	return 0;
// }

// // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// // ---------------------------------------------------------------------------------------------------------
// void processInput(GLFWwindow *window)
// {
// 	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
// 		glfwSetWindowShouldClose(window, true);

// 	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
// 		camera.ProcessKeyboard(FORWARD, deltaTime);
// 	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
// 		camera.ProcessKeyboard(BACKWARD, deltaTime);
// 	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
// 		camera.ProcessKeyboard(LEFT, deltaTime);
// 	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
// 		camera.ProcessKeyboard(RIGHT, deltaTime);
// }

// // glfw: whenever the window size changed (by OS or user resize) this callback function executes
// // ---------------------------------------------------------------------------------------------
// void framebuffer_size_callback(GLFWwindow* window, int width, int height)
// {
// 	// make sure the viewport matches the new window dimensions; note that width and 
// 	// height will be significantly larger than specified on retina displays.
// 	glViewport(0, 0, width, height);
// }

// // glfw: whenever the mouse moves, this callback is called
// // -------------------------------------------------------
// void mouse_callback(GLFWwindow* window, double xpos, double ypos)
// {
// 	if (firstMouse)
// 	{
// 		lastX = xpos;
// 		lastY = ypos;
// 		firstMouse = false;
// 	}

// 	float xoffset = xpos - lastX;
// 	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

// 	lastX = xpos;
// 	lastY = ypos;

// 	camera.ProcessMouseMovement(xoffset, yoffset);
// }

// // glfw: whenever the mouse scroll wheel scrolls, this callback is called
// // ----------------------------------------------------------------------
// void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
// {
// 	camera.ProcessMouseScroll(yoffset);
// }
