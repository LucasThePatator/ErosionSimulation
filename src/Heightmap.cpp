#include "Heightmap.h"

namespace ErosionSimulation
{
	Heightmap::Heightmap(unsigned int width, unsigned int height) :
		_width(width),
		_height(height)
	{
		create(width, height);
	}

	Heightmap::Heightmap(const Heightmap& other) :
		_data(other._data),
		refCount(other.refCount)
	{
		(*refCount)++;
	}

	Heightmap::Heightmap(const Heightmap&& other) noexcept :
		_data(other._data),
		refCount(other.refCount)
	{
		(*refCount)++;
	}

	Heightmap& Heightmap::operator=(const Heightmap& other)
	{
		_data = other._data;
		refCount = other.refCount;
		(*refCount)++;
		return *this;
	}

	Heightmap& Heightmap::operator=(const Heightmap&& other)
	{
		_data = other._data;
		refCount = other.refCount;
		(*refCount)++;
		return *this;
	}

	float Heightmap::operator[](unsigned int index) const
	{
		return _data[index];
	}

	float& Heightmap::at(unsigned int x, unsigned int y)
	{
		return _data[y * _width + x];
	}

	const float& Heightmap::at(unsigned int x, unsigned int y) const
	{
		return _data[y * _width + x];
	}

	Heightmap::~Heightmap()
	{
		(*refCount)--;
		if (*refCount == 0)
		{
			delete[] _data;
			delete refCount;
		}
	}

	void Heightmap::create(unsigned int width, unsigned int height)
	{
		if (width > 0 and height > 0)
		{
			_data = new float[width * height] {};
			refCount = new unsigned int{};
			(*refCount) = 1;
		}
	}

	std::vector<float> Heightmap::computeGradient() const
	{
		auto gradient = std::vector<float>(_width * _height * 2, 0.f);

		#pragma omp parallel for
		for (int y = 0; y < _height - 1; y++)
		{
			for (unsigned int x = 0; x < _width - 1; x++)
			{
				auto index = x + y * _width;

				gradient[2 * index] = _data[index + 1] - _data[index];
				gradient[2 * index + 1] = _data[index + _width] - _data[index];
			}
			gradient[2 * (y + 1) * _width - 2] = _data[(y + 1) * _width - 1] - _data[(y + 1) * _width - 2];
			gradient[2 * (y + 1) * _width - 1] = _data[(y + 2) * _width - 1] - _data[(y + 1) * _width - 1];
		}

		#pragma omp parallel for
		for (int x = 0; x < _width - 1; x++)
		{
			gradient[2 * ((_height - 1) * (_width) + x)] = _data[(_height - 2) * (_width) + x + 1] - _data[(_height - 2) * (_width) + x];
			gradient[2 * ((_height - 1) * (_width) + x) + 1] = _data[(_height - 1) * (_width) + x] - _data[(_height - 2) * (_width) + x];
		}

		gradient[2 * _height * _width - 2] = gradient[2 * _height * _width - 4];
		gradient[2 * _height * _width - 1] = gradient[2 * (_height - 1) * _width - 1];

		return gradient;
	}

	Heightmap& Heightmap::operator*=(float value)
	{
#pragma omp parallel for
		for (int y = 0; y < _height; y++)
		{
			for (unsigned int x = 0; x < _width; x++)
			{
				_data[x + _width * y] = _data[x + _width * y] * value;
			}
		}
		return *this;
	}


	Heightmap& Heightmap::operator/=(float value)
	{
#pragma omp parallel for
		for (int y = 0; y < _height; y++)
		{
			for (unsigned int x = 0; x < _width; x++)
			{
				_data[x + _width * y] = _data[x + _width * y] / value;
			}
		}
		return *this;
	}


	Heightmap& Heightmap::operator+=(float value)
	{
#pragma omp parallel for
		for (int y = 0; y < _height; y++)
		{
			for (unsigned int x = 0; x < _width; x++)
			{
				_data[x + _width * y] = _data[x + _width * y] + value;
			}
		}
		return *this;
	}


	Heightmap& Heightmap::operator-=(float value)
	{
#pragma omp parallel for
		for (int y = 0; y < _height; y++)
		{
			for (unsigned int x = 0; x < _width; x++)
			{
				_data[x + _width * y] = _data[x + _width * y] - value;
			}
		}
		return *this;
	}
}