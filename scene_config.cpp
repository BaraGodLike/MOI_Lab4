#include "scene_config.hpp"

namespace renderlab
{
SceneDefinition MakeDefaultScene()
  {
  SceneDefinition scene;

  // ---------------------------------------------------------------------------
  // CAMERA
  // First point: where the camera is.
  // Second point: where the camera looks.
  // Third vector: which direction is "up".
  // ---------------------------------------------------------------------------
  scene.camera = {
      Pf(2.05f, 0.74f, 0.70f),
      Pf(0.74f, 0.63f, 0.42f),
      Vf(0.0f, 0.0f, 1.0f),
  };

  // ---------------------------------------------------------------------------
  // BOXES
  // Add more entries to create more boxes.
  //
  // For each box:
  // 1. Pf(x, y, z)        -> box origin / position
  // 2. Vf(x, y, z)        -> box size
  // 3. OMIT_*             -> which wall to remove
  // 4. Matrix43f(...)     -> scale transform
  // 5. {diffuse, specular, shininess} -> material
  // ---------------------------------------------------------------------------
  scene.boxes = {
      {
          Pf(0.0f, 0.0f, 0.0f),
          Vf(1.40f, 1.30f, 1.00f),
          OMIT_X_POS,
          Matrix43f(1.0f, 1.0f, 1.0f),
          {Vf(0.82f, 0.84f, 0.88f), Vf(0.10f, 0.10f, 0.10f), 10.0f},
      },
      {
          Pf(1.2f, 0.18f, 0.00f),
          Vf(0.24f, 0.24f, 0.34f),
          OMIT_NONE,
          Matrix43f(1.0f, 1.0f, 1.0f),
          {Vf(0.16f, 0.62f, 0.26f), Vf(0.15f, 0.18f, 0.15f), 18.0f},
      },
      {
          Pf(1.00f, 0.20f, 0.00f),
          Vf(0.18f, 0.30f, 0.60f),
          OMIT_NONE,
          Matrix43f(1.0f, 1.0f, 1.0f),
          {Vf(0.85f, 0.56f, 0.18f), Vf(0.20f, 0.14f, 0.10f), 22.0f},
      },
      {
          Pf(0.58f, 0.92f, 0.00f),
          Vf(0.38f, 0.14f, 0.20f),
          OMIT_NONE,
          Matrix43f(1.0f, 1.0f, 1.0f),
          {Vf(0.18f, 0.42f, 0.82f), Vf(0.15f, 0.18f, 0.22f), 14.0f},
      },
  };

  // ---------------------------------------------------------------------------
  // SPHERES
  // Add more entries to create more spheres.
  //
  // For each sphere:
  // 1. Pf(x, y, z)        -> sphere center
  // 2. radius             -> sphere radius
  // 3. depth              -> mesh detail level
  // 4. Matrix43f(...)     -> scale transform
  // 5. {diffuse, specular, shininess} -> material
  // ---------------------------------------------------------------------------
  scene.spheres = {
      {
          Pf(0.66f, 0.52f, 0.29f),
          0.28f,
          4,
          Matrix43f(1.0f, 1.0f, 1.0f),
          {Vf(0.86f, 0.22f, 0.24f), Vf(0.95f, 0.95f, 0.95f), 72.0f},
      },
      {
          Pf(1.05f, 0.86f, 0.16f),
          0.16f,
          4,
          Matrix43f(1.0f, 1.0f, 1.0f),
          {Vf(0.96f, 0.84f, 0.22f), Vf(0.95f, 0.91f, 0.72f), 52.0f},
      },
  };

  // ---------------------------------------------------------------------------
  // LIGHTS
  // Add more entries to create more light sources.
  //
  // POINT LIGHT:
  // {LIGHT_POINT, color, position, unused_direction, intensity}
  //
  // DIRECTIONAL LIGHT:
  // {LIGHT_DIRECTIONAL, color, unused_position, direction, intensity}
  // ---------------------------------------------------------------------------
  scene.lights = {
      {LIGHT_POINT, Vf(1.00f, 0.94f, 0.88f), Pf(1.2f, 0.22f, 0.92f), Vf(0.0f, 0.0f, 0.0f), 5.2f},
      {LIGHT_POINT, Vf(0.56f, 0.66f, 0.95f), Pf(0.24f, 1.08f, 0.76f), Vf(0.0f, 0.0f, 0.0f), 4.2f},
      {LIGHT_DIRECTIONAL, Vf(0.32f, 0.36f, 0.52f), Pf(0.0f, 0.0f, 0.0f), Normalize(Vf(-1.0f, 0.3f, -0.7f)), 0.4f},
  };

  // ---------------------------------------------------------------------------
  // QUICK EDIT CHEAT SHEET
  //
  // Move camera:
  //   change scene.camera first Pf(...)
  //
  // Change where camera looks:
  //   change scene.camera second Pf(...)
  //
  // Add a sphere:
  //   copy one block inside scene.spheres and change Pf(...) and radius
  //
  // Add a box:
  //   copy one block inside scene.boxes and change Pf(...) and size
  //
  // Move a point light:
  //   change Pf(...) in LIGHT_POINT
  //
  // Change light brightness:
  //   change the last number in the light entry
  //
  // Change object color:
  //   change the first Vf(...) inside the material
  // ---------------------------------------------------------------------------

  return scene;
  }
}
