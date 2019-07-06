#ifdef _WIN32 || _WIN64 // windows glad.h order is different?
    #include <GLFW/glfw3.h>
    #include <glad/glad.h>
#else
    #include <glad/glad.h>
    #include <GLFW/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include <learnopengl/Camera.h> // custom camera.h
#include <learnopengl/shader.h>
#ifdef _WIN32 || _WIN64
	#include <learnopengl/Model.h>
#else
	#include <learnopengl/model.h>
#endif
#include <learnopengl/filesystem.h>
#include <string.h>
#include <algorithm>
#include <limits>
#include <thread>
#include <future>

// Dear ImGui static library
#include "../includes/imgui/imgui.h"
#include "../includes/imgui/imgui_impl_glfw.h"
#include "../includes/imgui/imgui_impl_opengl3.h"

#include "../headers/CubeCore.h"
#include "../headers/Skeleton.h"
#include "../headers/Animation.h"
#include "../headers/Bone.h"
#include "../headers/PointLight.h"
#include "../headers/PointCloud.h"

#define ROOT_DIR FileSystem::getRoot()

#define FPS 120
// settings
struct screen_size {
	unsigned int width = 1600, height = 1000;
	unsigned int posX = 0, posY = 0;
} init_window, curr_window, fullscreen_window, region_a, region_b;

// states (feature on off)
struct state_flags {
    // Controls
    bool play = false;
    bool is_full_screen = false;
    bool lock_view = false;
    bool show_cloud = false;

    bool show_selected_frames = false;

    bool mouse_btn2_pressed = false;
} states;

// timing
struct times {
    float delta_time = 0.0f;	// time between current frame and last frame
    float last_frame = 0.0f;
    long num_frames = 0;
    float agg_fps, agg_anim, agg_input, agg_render = 0.f; // for benchmarking
} timings;

// INPUT CALLBACKS
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_movement(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void keyboardInput(GLFWwindow *window);

// IMGUI
void imgui_file_selector(string name, string root, string &filename);

// CUSTOM
Animation* get_anim(string amc);
void draw(Model plane, Model sphere, Model cylinder, CubeCore cube, Shader diffShader, Shader lampShader, screen_size window_region);
pair<vector<vector<float>>, pair<float,float>> compute_distance_matrix();


// Camera
Camera camera(glm::vec3(0.0f, 1.4f, 2.f));
float lastX = init_window.width / 2.0f;
float lastY = init_window.height / 2.0f;
bool firstMouse = true;

// Lights
PointLight lamp = PointLight();


float scale = 0.056444f; //inches to meters

int skip_frame = 1;
int k = 40;
float progress = 0;

// Animation & skeleton
string res_path = ROOT_DIR + "/resources/";
string file_asf = res_path + "mocap/02/02.asf";
string file_amc = res_path + "mocap/02/02_0";
//string file_asf = res_path + "mocap/14/14.asf";
//string file_amc = res_path + "mocap/14/14_0";
map<string, Animation*> anim_cache;

// Loading mocap data: skeleton from .asf and animation (poses) from .amc
Skeleton* sk = new Skeleton((char*)file_asf.c_str(), scale);
string anim_a = (file_amc + "2.amc");
string anim_b = (file_amc + "3.amc");

int main()
{
	/** GLFW initialization **/
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const char* glsl_version = "#version 330";
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
    #endif

	/** GLFW window **/
	GLFWvidmode *primary_mode = (GLFWvidmode*)glfwGetVideoMode(glfwGetPrimaryMonitor());
	fullscreen_window.width = primary_mode->width; 
	fullscreen_window.height = primary_mode-> height;
	curr_window.width = fullscreen_window.width * .75f;
	curr_window.height = fullscreen_window.height * .75f;
	init_window = curr_window;
	cout << "Screen size: " << fullscreen_window.width << "x" << fullscreen_window.height << endl;
	cout << "Init window size: " << init_window.width << "x" << init_window.height << endl;
	region_a.width = curr_window.width / 2;
	region_a.height = curr_window.height;
	region_b.width = curr_window.width / 2;
	region_b.height = curr_window.height;
	region_b.posX = curr_window.width/2;

	GLFWwindow* window = glfwCreateWindow(curr_window.width, curr_window.height, "Mocap", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetScrollCallback(window, mouse_scroll);
	glfwSetCursorPosCallback(window, mouse_movement);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	/** GLFW capture mouse **/
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	/** GLAD: loads the correct opengl functions **/
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Disable v-sync
	glfwSwapInterval(0);

	/** configure global opengl state **/
	glEnable(GL_DEPTH_TEST);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("../resources/fonts/Roboto-Medium.ttf", 32.0f);
    io.Fonts->AddFontFromFileTTF("../resources/fonts/Cousine-Regular.ttf", 30.0f);
    io.Fonts->AddFontFromFileTTF("../resources/fonts/DroidSans.ttf", 32.0f);
    io.Fonts->AddFontFromFileTTF("../resources/fonts/ProggyTiny.ttf", 20.0f);
	
	/** build and compile shaders + load .obj 3D models**/
#ifdef _WIN32 || _WIN64
	Shader diffShader("shaders/basic_lighting.vs", "shaders/basic_lighting.fs");
	Shader lampShader("shaders/lamp.vs", "shaders/lamp.fs");
	//string r = (string)ROOT_DIR;
//      Model sphere(r.append("\\res\\sphere\\sphere.obj"));
//      Model cylinder(r.append("\\res\\cylinder\\cylinder.obj"));
//      Model plane(r.append("\\res\\plane\\plane.obj"));
//      Model monkey(r.append("\\res\\monkey\\monkey.obj"));
#else
	Shader diffShader((ROOT_DIR + "/shaders/basic_lighting.vs").c_str(), (ROOT_DIR + "/shaders/basic_lighting.fs").c_str());
	Shader lampShader((ROOT_DIR + "/shaders/lamp.vs").c_str(), (ROOT_DIR + "/shaders/lamp.fs").c_str());
#endif
	Model sphere(FileSystem::getPath("resources/objects/sphere/sphere.obj"));
	Model cylinder(FileSystem::getPath("resources/objects/cylinder/cylinder.obj"));
	Model plane(FileSystem::getPath("resources/objects/plane/plane.obj"));
	Model monkey(FileSystem::getPath("resources/objects/monkey/monkey.obj"));
	CubeCore cube = CubeCore();

	// Lights buffers
	lamp.setBuffers();

	// Add first anim to cache 
	get_anim(anim_a);
	get_anim(anim_b);
	sk->apply_pose(NULL);
	cube.setBuffers();

	// Set shader to use
	diffShader.use();

	
    // Vars
    pair<int,int> selected_frames = {0,0};
	pair<int, int> min_dist_frames = {2,2};
	pair<float, float> dist_mat_range = {-1,-1};
    vector<vector<float>> dist_mat;
    future<pair<vector<vector<float>>, pair<float,float>>> ftr;
    bool compute_running = false;
    bool show_selected_text, show_hoovered_text = false;

	// lambda function to normalise [0,1] a float value
	auto normalise = [](float val, float min, float max) {
		return (val - min) / (max - min);
	};

	/** render loop **/
	while (!glfwWindowShouldClose(window))
	{
		// frame time
		timings.num_frames++;
		float currentFrame = glfwGetTime();
		timings.delta_time = currentFrame - timings.last_frame;
		timings.last_frame = currentFrame;
		float last_fps = 1.f / timings.delta_time;
		timings.agg_fps += last_fps;


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Begin("Controls");
        if (ImGui::Button("Play"))
            states.play = !states.play;   
        ImGui::Separator();

        imgui_file_selector("Select motion A",res_path + "mocap/", anim_a);
        ImGui::SameLine();
        imgui_file_selector("Select motion B",res_path + "mocap/", anim_b);
        ImGui::BulletText( ("Motion A: " + anim_a.substr(anim_a.find_last_of("/"))).c_str());
        ImGui::SameLine();
        ImGui::BulletText( ("Motion B: " + anim_b.substr(anim_b.find_last_of("/"))).c_str());

        ImGui::Separator();

        if(ImGui::TreeNode("Distance Matrix")) {
            /* - This button starts an async thread to do the distance matrix computation
               - When computation is done, the value is retrivied */
            if (ImGui::Button("Compute distance matrix")) {
                ftr = std::async(compute_distance_matrix);
                compute_running = true;
            }
            // Only tries to retrieve the return value of the thread compute, if it is has started.
            if (compute_running) {
                auto status = ftr.wait_for(0ms);
                if(status == std::future_status::ready){
                    if (ftr.valid()) {
                        pair<vector<vector<float>>, pair<float, float>> dist_mat_res = ftr.get();
                        dist_mat = dist_mat_res.first;
                        dist_mat_range = dist_mat_res.second;

                        // cout << "dist_mat_res.first.size() = " << dist_mat_res.first.size() << endl;
                    }
                    compute_running = false;
                }
            }
            ImGui::Text("the dist_mat size is %d", dist_mat.size());
            ImGui::Text("range distance %f-%f", dist_mat_range.first, dist_mat_range.second);

            ImGui::ProgressBar(progress, ImVec2(0.0f,0.0f));
            static int btn_size = 10;
			static int lines = 10;
            ImGui::SliderInt("Lines", &btn_size, 1, 300);
			ImGui::SliderInt("Button size", &lines, 1, 50);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f));
            ImGui::BeginChild("scrolling", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * lines), true, ImGuiWindowFlags_HorizontalScrollbar);
            pair<int,int> hoovered = {0,0};
            for (int line = 0; line < dist_mat.size(); line++)
            {
                vector<float> mat_line = dist_mat.at(line);
                // Display random stuff (for the sake of this trivial demo we are using basic Button+SameLine. If you want to create your own time line for a real application you may be better off
                // manipulating the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets yourself. You may also want to use the lower-level ImDrawList API)
                for (int n = 0; n < mat_line.size(); n++)
                {
                    if (n > 0) ImGui::SameLine();
                    ImGui::PushID(n*line);
                    float hue = n*0.05f;
					float dist = mat_line.at(n);
					float normalised_val = normalise(dist, dist_mat_range.first, dist_mat_range.second);
					ImVec4 btn_color = ImVec4(normalised_val, normalised_val, normalised_val, 1.f);
                    ImGui::PushStyleColor(ImGuiCol_Button, btn_color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
                    // char num_buf[16];
                    // sprintf(num_buf, "%d", line);
                    if (ImGui::Button("", ImVec2( btn_size, btn_size)) ) {
                        selected_frames = {line + 1, n + 1};
                        states.show_selected_frames = true;
                    }  
                    if (ImGui::IsItemHovered()) {
                        hoovered = {line + 1, n + 1};
                        selected_frames = {line + 1, n + 1};
                        show_hoovered_text = true;
                    } else 
						show_hoovered_text = false;
                    ImGui::PopStyleColor(3);
                    ImGui::PopID();
                }
            }
            float scroll_x = ImGui::GetScrollX();
            float scroll_max_x = ImGui::GetScrollMaxX();
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            float scroll_x_delta = 0.0f;
            ImGui::SmallButton("<<"); if (ImGui::IsItemActive()) { scroll_x_delta = -ImGui::GetIO().DeltaTime * 1000.0f; } ImGui::SameLine();
            ImGui::Text("Scroll from code"); ImGui::SameLine();
            ImGui::SmallButton(">>"); if (ImGui::IsItemActive()) { scroll_x_delta = +ImGui::GetIO().DeltaTime * 1000.0f; } ImGui::SameLine();
            ImGui::Text("%.0f/%.0f", scroll_x, scroll_max_x);
            if (scroll_x_delta != 0.0f)
            {
                ImGui::BeginChild("scrolling"); // Demonstrate a trick: you can use Begin to set yourself in the context of another window (here we are already out of your child window)
                ImGui::SetScrollX(ImGui::GetScrollX() + scroll_x_delta);
                ImGui::EndChild();
            }
            ImGui::Spacing();
            ImGui::Checkbox("Show selected_frames", &states.show_selected_frames);
            if (show_selected_text) {
                ImGui::Text("Selected frames A-B: %d-%d", selected_frames.first, selected_frames.second); 
				ImGui::SameLine();
				// if (dist_mat.size() > 0) {
					float dist = dist_mat.at(selected_frames.first).at(selected_frames.second);
					ImGui::Text("| Distance: %.0f (%.0f)", dist, normalise(dist, dist_mat_range.first, dist_mat_range.second));
				// }
			}
            // if (show_hoovered_text) {
            //     ImGui::SameLine(); ImGui::Text("Hoovering on frames (A-B): %d-%d", hoovered.first, hoovered.second);
            // }

            ImGui::TreePop();
        }


        ImGui::Separator();
        if (ImGui::Button("Exit"))
            break; //exit gameloop
        ImGui::End();


		Animation* anim = get_anim(anim_a);
		// Update animation 
		if (states.play)
		{
			if (anim->isOver()) {
				anim->reset();
				sk->resetAll();
			}
			int frame = anim->getCurrentFrame();
			sk->apply_pose(anim->getPoseAt(frame + skip_frame));
			for (int i = 0; i < skip_frame; i++) {
				anim->getNextPose();
			}
		}
		else {
            if (states.show_selected_frames){
			    sk->apply_pose(get_anim(anim_a)->getPoseAt(selected_frames.first));
            }
		}
		float pose_time = glfwGetTime() - currentFrame;
		timings.agg_anim += pose_time;

		// input
		// -----
		float input_start_time = glfwGetTime();
		keyboardInput(window);
		float input_time = (glfwGetTime() - input_start_time);
		timings.agg_input += input_time;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

		glViewport(region_a.posX, region_a.posY, region_a.width, region_a.height);
		draw(plane, sphere, cylinder, cube, diffShader, lampShader, region_a);

		anim = get_anim(anim_b);
		// Update animation 
		if (states.play)
		{
			if (anim->isOver()) {
				anim->reset();
				sk->resetAll();
			}
			int frame = anim->getCurrentFrame();
			sk->apply_pose(anim->getPoseAt(frame + skip_frame));
			for (int i = 0; i < skip_frame; i++) {
				anim->getNextPose();
			}
		}
		else {
			if (states.show_selected_frames){
			    sk->apply_pose(get_anim(anim_b)->getPoseAt(selected_frames.second));
            }
		}
		glViewport(region_b.posX, region_b.posY, region_b.width, region_b.height);
		draw(plane, sphere, cylinder, cube, diffShader, lampShader, region_b);

        {
            ImGui::Begin("Another Window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            ImGui::End();
        }


		//glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		//-------------------------------------------------------------------------------
		glfwPollEvents();
        glViewport(curr_window.posX, curr_window.posY, curr_window.width, curr_window.height);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		// wait for sync
		while (glfwGetTime() - currentFrame < 1.f / FPS) { ; }
	}

	// de-allocation
	cube.~CubeCore();
	sk->~Skeleton();
	// anim->~Animation();

	// end glfw and ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
	return 0;
}

void draw(Model plane, Model sphere, Model cylinder, CubeCore cube, Shader diffShader, Shader lampShader, screen_size window_region)
{
	/** Start Rendering **/
	float render_start_time = glfwGetTime();

	// activate shader
	diffShader.use();
	diffShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	diffShader.setVec3("lightPos", lamp.Position);
	diffShader.setVec3("viewPos", camera.Position);

	// pass projection matrix to shader (note that in this case it could change every frame)
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window_region.width / (float)window_region.height, 0.1f, 100.0f);
	diffShader.setMat4("projection", projection);

	// camera/view transformation
	if (states.lock_view) {
		diffShader.setMat4("view", glm::lookAt(camera.Position, sk->getPos(), camera.Up));
	}
	else {
		diffShader.setMat4("view", camera.GetViewMatrix());
	}

	// floor
	diffShader.setVec3("objectColor", .95f, 0.95f, 0.95f);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(50.f, 0.001f, 50.f));
	diffShader.setMat4("model", model);
	//glBindVertexArray(cube.VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	plane.Draw(diffShader);


	// render Skeleton, root first
	float render_scale = .02f;
	model = glm::scale(sk->getJointMat(), glm::vec3(render_scale));
	diffShader.setVec3("objectColor", 1.0f, 0.1f, 0.1f);
	diffShader.setMat4("model", model);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	sphere.Draw(diffShader);

	for (Bone* bone : sk->getAllBones())
	{
		diffShader.setVec3("objectColor", 0.31f, 1.f, 0.31f);

		bool highlight = !strcmp(bone->name.c_str(), "rtibia") || !strcmp(bone->name.c_str(), "ltibia")
			|| !strcmp(bone->name.c_str(), "rradius") || !strcmp(bone->name.c_str(), "lradius")
			|| !strcmp(bone->name.c_str(), "rclavicle") || !strcmp(bone->name.c_str(), "lclavicle")
			//|| !strcmp(bone->name.c_str(), "rhumerus") || !strcmp(bone->name.c_str(), "lhumerus")
		|| !strcmp(bone->name.c_str(), "lowerback");
		if (highlight) {
			diffShader.setVec3("objectColor", 0.31f, 0.31f, 1.f);
		}
		// calculate the model matrix for each object and pass it to shader before drawing
		model = glm::scale(bone->getJointMat(), glm::vec3(render_scale));
		diffShader.setMat4("model", model);
		sphere.Draw(diffShader);

		// Draw segment
		diffShader.setVec3("objectColor", .6f, 0.6f, 0.6f);
		if (highlight) {
			diffShader.setVec3("objectColor", 0.31f, 0.31f, .6f);
		}
		model = glm::scale(bone->getSegMat(), glm::vec3(render_scale));
		diffShader.setMat4("model", model);
		cylinder.Draw(diffShader);

		//Cloud point guideline
		/*diffShader.setVec3("objectColor", 0.41f, 0.41f, .6f);
		model = glm::scale(bone->cp_planez, glm::vec3(render_scale));
		diffShader.setMat4("model", bone->cp_planez);
		glBindVertexArray(cube.VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		diffShader.setVec3("objectColor", 6.0f, 0.41f, 0.41f);
		model = glm::scale(bone->cp_planex, glm::vec3(render_scale));
		diffShader.setMat4("model", bone->cp_planex);
		glBindVertexArray(cube.VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);*/

		//Cloud points
		if (states.show_cloud) 
		{
			diffShader.setVec3("objectColor", .8f, 0.8f, 0.8f);
			for (auto p : bone->getLocalPointCloud()->points) {
				model = glm::scale(bone->getLocalPointCloud()->getPointMat(p), glm::vec3(0.01f));
				diffShader.setMat4("model", model);
				// sphere.Draw(diffShader);
				glBindVertexArray(cube.VAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}			
	}

	// Draw Lights
	lampShader.use();
	lampShader.setMat4("projection", projection);
	lampShader.setMat4("view", camera.GetViewMatrix());
	model = glm::mat4(1.0f);
	model = glm::translate(model, lamp.Position);
	model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube
	lampShader.setMat4("model", model);

	glBindVertexArray(lamp.VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	float render_time = glfwGetTime() - render_start_time;
	timings.agg_render += render_time;
	/** END RENDERING **/
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void keyboardInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		cout << endl << "==========\tPerformance report:\t==========" << endl << endl;
		cout << "\tAVG FPS: " << timings.agg_fps / timings.num_frames << endl;
		cout << "\tAVG anim update time = " << timings.agg_anim / timings.num_frames * 1000.f << endl;
		cout << "\tAVG input time = " << timings.agg_input / timings.num_frames * 1000.f << endl;
		cout << "\tAVG render time = " << timings.agg_render / timings.num_frames * 1000.f << endl;
		cout << endl << "==================================================" << endl;
		cout << timings.agg_fps / timings.num_frames << endl
			<< timings.agg_anim / timings.num_frames * 1000.f << endl
			<< timings.agg_input / timings.num_frames * 1000.f << endl
			<< timings.agg_render / timings.num_frames * 1000.f << endl;
		cout << endl << "lamp pos = " << lamp.Position.x << ", " << lamp.Position.y <<
			", " << lamp.Position.z;
		glfwSetWindowShouldClose(window, true);
	}

	// TOGGLE: Play button
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		states.play = !states.play;
		std::cout << "Play" << "\n";
	}
	// TOGGLE: Fullscreen
	if ((glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS))
	{
		states.is_full_screen = !states.is_full_screen;
		if (states.is_full_screen) {
			glfwSetWindowSize(window, fullscreen_window.width, fullscreen_window.height);
			// upper left corner
			glfwSetWindowPos(window, 0, 0);
			curr_window.width = fullscreen_window.width;
			curr_window.height = fullscreen_window.height;
		}
		else {
			glfwSetWindowSize(window, init_window.width, init_window.height);
			glfwSetWindowPos(window, 0, 0);
			curr_window.width = init_window.width;
			curr_window.height = init_window.height;
		}
		region_a.width = curr_window.width / 2;
		region_a.height = curr_window.height;
		region_b.width = curr_window.width / 2;
		region_b.height = curr_window.height;
		region_b.posX = curr_window.width/2;
		glViewport(0, 0, curr_window.width, curr_window.height);
	}
	// TOGGLE: lock view
	if ((glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS))
		states.lock_view = !states.lock_view;
	// TOGGLE: show cloud point
	if ((glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS))
		states.show_cloud = !states.show_cloud;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, timings.delta_time);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, timings.delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, timings.delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, timings.delta_time);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, timings.delta_time);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, timings.delta_time);

	// Lights control
	float light_offset = 1.f;
	if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
		lamp.Position += glm::vec3(0.f, light_offset, 0.f);
	if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
		lamp.Position += glm::vec3(0.f, -light_offset, 0.f);
	if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
		lamp.Position += glm::vec3(-light_offset, 0.f, 0.f);
	if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
		lamp.Position += glm::vec3(light_offset, 0.f, 0.f);
	if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
		lamp.Position += glm::vec3(0.f, 0.f, -light_offset);
	if (glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS)
		lamp.Position += glm::vec3(0.f, 0.f, light_offset);

	// Switching animation 01-09
	// if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
	// 	change_anim(file_amc + "1.amc");
	// }
	// if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
	// 	// amc = file_amc + "3.amc"; //demo
	// 	change_anim(file_amc + "2.amc");
	// }
	// if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
	// 	change_anim(file_amc + "3.amc");
	// }
	// if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
	// 	change_anim(file_amc + "4.amc");
	// }
	// if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
	// 	// amc = res_path + "mocap/14/14_06.amc"; //demo
	// 	change_anim(file_amc + "5.amc");
	// }
	// if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
	// 	change_anim(file_amc + "6.amc");
	// }
	// if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
	// 	change_anim(file_amc + "7.amc");
	// }
	// if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
	// 	change_anim(file_amc + "8.amc");
	// }
	// if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
	// 	change_anim(file_amc + "9.amc");		
	// }
}

// Get the animation from the cache, adds it if not present
Animation* get_anim(string amc)
{
	if (!anim_cache.count(amc)) // if animation is NOT in cache exits
	{
		Animation* an = new Animation(sk, (char*)(amc).c_str());
		anim_cache.insert({ amc, an }); // insert only if not present
	}
	return anim_cache[amc];
}

// glfw: called when window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	curr_window.width = width;
	curr_window.height = height;
	region_a.width = curr_window.width / 2;
	region_a.height = curr_window.height;
	region_b.width = curr_window.width / 2;
	region_b.height = curr_window.height;
	region_b.posX = curr_window.width/2;
}


// glfw: called when mouse moves
void mouse_movement(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastY = ypos;
    lastX = xpos;
    if (states.mouse_btn2_pressed) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// glfw: called when mouse wheel is used
void mouse_scroll(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// glfw: called when mouse button is used
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Mouse button 2 pressed == move camera
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS)
            states.mouse_btn2_pressed = true;
        else if (action == GLFW_RELEASE)
            states.mouse_btn2_pressed = false;
    }
}


void imgui_file_selector(string name, string root, string &filename) 
{
    if (ImGui::Button(name.c_str()))
        ImGui::OpenPopup(("my_file_popup" + name).c_str());
    if (ImGui::BeginPopup(("my_file_popup" + name).c_str()))
    {
        vector<string> sorted_dirs = FileSystem::getDirs(root);
        std::sort(sorted_dirs.begin(), sorted_dirs.end());
        for (string dirs : sorted_dirs) {
            if(ImGui::BeginMenu(dirs.c_str())) // add files to dir
            {
                cout << "get Files form " << res_path + "mocap/" + dirs << endl;
                vector<string> sorted_dirs = FileSystem::getFiles(root + dirs + "/");
                std::sort(sorted_dirs.begin(), sorted_dirs.end());
                for (string file : sorted_dirs) {
                    if (ImGui::MenuItem(file.c_str()) ) {
                        filename = root + dirs + "/" + file;
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndPopup();
    }    
}

pair<vector<vector<float>>, pair<float,float>> compute_distance_matrix()
{
	cout << "Calculating distance matrix" << endl;
	const int num_frames_a = get_anim(anim_a)->getNumberOfFrames();
	const int num_frames_b = get_anim(anim_b)->getNumberOfFrames();
	map<int, PointCloud*> cloud_a_to_frame;
	map<int, PointCloud*> cloud_b_to_frame;

	for (int i = 1; i < num_frames_a; i++) {
		Pose* pose = get_anim(anim_a)->getPoseAt(i);
		cloud_a_to_frame.insert({ i, sk->getGlobalPointCloud(pose) });
	}
	for (int i = 1; i < num_frames_b; i++) {
		Pose* pose = get_anim(anim_b)->getPoseAt(i);
		cloud_b_to_frame.insert({ i, sk->getGlobalPointCloud(pose) });
	}

	vector<vector<float>> distance_mat;
	pair<int, int> min_dist_frames = {32,56};
	pair<float, float> range = {std::numeric_limits<float>::infinity(),-1};
	for (int i = 1; i < num_frames_a; i++) {
		PointCloud* cloud_a = new PointCloud();
		std::for_each(cloud_a_to_frame.begin(), cloud_a_to_frame.end(), 
			[&](pair<int, PointCloud*> p) { if(p.first >= i && p.first < i + k) cloud_a->addPointCloud(p.second); });

		vector<float> dist_mat_row;
		for (int j = 1; j < num_frames_b; j++) {
			PointCloud* cloud_b = new PointCloud();
			std::for_each(cloud_b_to_frame.begin(), cloud_b_to_frame.end(),
				[&](pair<int, PointCloud*> p) { if (p.first > j-k && p.first <= j) cloud_b->addPointCloud(p.second); });

			float distance = -1.f;
			if (i+k-1 <= num_frames_a && j-k+1 > 0) {
				distance = cloud_a->computeDistance(cloud_b);
				if (distance != -1.f && distance < range.first) {
					range.first = distance;
					min_dist_frames = { i,j };
				}
				if (distance > range.second) {
					range.second = distance;
				}
			}
			//cout << "| @"<<i<<"," <<j<<"\td="<<distance<<"\t"; 
			dist_mat_row.push_back(distance);
		}
        progress = (float)i/(float)num_frames_a;
		// cout << "i = "<< i <<endl;
        // cout << "progress = " << progress << endl;
		distance_mat.push_back(dist_mat_row);
	}

    cout << distance_mat.size() << "== size of mat row" << endl;

	cout << "Min distance at frames " << min_dist_frames.first << " - " << min_dist_frames.second << endl;
    return {distance_mat, range};
}