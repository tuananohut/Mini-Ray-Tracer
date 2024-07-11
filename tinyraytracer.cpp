#define _USE_MATH_DEFINES
#include <limits>
#include <cmath>
#include <fstream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

#include "geometry.h"



void render()
{
  auto aspect_ratio = 16.0 / 9.0;
  const int width = 1024;
  const int height = int(width / aspect_ratio);
  std::vector<Vec3f> canvas(width * height);

  for (size_t j = 0; j < height; j++)
    {
      for (size_t i = 0; i < width; i++)
	{
	  canvas[i + j * width] = Vec3f(j / float(height), i / float(width), 0);
	}
    }
  
  std::vector<unsigned char> pixmap(width * height * 3);
  for (size_t i = 0; i < height * width; ++i)
    {
      Vec3f &c = canvas[i];
      float max = std::max(c[0], std::max(c[1], c[2]));
      if (max > 1) c = c * (1. / max);

      for (size_t j = 0; j < 3; j++)
	{
	  pixmap[i * 3 + j] = (unsigned char)(255 * std::max(0.f, std::min(1.f, canvas[i][j])));
	}
    }
    
  stbi_write_jpg("./out.jpg", width, height, 3, pixmap.data(), 100);

}

int main()
{f
  render();
}
