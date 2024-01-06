#include "ErosionGenerator.h"
#include <iostream>
#include "Heightmap.h"

namespace ErosionSimulation
{
	template<int channels>
	std::array<float, channels> bilinearInterp(const float* data, unsigned int width, unsigned int height, const point2f &point)
	{
		if (point.x < 0 || point.x >= width || point.y < 0 || point.y >= height)
 			return {};

		point2f current_point = point;

		if (current_point.x > width - 1)
			current_point.x = width - 1 - 0.0001f;

		if (current_point.y > height - 1)
			current_point.y = height - 1 - 0.0001f;

		const unsigned int x_left = int(current_point.x);
		const unsigned int x_right = x_left + 1;

		const unsigned int y_top = int(current_point.y);
		const unsigned int y_bottom = y_top + 1;

		float x_remain = current_point.x - x_left;
		float y_remain = current_point.y - y_top;

		std::array<float, channels> ret;
		for (unsigned int c = 0; c < channels; c++)
		{
			float top_value = data[channels * (y_top * width + x_left) + c] * (1 - x_remain) + data[channels * (y_top * width + x_right) + c] * x_remain;
			float bottom_value = data[channels * (y_bottom * width + x_left) + c] * (1 - x_remain) + data[channels * (y_bottom * width + x_right) + c] * x_remain;

			float value = top_value * (1 - y_remain) + bottom_value * y_remain;
			ret[c] = value;
		}

		return ret;
	}

	ErosionGenerator::ErosionGenerator() :
		_generator(FastNoise::New<FastNoise::OpenSimplex2S>()),
		_rn_engine(_rng())
	{
	}

	ErosionGenerator::ErosionGenerator(const Config& config) :
		_generator(FastNoise::New<FastNoise::OpenSimplex2S>()),
		_rn_engine(_rng()),
		_config(config)
	{
	}

	ErosionGenerator::ErosionGenerator(const Config&& config) :
		_generator(FastNoise::New<FastNoise::OpenSimplex2S>()),
		_rn_engine(_rng()),
		_config(config)
	{
	}

	Heightmap ErosionGenerator::generateNoisyTerrain(unsigned int width, unsigned int height, float maxValue)
	{
		Heightmap _hmap(width, height);
		auto minMax = _generator->GenUniformGrid2D(_hmap._data, 0, 0, _hmap._width, _hmap._height, 0.003f, 0);
		_hmap -= minMax.min;
		_hmap *= maxValue * (minMax.max - minMax.min);
		return  _hmap;
	}

	float applyErosion(Heightmap& hmap, const point2f &point, float radius, float weight)
	{
		const auto Rsquared = radius * radius;
		const auto area = 3.14f * Rsquared;
		const auto norm_factor = 0.33f * area; //volume of cone
		float total_sediment = 0.f;
		for (unsigned int x = static_cast<unsigned int>(point.x - radius); x <= static_cast<unsigned int>(point.x + radius); x++)
		{
			if (x < 0 || x >= hmap._width)
				continue;

			const auto x_diff = (x - point.x);
			const auto x_diff_squared = x_diff * x_diff;
			const auto min_y = point.y - std::sqrt(Rsquared - x_diff_squared);
			const auto max_y = point.y + std::sqrt(Rsquared - x_diff_squared);
			for (unsigned int y = static_cast<unsigned int>(min_y); static_cast<unsigned int>(y) <= max_y; y++)
			{
				if (y < 0 || y >= hmap._height)
					continue;

				const auto y_diff = (y - point.y);
				const auto y_diff_squared = y_diff * y_diff;
				auto distance = std::sqrt(y_diff_squared + x_diff_squared);
				auto erosionWeight = std::abs(radius - distance) * weight / (radius * norm_factor) ;

				auto erosionValue = erosionWeight;
				total_sediment += erosionValue;
 				hmap.at(x, y) -= erosionValue;
			}
		}
		return total_sediment;
	}

	float deposit(Heightmap& hmap, const point2f& point, float weight, float max)
	{	
		if (weight < 1e-5)
			return 0.f;

		float deposited = 0.f;
		const int x = static_cast<unsigned int>(point.x);
		const int y = static_cast<unsigned int>(point.y);

		const float x_remain = point.x - x - 0.5;
		const float y_remain = point.y - y - 0.5;

		float value = std::min(weight * (1 - std::abs(x_remain)) * (1 - std::abs(y_remain)), max);
		deposited += value;
		hmap.at(x, y) += value;
		if (x + 1 < hmap._width)
		{
			value = std::min(weight * std::max(0.f, x_remain) * (1 - std::abs(y_remain)), max);
			deposited += value;
			hmap.at(x + 1, y) += value;
		}
		if (x - 1 >= 0)
		{
			value = std::min(weight * std::max(0.f, -x_remain) * (1 - std::abs(y_remain)), max);
			deposited += value;
			hmap.at(x - 1, y) += value;
		}
		if (y + 1 < hmap._height)
		{
			value = std::min(weight * (1 - std::abs(x_remain)) * std::max(0.f, y_remain), max);
			deposited += value;
			hmap.at(x, y + 1) += value;
		}

		if (y - 1 >= 0)
		{
			value = std::min(weight * (1 - std::abs(x_remain)) * std::max(0.f, -y_remain), max);
			deposited += value;
			hmap.at(x, y - 1) += value;
		}
		return deposited;
	}

	std::vector<point2f> ErosionGenerator::launchDroplet(Heightmap &hmap)
	{
		if (_debug)
			std::cout << "New droplet" << std::endl;
		const auto width = hmap._width;
		const auto height = hmap._height;

		std::uniform_real_distribution<float> dist_x(0, static_cast<float>(width));
		std::uniform_real_distribution<float> dist_y(0, static_cast<float>(height));

		std::uniform_real_distribution<float> grad_x_dist(-1, 1);
		std::uniform_real_distribution<float> grad_y_dist(-1, 1);

		point2f currentPoint = { dist_x(_rn_engine), dist_y(_rn_engine) };

		float speed_x{}, speed_y{};
		bool stop = false;

		float sediments = 0.f;
		float volume = 1.0f;
		float speed = 0.f;

		float dir_x{}, dir_y{};

		std::vector<point2f> trajectory = {currentPoint};
		trajectory.reserve(static_cast<size_t>(_config.maxDropletSteps));

		for(unsigned int step = 0; step < _config.maxDropletSteps; step++)
		{
			const auto local_height = bilinearInterp<1>(hmap._data, width, height, currentPoint);
			const auto local_gradient = computeGradient(hmap, currentPoint);
			
			const auto grad_norm = std::sqrt(local_gradient[0] * local_gradient[0] + local_gradient[1] * local_gradient[1]);
			float new_dir_x, new_dir_y = 0.f;
			if (grad_norm == 0.f)
			{
				new_dir_x = grad_x_dist(_rn_engine);
				new_dir_y = grad_y_dist(_rn_engine);
			}
			else
			{
				new_dir_x = -local_gradient[0];
				new_dir_y = -local_gradient[1];
			}

			dir_x = _config.inertia * dir_x + (1 - _config.inertia) * new_dir_x / grad_norm;
			dir_y = _config.inertia * dir_y + (1 - _config.inertia) * new_dir_y / grad_norm;

			const auto dir_norm = std::sqrt(dir_x * dir_x + dir_y * dir_y);

			dir_x /= dir_norm;
			dir_y /= dir_norm;

			point2f newPoint = currentPoint;
			newPoint.x += dir_x;
			newPoint.y += dir_y;

			if (newPoint.x < 0 || newPoint.x >= width || newPoint.y < 0 || newPoint.y >= height)
				break;

			trajectory.push_back(newPoint);

			const auto new_height = bilinearInterp<1>(hmap._data, width, height, newPoint);
			const auto hdiff = new_height[0] - local_height[0];

			if (_debug)
				std::cout << "speed : " << speed << "; height diff : " << hdiff << std::endl;

			if (hdiff < 0)
			{
				float capacity = std::max(-hdiff, _config.minSlope) * speed * volume * _config.capacityFactor;

				if (_debug)
					std::cout << "capacity" << capacity << std::endl;

				if (capacity > sediments)
				{
					const auto erosionFactor = std::min((capacity - sediments) * _config.erosionFactor, -hdiff);
					if (_debug)
						std::cout << "erosion factor " << erosionFactor << std::endl;
					sediments += applyErosion(hmap, currentPoint, _config.erosionRadius, erosionFactor);
				}
				else
				{
					const auto sedimentsToDeposit = _config.depositFactor * (sediments - capacity);
					if (_debug)
						std::cout << "sedimentsToDeposit " << sedimentsToDeposit << std::endl;
					const auto deposited = deposit(hmap, currentPoint, sedimentsToDeposit, -hdiff);
					sediments -= deposited;
				}
			}
			else
			{
				if (hdiff == 0.f)
					break;
				const auto sedimentsToDeposit = sediments;

				if (_debug)
					std::cout << "sedimentsToDeposit " << sedimentsToDeposit << std::endl;

				const auto deposited = deposit(hmap, currentPoint, sedimentsToDeposit, hdiff);
				sediments -= deposited;
				if (sediments == 0.f || deposited < 1e-5)
					break;
			}

			speed = std::sqrt(std::max(0.f, speed * speed - hdiff * _config.gravity));
			volume *= _config.evaporation;
			currentPoint = newPoint;
			if (volume < 1e-3)
				break;


		}
		return trajectory;

	}

	std::array<float, 2> ErosionGenerator::computeGradient(const Heightmap& hmap, const point2f point)
	{
		std::array<float, 2> ret;
		float local_value = bilinearInterp<1>(hmap._data, hmap._width, hmap._height, point)[0];

		point2f pointx = { point.x + 1.f, point.y };
		ret[0] = bilinearInterp<1>(hmap._data, hmap._width, hmap._height, pointx)[0] - local_value;

		point2f pointy = { point.x, point.y + 1.f };
		ret[1] = bilinearInterp<1>(hmap._data, hmap._width, hmap._height, pointy)[0] - local_value;

		return ret;
	}
}
