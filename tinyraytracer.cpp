#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>
# define M_PI 3.14159265358979323846

#include "geometry.h"

struct Light
{
  Light(const Vec3f &p, const float &i): position(p), intensity(i) {}
  Vec3f position;
  float intensity;
};

struct Material
{
  Material(const Vec3f &color): diffuse_color(color) {}
  Material(): diffuse_color() {}
  Vec3f diffuse_color;
};

struct Sphere
{
  Vec3f center;
  float radius;
  Material material;
  
  Sphere(const Vec3f &c, const float &r, const Material &m)
    : center(c), radius(r), material(m) {}

  bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const
  {
    Vec3f L = center - orig;
    float tca = L * dir;
    float d2 = L * L - tca * tca;
    if (d2 > radius * radius) return false;
    float thc = sqrtf(radius * radius - d2);
    t0 = tca - thc;
    float t1 = tca + thc;
    if (t0 < 0) t0 = t1;
    if (t0 < 0) return false;
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
  float spheres_dist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < spheres.size(); i++)
    {
      float dist_i;
      if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist)
	{
	  spheres_dist = dist_i;
	  hit = orig + dir * dist_i;
	  N = (hit - spheres[i].center).normalize();
	  material = spheres[i].material;
	}
    }
  return spheres_dist < 1000;
}

Vec3f cast_ray(const Vec3f &orig,
	       const Vec3f &dir,
	       const std::vector<Sphere> &spheres,
	       const std::vector <Light> &lights)
{
  Vec3f point, N;
  Material material;

  if(!scene_intersect(orig, dir, spheres, point, N, material))
    {
      return Vec3f(1, 1, 1);
    }

  float diffuse_light_intensity = 0;
  for (size_t i = 0; i < lights.size(); i++)
    {
      Vec3f light_dir = (lights[i].position - point).normalize();
      diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * N);
    }

  return material.diffuse_color * diffuse_light_intensity;
}

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights)
{
  auto aspect_ratio = 16.0 / 9.0;
  const int width = 3000;
  const int height = int(width / aspect_ratio);
  
  const int fov = M_PI / 2.;
  std::vector<Vec3f> framebuffer(width * height);

  #pragma omp parallel for
  for(size_t j = 0; j < height; j++)
    {
      for(size_t i = 0; i < width; i++)
	{
	  float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.) * width / (float)height;
	  float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.);
	  Vec3f dir = Vec3f(x, y, -1).normalize();
	  framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
	}
    }

  std::ofstream ofs;
  ofs.open("./out.ppm", std::ofstream::out | std::ofstream::binary);
  ofs << "P6\n" << width << " " << height << "\n255\n";
  for(size_t i = 0; i < height * width; ++i)
    {
      for(size_t j = 0; j < 3; j++)
	{
	  ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
	}
    }
  ofs.close();
}

int main()
{ 
  Material nature_green1(Vec3f(0.275, 0.522, 0.522));
  Material nature_green2(Vec3f(0.314, 0.706, 0.596));
  Material nature_green3(Vec3f(0.612, 0.859, 0.651));
  Material nature_green4(Vec3f(0.871, 0.976, 0.769));

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-3, 0, -16), 4, nature_green1));
  spheres.push_back(Sphere(Vec3f(-8, -1.5, -12), 3.5, nature_green2));
  spheres.push_back(Sphere(Vec3f(1.5, -0.5, -14), 3, nature_green3));
  spheres.push_back(Sphere(Vec3f(7, 5, -18), 2, nature_green4));

  std::vector<Light> lights;
  lights.push_back(Light(Vec3f(-20, 20, 20), 1));
  
  render(spheres, lights);
}
