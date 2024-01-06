#pragma once

#include <FastNoise/FastNoise.h>
#include <memory>
#include <random>
#include <array>
#include "Heightmap.h"


namespace ErosionSimulation
{
	class ErosionGenerator {
	public:
		struct Config
		{
			float gravity = 10.0f;
			float friction = 0.95f;
			int maxDropletSteps = 256;
			float evaporation = 0.95f;
			float erosionFactor = 10000.f;
			float erosionRadius = 7.f;
			float minSlope = 0.001f;
			float capacityFactor = 256.f;
			float depositFactor = 0.01f;
			float inertia = 0.1f;
		} _config;

		ErosionGenerator();
		ErosionGenerator(const Config& config);
		ErosionGenerator(const Config&& config);

		Heightmap generateNoisyTerrain(unsigned int width, unsigned int height, float MaxValue);

		std::vector<point2f> launchDroplet(Heightmap &hmap);
		std::array<float, 2> computeGradient(const Heightmap& hmap, const point2f point);

	private:
		FastNoise::SmartNode<FastNoise::OpenSimplex2S> _generator;
		std::random_device _rng;
		std::default_random_engine _rn_engine;

		bool _debug = false;

	};
}

