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

struct Material
{
  Material(const Vec3f &color):
    diffuse_color(color) {}
  Material():
    diffuse_color(){}

  Vec3f diffuse_color;
};
  
struct Sphere
{
  Vec3f center;
  float radius;
  Material material;
  
  Sphere(const Vec3f &c, const float &r, const Material &m):
    center(c), radius(r), material(m) {}
  
  bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const
  {
    Vec3f L = center - orig;
    float tca = L * dir;
    float d2 = L * L - tca * tca;

    if (d2 > radius * radius)
      return false;

    float thc = sqrtf(radius * radius - d2);
    t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 < 0)
      t0 = t1;

    if (t0 < 0)
      return false;

    return true;
  }
};

 bool scene_intersect(const Vec3f &orig,
		     const Vec3f &dir,
		     const std::vector<Sphere> &spheres,
		     Vec3f &hit,
		     Vec3f &N,
		     Material &material)
{
  float sphere_dist = std::numeric_limits<float>::max();

  for (size_t i = 0; i < spheres.size(); i++)
    {
      float dist_i;

      if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < sphere_dist)
	{
	  sphere_dist = dist_i;
	  hit = orig + dir * dist_i;
	  N = (hit - spheres[i].center).normalize();
	  material = spheres[i].material;
	}
    }
  
  return sphere_dist < 1000;
}


Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres)
{
  Vec3f point, N;
  Material material;

  if (!scene_intersect(orig, dir, spheres, point, N, material))
    {
      return Vec3f(1.f, 1.f, 1.f);
    }

  return material.diffuse_color;
}

void render(const std::vector<Sphere> &spheres)
{
  auto aspect_ratio = 16.0 / 9.0;
  const int width = 800;
  const int height = int(width / aspect_ratio);
  std::vector<Vec3f> canvas(width * height);
  const int fov = M_PI / 2.f; // field of view

  #pragma omp parallel for 
  for (size_t j = 0; j < height; j++)
    {
      for (size_t i = 0; i < width; i++)
	{
	  float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.) * width / (float)height;
	  float y = -(2 * (j + 0.5) / (float) height - 1) * tan(fov / 2.);
	  Vec3f dir = Vec3f(x, y, -1).normalize();
	  canvas[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, spheres);
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
{
  Material purple_1(Vec3f(0.353f, 0.388f, 0.612f));
  Material purple_2(Vec3f(0.467f, 0.463f, 0.702f));
  Material purple_3(Vec3f(0.608f, 0.525f, 0.741f));
  Material purple_4(Vec3f(0.886f, 0.733f, 0.914f));

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-3.f, 0.f, -16.f), 2, purple_4));
  spheres.push_back(Sphere(Vec3f(-1.0f, -1.5f, -12.f), 2, purple_2));
  spheres.push_back(Sphere(Vec3f(1.5f, -0.5f, -18.f), 3, purple_1));
  spheres.push_back(Sphere(Vec3f(7.f, 5.f, -18.f), 4, purple_3));
  
  render(spheres);
}
