#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <vector>

namespace ErosionSimulation
{
	struct point2f
	{
		float x;
		float y;
	};

	struct Heightmap
	{
		Heightmap() = delete;
		Heightmap(unsigned int width, unsigned int height);
		Heightmap(const Heightmap& other);
		Heightmap(const Heightmap&& other) noexcept;
		Heightmap& operator=(const Heightmap& other);
		Heightmap& operator=(const Heightmap&& other);
		Heightmap& operator*=(float value);
		Heightmap& operator/=(float value);
		Heightmap& operator+=(float value);
		Heightmap& operator-=(float value);

		float operator[](unsigned int) const;
		float& at(unsigned int x, unsigned int y);
		const float& at(unsigned int x, unsigned int y) const;

		std::vector<float> computeGradient() const;

		~Heightmap();

		void create(unsigned int width, unsigned int height);
		unsigned int _width = 0;
		unsigned int _height = 0;

		float* _data = nullptr;

		unsigned int* refCount = nullptr;
	};
}

#endif // !HEIGHTMAP_H