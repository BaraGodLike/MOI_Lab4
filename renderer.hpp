#pragma once

#include <vector>

#include "scene_types.hpp"

namespace renderlab
{
bool RenderScene(const SceneDefinition &scene,
                 const RenderSettings &settings,
                 std::vector<double> &ppm_data);
}
