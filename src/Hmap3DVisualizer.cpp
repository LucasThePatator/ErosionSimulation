#include "Hmap3DVisualizer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Heightmap.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ", " << description;
}

GLuint LoadShaders(const std::string vertex_file_path, const std::string fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else {
        std::cerr << "Impossible to open " << vertex_file_path << ".Are you in the right directory ? Don't forget to read the FAQ !" << std::endl;
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    std::cout << "Compiling shader : " << vertex_file_path << std::endl;;
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        std::cerr << &VertexShaderErrorMessage[0] << std::endl;
    }

    // Compile Fragment Shader
    std::cout << "Compiling shader : " << fragment_file_path << std::endl;;
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        std::cerr << &FragmentShaderErrorMessage[0] << std::endl;
    }

    // Link the program
    std::cout << "Linking program" << std::endl;
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::cerr << &ProgramErrorMessage[0] << std::endl;
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

Hmap3DVizualizer::Hmap3DVizualizer(int display_w, int display_h, bool debug):
    _display_h(display_h),
    _display_w(display_w),
    _debug(debug)
{
    glfwSetErrorCallback(glfw_error_callback);
   
}

Hmap3DVizualizer::~Hmap3DVizualizer()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(_window);
    glfwTerminate();
}

void Hmap3DVizualizer::init(const ErosionSimulation::Heightmap* hmap, const std::vector<std::vector<ErosionSimulation::point2f>>* trajs)
{
    _hmap = hmap;
    _trajs = trajs;

    auto glfw_error = glfwInit();
    if (!glfw_error)
    {
        
        throw std::runtime_error(std::to_string(glfw_error));
    }

    //const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

    _window = glfwCreateWindow(1280, 720, "Erosion simulation", nullptr, nullptr);


    if (!_window)
    {
        throw std::runtime_error("Could not create window");
    }

    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1); // Enable vsync

    if (GLEW_OK != glewInit())
    {
        GLenum error;
        do
        {
            error = glGetError();
            std::cerr << glewGetErrorString(error) << std::endl;
        } while (GL_NO_ERROR != error);

        return;
    }

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    if (_debug)
    {
        std::cout << "Vendor graphic card: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Version GL: " << glGetString(GL_VERSION) << std::endl;
    }

    // Setup Platform/Renderer backends
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //temp test
    glGenBuffers(1, &vertexbufferID);
    glGenBuffers(1, &normalsbufferID);
    glGenBuffers(1, &uvbufferID);
    glGenBuffers(1, &elementbufferID);

    programID = LoadShaders("E:/Workspace/ErosionSimulation/shaders/terrain_vertex.glsl", "E:/Workspace/ErosionSimulation/shaders/terrain_frag.glsl");


}

void Hmap3DVizualizer::renderSlider(const Parameter& parameter)
{
    switch (parameter.type)
    {
    case Parameter::TYPE::INT:
        ImGui::SliderInt(parameter.name.c_str(), reinterpret_cast<int*>(parameter.pointer), std::get<int>(parameter.min), std::get<int>(parameter.max));
        break;
    case Parameter::TYPE::FLOAT:
        ImGui::SliderFloat(parameter.name.c_str(), reinterpret_cast<float*>(parameter.pointer), std::get<float>(parameter.min), std::get<float>(parameter.max));
        break;
    }
}

void Hmap3DVizualizer::renderUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Parameters");

    for (const auto& p : _parameters)
    {
        renderSlider(p);
    }
    if (ImGui::Button("New"))
    {
        _onNew();
    }
    ImGui::SameLine();
    if (ImGui::Button("Run"))
    {
        _onRun();
    }

    if (ImGui::CollapsingHeader("Camera"))
    {
        ImGui::SliderFloat("Elevation##Camera", &_cameraElevation, 0.f, 90.f);
        ImGui::SliderFloat("Azimut##Camera", &_cameraAzimut, 0.f, 360.f);
        ImGui::SliderFloat("Distance##Camera", &_cameraDistance, 0.f, 1000.f);
    }

    if (ImGui::CollapsingHeader("Sun"))
    {
        ImGui::SliderFloat("Elevation##Sun", &_sunElevation, 0.f, 90.f);
        ImGui::SliderFloat("Azimut##Sun", &_sunAzimut, 0.f, 360.f);
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void print_mat4(const glm::mat4& mat)
{
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            std::cout << mat[col][row] << "\t";
        }
        std::cout << std::endl;
    }
}


void Hmap3DVizualizer::run()
{
    ImVec4 clear_color(0.f, 0.f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();

        glfwGetFramebufferSize(_window, &_display_w, &_display_h);
        glViewport(0, 0, _display_w, _display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        const glm::mat4 view = glm::rotate(glm::radians(180.f), glm::vec3{ 0.f, 1.f, 0.f })
            *glm::translate(glm::vec3{ 0.f, 0.f , _cameraDistance})
            *glm::rotate(glm::radians(-90.f - _cameraElevation), glm::vec3{ 1.f, 0.f, 0.f })
            *glm::rotate(glm::radians(_cameraAzimut), glm::vec3{ 0.f, 0.f, 1.f });

        const glm::mat4 projection = glm::perspective(glm::radians(_cameraFoV), 4.f/3.f, 1.f, 1000.f);
        const glm::mat4 mvp = projection * view;

        const float elevation_rad = glm::radians(_sunElevation);
        const float azimut_rad = glm::radians(_sunAzimut);

        const glm::vec3 sun_position{ std::cos(azimut_rad) * std::cos(elevation_rad),
                                        std::sin(azimut_rad) * std::cos(elevation_rad),
                                        std::sin(elevation_rad) };

        // Use our shader

        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferID);
        const std::vector<float> vertexBuffer = makeVertexBuffer();
        glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(float), vertexBuffer.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, normalsbufferID);
        const std::vector<float> normalsBuffer = makeNormalsBuffer();
        glBufferData(GL_ARRAY_BUFFER, normalsBuffer.size() * sizeof(float), normalsBuffer.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, uvbufferID);
        const std::vector<float> uvBuffer = makeUVBuffer();
        glBufferData(GL_ARRAY_BUFFER, uvBuffer.size() * sizeof(float), uvBuffer.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbufferID);
        const std::vector<unsigned int> indexBuffer = makeIndexBuffer();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), indexBuffer.data(), GL_STATIC_DRAW);

        glUseProgram(programID);

        GLuint MatrixID = glGetUniformLocation(programID, "V");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);

        MatrixID = glGetUniformLocation(programID, "MV");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &view[0][0]);

        MatrixID = glGetUniformLocation(programID, "MVP");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

        MatrixID = glGetUniformLocation(programID, "sun_position");
        glUniform3fv(MatrixID, 1, &sun_position[0]);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferID);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbufferID);
        glVertexAttribPointer(
            1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, uvbufferID);
        glVertexAttribPointer(
            2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        glDrawElements(
            GL_TRIANGLES,      // mode
            indexBuffer.size(),    // count
            GL_UNSIGNED_INT,   // type
            (void*)0           // element array buffer offset
        );        

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        renderUI();

        glfwSwapBuffers(_window);
    }
}

std::vector<float> Hmap3DVizualizer::makeVertexBuffer()
{
    //export the normals
    std::vector<float> vertices;
    for (unsigned int y = 0; y < _hmap->_height; y++)
    {
        for (unsigned int x = 0; x < _hmap->_width; x++)
        {
            vertices.insert(vertices.cend(), { static_cast<float>(x) - _hmap->_width / 2, static_cast<float>(y) - _hmap->_height / 2, _hmap->at(x, y) });
        }
    }
    return vertices;
}

std::vector<float> Hmap3DVizualizer::makeNormalsBuffer()
{
    const auto gradient = _hmap->computeGradient();
    //export the normals
    std::vector<float> normals;
    for (unsigned int y = 0; y < _hmap->_height; y++)
    {
        for (unsigned int x = 0; x < _hmap->_width; x++)
        {
            normals.insert(normals.cend(), { -gradient[2 * (x + y * _hmap->_width)], -gradient[2 * (x + y * _hmap->_width) + 1], 1.f }); //Approximate the tan to compute quicker
        }
    }
    return normals;
}

std::vector<float> Hmap3DVizualizer::makeUVBuffer()
{
    //export the texture coodinates
    std::vector<float> uv;
    for (unsigned int y = 0; y < _hmap->_height; y++)
    {
        for (unsigned int x = 0; x < _hmap->_width; x++)
        {
            uv.insert(uv.cend(), { static_cast<float>(x) / _hmap->_width, static_cast<float>(y) / _hmap->_height });
        }
    }
    return uv;
}

std::vector<unsigned int> Hmap3DVizualizer::makeIndexBuffer()
{
    //export the faces
    std::vector<unsigned int> indices;
    for (unsigned int y = 0; y < _hmap->_height - 1; y++)
    {
        for (unsigned int x = 0; x < _hmap->_width - 1; x++)
        {
            indices.insert(indices.cend(), {x + y * _hmap->_width, x + (y + 1) * _hmap->_width, x + 1 + y * _hmap->_width });
            indices.insert(indices.cend(), {x + 1 + y * _hmap->_width , x + (y + 1) * _hmap->_width,  x + 1 + (y + 1) * _hmap->_width});
        }
    }
    return indices;
}


void Hmap3DVizualizer::addParameter(const std::string& name, int* parameter, const int& minValue, const int& maxValue)
{
    _parameters.emplace_back(name, Parameter::TYPE::INT, parameter, minValue, maxValue);
}

void Hmap3DVizualizer::addParameter(const std::string& name, float* parameter, const float& minValue, const float& maxValue)
{
    _parameters.emplace_back(name, Parameter::TYPE::FLOAT, parameter, minValue, maxValue);
}

void Hmap3DVizualizer::showHmap()
{
}



