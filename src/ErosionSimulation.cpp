// ErosionSimulation.cpp : définit le point d'entrée de l'application.
//

#include "ErosionSimulation.h"
#include "ErosionGenerator.h"

#include "Hmap3DVisualizer.h"

#include <fstream>
#include <chrono>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <future>

using namespace std;
using namespace ErosionSimulation;
using namespace cv;

void plotTraj(cv::Mat image, const std::vector<point2f>& traj)
{
	cv::Mat plotImage;
	cv::cvtColor(image, plotImage, cv::COLOR_GRAY2BGR);
	const auto trajLength = traj.size();
	for (unsigned int i = 0; i < trajLength; i++)
	{
		auto colorWeight = float(i) / trajLength;
		cv::Scalar color(colorWeight, 0., 1 - colorWeight);
		cv::drawMarker(plotImage, { int(traj[i].x), int(traj[i].y) }, color, 0, 3);
	}

	cv::imshow("traj", plotImage);
}

void exportObj(const Heightmap& hmap, std::ofstream& file)
{
	file << "o terrain\n\n";

	//export all the vertices
	for (unsigned int y = 0; y < hmap._height; y++)
	{
		for (unsigned int x = 0; x < hmap._width; x++)
		{
			file << "v " << static_cast<float>(x) << " " << static_cast<float>(y) << " " << hmap.at(x, y) * 10 << "\n";
		}
	}

	file << '\n';

	auto gradient = hmap.computeGradient();
	//export the normals
	for (unsigned int y = 0; y < hmap._height; y++)
	{
		for (unsigned int x = 0; x < hmap._width; x++)
		{
			file << "vn " << -gradient[2 * (x + y * hmap._width)] << " " << -gradient[2 * (x + y * hmap._width) + 1] << " 1.0\n"; //Approximate the tan to compute quicker
		}
	}

	file << '\n';

	//export the texture coodinates
	for (unsigned int y = 0; y < hmap._height; y++)
	{
		for (unsigned int x = 0; x < hmap._width; x++)
		{
			file << "vt " << static_cast<float>(x) / hmap._width << " " << static_cast<float>(y) / hmap._height << "\n";
		}
	}

	file << '\n';

	//export the faces
	for (unsigned int y = 0; y < hmap._height - 1; y++)
	{
		for (unsigned int x = 0; x < hmap._width - 1; x++)
		{
			std::ostringstream oss1;
			oss1 << x + y * hmap._width + 1 << " " << x + (y + 1) * hmap._width + 1 << " " << x + 1 + y * hmap._width + 1;
			file << "f " << oss1.str() << "\n";

			std::ostringstream oss2;
			oss2 << x + 1 + y * hmap._width + 1 << " " << x + (y + 1) * hmap._width + 1 << " " << x + 1 + (y + 1) * hmap._width + 1;
			file << "f " << oss2.str() << "\n";

		}
	}

	file << std::endl;
}

int main()
{
	ErosionGenerator erosionGenerator{};
	Heightmap hmap(256, 256);
	std::vector<std::vector<point2f>> trajs;

	std::atomic<bool> run = false;

	Hmap3DVizualizer hmapViz(1024, 768, true);
	hmapViz.init(&hmap, &trajs);
	int steps = 1;

	hmapViz.addParameter("gravity", &erosionGenerator._config.gravity, 0.f, 90.f);
	hmapViz.addParameter("friction", &erosionGenerator._config.friction, 0.f, 1.f);
	hmapViz.addParameter("maxDropletSteps", &erosionGenerator._config.maxDropletSteps, 1, 512);
	hmapViz.addParameter("evaporation", &erosionGenerator._config.evaporation, 0.f, 1.f);
	hmapViz.addParameter("erosionFactor", &erosionGenerator._config.erosionFactor, 0.f, 1000.f);
	hmapViz.addParameter("erosionRadius", &erosionGenerator._config.erosionRadius, 0.f, 15.f);
	hmapViz.addParameter("minSlope", &erosionGenerator._config.minSlope, 0.f, 1.f);
	hmapViz.addParameter("capacityFactor", &erosionGenerator._config.capacityFactor, 0.f, 1000.f);
	hmapViz.addParameter("depositFactor", &erosionGenerator._config.depositFactor, 0.f, 1.f);
	hmapViz.addParameter("inertia", &erosionGenerator._config.inertia, 0.f, 1.f);
	hmapViz.addParameter("Steps 10^", &steps, 0, 10);


	hmapViz.setOnNew([&erosionGenerator, &hmap, &trajs]()
		{
			hmap = erosionGenerator.generateNoisyTerrain(256, 256, 75.f);
		});

	hmapViz.setOnRun([&erosionGenerator, &hmap, &steps, &trajs]()
		{
			const unsigned int maxSteps = 1U << steps;
			std::async(std::launch::async, [maxSteps, &erosionGenerator, &hmap, &trajs]()
				{
					for (int i = 0; i < maxSteps; i++)
					{
						const auto local_traj = erosionGenerator.launchDroplet(hmap);
						trajs.push_back(local_traj);
					}
				});
		});

	hmapViz.run();

}
