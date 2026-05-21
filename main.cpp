#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "renderer.hpp"
#include "scene_config.hpp"
#include "scene_types.hpp"

template <class FLOAT>
int WritePPM(const char *file_name, int columns, int rows, FLOAT *data);

namespace renderlab
{
static void ParseArgs(int argc, char **argv, RenderSettings &s)
  {
  for (int i = 1; i < argc; i++)
    {
    if (std::strcmp(argv[i], "--width") == 0 && i + 1 < argc)
      s.width = std::atoi(argv[++i]);
    else if (std::strcmp(argv[i], "--height") == 0 && i + 1 < argc)
      s.height = std::atoi(argv[++i]);
    else if (std::strcmp(argv[i], "--ppm") == 0 && i + 1 < argc)
      s.ppm_file = argv[++i];
    else if (std::strcmp(argv[i], "--nit") == 0 && i + 1 < argc)
      {
      s.nit_file = argv[++i];
      s.save_nit = true;
      }
    else if (std::strcmp(argv[i], "--no-nit") == 0)
      s.save_nit = false;
    else if (std::strcmp(argv[i], "--fov") == 0 && i + 1 < argc)
      s.fov_y_deg = static_cast<float>(std::atof(argv[++i]));
    }
  if (s.width < 500)
    s.width = 500;
  if (s.height < 500)
    s.height = 500;
  if (s.width > 1000)
    s.width = 1000;
  if (s.height > 1000)
    s.height = 1000;
  }
}

int main(int argc, char **argv)
  {
  renderlab::RenderSettings settings = {800, 800, "result.ppm", "result.nit", false, 45.f};
  renderlab::ParseArgs(argc, argv, settings);

  std::printf("Rendering %dx%d image to %s\n", settings.width, settings.height, settings.ppm_file);

  renderlab::SceneDefinition scene = renderlab::MakeDefaultScene();
  std::vector<double> ppm_data;
  if (!renderlab::RenderScene(scene, settings, ppm_data))
    return 1;

  if (WritePPM(settings.ppm_file, settings.width, settings.height, ppm_data.data()) != 0)
    std::printf("Failed to write PPM image %s\n", settings.ppm_file);
  else
    std::printf("Saved PPM image to %s\n", settings.ppm_file);

  if (settings.save_nit)
    {
    std::printf("NIT output is not available in this standalone build.\n");
    std::printf("The original implementation depended on proprietary course libraries that were removed.\n");
    }

  return 0;
  }
