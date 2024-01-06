
#include <string>
#include <vector>
#include <variant>
#include <functional>
#include "Heightmap.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Hmap3DVizualizer
{
public:
	Hmap3DVizualizer(int display_w, int display_h, bool debug = false);
	~Hmap3DVizualizer();

	void init(const ErosionSimulation::Heightmap *hmap, const std::vector<std::vector<ErosionSimulation::point2f>> *trajs);
	void run();
	void showHmap();

	void addParameter(const std::string& name, int* parameter, const int& minValue, const int& maxValue);
	void addParameter(const std::string& name, float* parameter, const float& minValue, const float& maxValue);

	void setOnNew(std::function<void(void)> onNew) { _onNew = onNew; }
	void setOnRun(std::function<void(void)> onRun) { _onRun = onRun; }


private:
	bool _debug;
	int _display_w, _display_h;
	GLFWwindow* _window;

	struct Parameter
	{
		std::string name;
		enum class TYPE
		{
			INT,
			FLOAT
		} type;

		void* pointer;
		std::variant<int, float> min;
		std::variant<int, float> max;
	};

	std::function<void(void)> _onNew;
	std::function<void(void)> _onRun;

	std::vector<Parameter> _parameters;
	void renderUI();
	void renderSlider(const Parameter& parameter);

	const ErosionSimulation::Heightmap* _hmap;
	const std::vector<std::vector<ErosionSimulation::point2f>>* _trajs;

	std::vector<unsigned int> makeIndexBuffer();
	std::vector<float> makeVertexBuffer();
	std::vector<float> makeUVBuffer();
	std::vector<float> makeNormalsBuffer();

	float _cameraAzimut = 0.f;
	float _cameraElevation = 45.f;
	float _cameraDistance = 350.f;
	float _cameraFoV = 90.f;

	float _sunAzimut = 90.f;
	float _sunElevation = 45.f;

	GLuint vertexArrayID;
	GLuint vertexbufferID;
	GLuint normalsbufferID;
	GLuint uvbufferID;
	GLuint elementbufferID;

	GLuint programID;
};
