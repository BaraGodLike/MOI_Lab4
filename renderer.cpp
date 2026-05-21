#include "renderer.hpp"

#include <embree3/rtcore.h>
#if defined(__has_include)
#  if __has_include(<embree3/rtcore_ray.h>)
#    include <embree3/rtcore_ray.h>
#  elif __has_include(<embree/rtcore_ray.h>)
#    include <embree/rtcore_ray.h>
#  endif
#else
#  include <embree3/rtcore_ray.h>
#endif

#include <cstdio>
#include <cstring>
#include <limits>

namespace renderlab
{
namespace
{
Point3f RayPoint(const RTCRay &ray, float t)
  {
  return Pf(ray.org_x + ray.dir_x * t, ray.org_y + ray.dir_y * t, ray.org_z + ray.dir_z * t);
  }

void InitRayHit(RTCRayHit &rayhit, const Point3f &origin, const Vect3f &dir)
  {
  std::memset(&rayhit, 0, sizeof(rayhit));
  rayhit.ray.org_x = origin.x;
  rayhit.ray.org_y = origin.y;
  rayhit.ray.org_z = origin.z;
  rayhit.ray.dir_x = dir.x;
  rayhit.ray.dir_y = dir.y;
  rayhit.ray.dir_z = dir.z;
  rayhit.ray.tnear = 0.001f;
  rayhit.ray.tfar = std::numeric_limits<float>::infinity();
  rayhit.ray.mask = 0xFFFFFFFF;
  rayhit.ray.flags = 0;
  rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
  rayhit.hit.primID = RTC_INVALID_GEOMETRY_ID;
  rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
  }

bool IsOccluded(RTCScene scene, const Point3f &origin, const Vect3f &dir, float max_distance)
  {
  RTCIntersectContext context;
  RTCRay shadow_ray;
  rtcInitIntersectContext(&context);
  std::memset(&shadow_ray, 0, sizeof(shadow_ray));
  shadow_ray.org_x = origin.x;
  shadow_ray.org_y = origin.y;
  shadow_ray.org_z = origin.z;
  shadow_ray.dir_x = dir.x;
  shadow_ray.dir_y = dir.y;
  shadow_ray.dir_z = dir.z;
  shadow_ray.tnear = 0.0f;
  shadow_ray.tfar = max_distance;
  shadow_ray.mask = 0xFFFFFFFF;
  shadow_ray.flags = 0;
  rtcOccluded1(scene, &context, &shadow_ray);
  return shadow_ray.tfar < 0.0f;
  }

Vect3f ShadeSurface(const RTCRayHit &rayhit,
                    RTCScene scene,
                    const std::vector<Material> &materials,
                    const std::vector<Light> &lights)
  {
  if (rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID || rayhit.hit.geomID >= materials.size())
    return Vf(kBackgroundR, kBackgroundG, kBackgroundB);

  const Material &material = materials[rayhit.hit.geomID];
  Point3f hit_point = RayPoint(rayhit.ray, rayhit.ray.tfar);
  Vect3f ray_dir = Normalize(Vf(rayhit.ray.dir_x, rayhit.ray.dir_y, rayhit.ray.dir_z));
  Vect3f normal = Normalize(Vf(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z));
  if (Dot(normal, ray_dir) > 0)
    normal = Scale(normal, -1.f);

  Vect3f view_dir = Scale(ray_dir, -1.f);
  Point3f shadow_origin = Pf(hit_point.x + normal.x * kShadowBias,
                             hit_point.y + normal.y * kShadowBias,
                             hit_point.z + normal.z * kShadowBias);
  Vect3f result = Vf(0, 0, 0);

  for (std::size_t i = 0; i < lights.size(); i++)
    {
    const Light &light = lights[i];
    Vect3f light_dir;
    float attenuation = light.intensity;
    float max_distance = std::numeric_limits<float>::infinity();

    if (light.type == LIGHT_POINT)
      {
      Vect3f to_light = Sub(light.position, hit_point);
      float distance = Length(to_light);
      if (distance <= kShadowBias)
        continue;
      light_dir = Scale(to_light, 1.f / distance);
      max_distance = distance - 2.f * kShadowBias;
      attenuation = light.intensity / (distance * distance);
      }
    else
      {
      light_dir = Normalize(Scale(light.direction, -1.f));
      }

    float n_dot_l = Dot(normal, light_dir);
    if (n_dot_l <= 0 || IsOccluded(scene, shadow_origin, light_dir, max_distance))
      continue;

    Vect3f diffuse = Scale(Mul(material.diffuse, light.color), attenuation * n_dot_l);
    Vect3f half_vector = Normalize(Add(light_dir, view_dir));
    float spec = std::pow(Sat(Dot(normal, half_vector)), material.shininess);
    result = Add(result, Add(diffuse, Scale(Mul(material.specular, light.color), attenuation * spec)));
    }

  return ClampPositive(result);
  }

void DeviceErrorFunction(void * /*userPtr*/, enum RTCError error, const char *str)
  {
  std::printf("Device error %d: %s\n", error, str);
  }

RTCGeometry CreateBoxGeometry(RTCDevice device, const BoxObject &box)
  {
  RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
  float *vertices = static_cast<float *>(rtcSetNewGeometryBuffer(
      geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), 8));
  unsigned *indices = static_cast<unsigned *>(rtcSetNewGeometryBuffer(
      geom,
      RTC_BUFFER_TYPE_INDEX,
      0,
      RTC_FORMAT_UINT3,
      3 * sizeof(unsigned),
      12));

  if (vertices && indices)
    {
    Point3f points[8];
    points[0] = box.origin;
    points[1] = box.origin + Vect3f(box.size.x, 0, 0);
    points[2] = box.origin + Vect3f(box.size.x, 0, box.size.z);
    points[3] = box.origin + Vect3f(0, 0, box.size.z);
    points[4] = box.origin + Vect3f(0, box.size.y, box.size.z);
    points[5] = box.origin + Vect3f(0, box.size.y, 0);
    points[6] = box.origin + Vect3f(box.size.x, box.size.y, 0);
    points[7] = box.origin + box.size;

    for (int i = 0; i < 8; i++)
      {
      box.transform.PointTransform(points[i]);
      vertices[i * 3] = points[i].x;
      vertices[i * 3 + 1] = points[i].y;
      vertices[i * 3 + 2] = points[i].z;
      }

    int index = 0;
    if (!(box.omit_faces & OMIT_Y_NEG))
      {
      indices[index++] = 0; indices[index++] = 1; indices[index++] = 2;
      indices[index++] = 2; indices[index++] = 3; indices[index++] = 0;
      }
    if (!(box.omit_faces & OMIT_X_NEG))
      {
      indices[index++] = 4; indices[index++] = 5; indices[index++] = 0;
      indices[index++] = 0; indices[index++] = 3; indices[index++] = 4;
      }
    if (!(box.omit_faces & OMIT_Y_POS))
      {
      indices[index++] = 7; indices[index++] = 6; indices[index++] = 5;
      indices[index++] = 5; indices[index++] = 4; indices[index++] = 7;
      }
    if (!(box.omit_faces & OMIT_Z_POS))
      {
      indices[index++] = 3; indices[index++] = 2; indices[index++] = 7;
      indices[index++] = 7; indices[index++] = 4; indices[index++] = 3;
      }
    if (!(box.omit_faces & OMIT_X_POS))
      {
      indices[index++] = 7; indices[index++] = 2; indices[index++] = 1;
      indices[index++] = 1; indices[index++] = 6; indices[index++] = 7;
      }
    if (!(box.omit_faces & OMIT_Z_NEG))
      {
      indices[index++] = 0; indices[index++] = 5; indices[index++] = 6;
      indices[index++] = 6; indices[index++] = 1; indices[index++] = 0;
      }
    }

  rtcCommitGeometry(geom);
  return geom;
  }

void SphereSubdivide(Vect3f v1,
                     Vect3f v2,
                     Vect3f v3,
                     unsigned i1,
                     unsigned i2,
                     unsigned i3,
                     std::vector<Vect3f> &sphere_points,
                     std::vector<Vect3u> &sphere_indices,
                     unsigned depth)
  {
  if (depth == 0)
    {
    sphere_indices.push_back(Vect3u(i1, i2, i3));
    return;
    }

  Vect3f v12 = (v1 + v2).Normalize();
  Vect3f v23 = (v2 + v3).Normalize();
  Vect3f v31 = (v3 + v1).Normalize();

  unsigned i12 = static_cast<unsigned>(sphere_points.size());
  unsigned i23 = i12 + 1;
  unsigned i31 = i23 + 1;

  sphere_points.push_back(v12);
  sphere_points.push_back(v23);
  sphere_points.push_back(v31);

  SphereSubdivide(v1, v12, v31, i1, i12, i31, sphere_points, sphere_indices, depth - 1);
  SphereSubdivide(v2, v23, v12, i2, i23, i12, sphere_points, sphere_indices, depth - 1);
  SphereSubdivide(v3, v31, v23, i3, i31, i23, sphere_points, sphere_indices, depth - 1);
  SphereSubdivide(v12, v23, v31, i12, i23, i31, sphere_points, sphere_indices, depth - 1);
  }

void InitSphere(std::vector<Vect3f> &sphere_points, std::vector<Vect3u> &sphere_indices, unsigned depth)
  {
  const float x0 = 0.525731112119133606f;
  const float z0 = 0.850650808352039932f;
  sphere_points.clear();
  sphere_indices.clear();

  sphere_points.push_back(Vect3f(-x0, 0.0f, z0));
  sphere_points.push_back(Vect3f(x0, 0.0f, z0));
  sphere_points.push_back(Vect3f(-x0, 0.0f, -z0));
  sphere_points.push_back(Vect3f(x0, 0.0f, -z0));
  sphere_points.push_back(Vect3f(0.0f, z0, x0));
  sphere_points.push_back(Vect3f(0.0f, z0, -x0));
  sphere_points.push_back(Vect3f(0.0f, -z0, x0));
  sphere_points.push_back(Vect3f(0.0f, -z0, -x0));
  sphere_points.push_back(Vect3f(z0, x0, 0.0f));
  sphere_points.push_back(Vect3f(-z0, x0, 0.0f));
  sphere_points.push_back(Vect3f(z0, -x0, 0.0f));
  sphere_points.push_back(Vect3f(-z0, -x0, 0.0f));

  const int tindices[20][3] =
    {
      {0, 4, 1},    {0, 9, 4},   {9, 5, 4},   {4, 5, 8},   {4, 8, 1},
      {8, 10, 1},   {8, 3, 10},  {5, 3, 8},   {5, 2, 3},   {2, 7, 3},
      {7, 10, 3},   {7, 6, 10},  {7, 11, 6},  {11, 0, 6},  {0, 1, 6},
      {6, 1, 10},   {9, 0, 11},  {9, 11, 2},  {9, 2, 5},   {7, 2, 11}
    };

  for (int i = 0; i < 20; i++)
    {
    SphereSubdivide(sphere_points[tindices[i][0]],
                    sphere_points[tindices[i][1]],
                    sphere_points[tindices[i][2]],
                    static_cast<unsigned>(tindices[i][0]),
                    static_cast<unsigned>(tindices[i][1]),
                    static_cast<unsigned>(tindices[i][2]),
                    sphere_points,
                    sphere_indices,
                    depth);
    }
  }

RTCGeometry CreateSphereGeometry(RTCDevice device, const SphereObject &sphere)
  {
  std::vector<Vect3f> sphere_normals;
  std::vector<Vect3u> sphere_indices;
  InitSphere(sphere_normals, sphere_indices, sphere.depth);

  std::vector<Point3f> sphere_points;
  sphere_points.reserve(sphere_normals.size());
  for (std::size_t i = 0; i < sphere_normals.size(); i++)
    {
    Point3f point = sphere.center + sphere_normals[i] * sphere.radius;
    sphere.transform.PointTransform(point);
    sphere_points.push_back(point);
    }

  RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
  float *vertices = static_cast<float *>(rtcSetNewGeometryBuffer(
      geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), sphere_points.size()));
  unsigned *indices = static_cast<unsigned *>(rtcSetNewGeometryBuffer(
      geom,
      RTC_BUFFER_TYPE_INDEX,
      0,
      RTC_FORMAT_UINT3,
      3 * sizeof(unsigned),
      sphere_indices.size()));

  if (vertices && indices)
    {
    for (std::size_t i = 0; i < sphere_points.size(); i++)
      {
      vertices[i * 3] = sphere_points[i].x;
      vertices[i * 3 + 1] = sphere_points[i].y;
      vertices[i * 3 + 2] = sphere_points[i].z;
      }

    for (std::size_t i = 0; i < sphere_indices.size(); i++)
      {
      indices[i * 3] = sphere_indices[i].x;
      indices[i * 3 + 1] = sphere_indices[i].y;
      indices[i * 3 + 2] = sphere_indices[i].z;
      }
    }

  rtcCommitGeometry(geom);
  return geom;
  }
}

bool RenderScene(const SceneDefinition &scene, const RenderSettings &settings, std::vector<double> &ppm_data)
  {
  RTCDevice device = rtcNewDevice(NULL);
  if (device == NULL)
    {
    std::printf("Device error %d: cannot create device\n", rtcGetDeviceError(NULL));
    return false;
    }
  rtcSetDeviceErrorFunction(device, DeviceErrorFunction, NULL);

  RTCScene rtc_scene = rtcNewScene(device);
  std::vector<Material> materials;

  for (std::size_t i = 0; i < scene.boxes.size(); i++)
    {
    RTCGeometry box = CreateBoxGeometry(device, scene.boxes[i]);
    unsigned geom_id = rtcAttachGeometry(rtc_scene, box);
    rtcReleaseGeometry(box);
    if (geom_id >= materials.size())
      materials.resize(static_cast<std::size_t>(geom_id) + 1);
    materials[geom_id] = scene.boxes[i].material;
    }

  for (std::size_t i = 0; i < scene.spheres.size(); i++)
    {
    RTCGeometry sphere = CreateSphereGeometry(device, scene.spheres[i]);
    unsigned geom_id = rtcAttachGeometry(rtc_scene, sphere);
    rtcReleaseGeometry(sphere);
    if (geom_id >= materials.size())
      materials.resize(static_cast<std::size_t>(geom_id) + 1);
    materials[geom_id] = scene.spheres[i].material;
    }

  rtcCommitScene(rtc_scene);

  Vect3f forward = Normalize(Sub(scene.camera.target, scene.camera.origin));
  Vect3f right = Normalize(Cross(forward, scene.camera.up));
  Vect3f up = Normalize(Cross(right, forward));
  float aspect = static_cast<float>(settings.width) / static_cast<float>(settings.height);
  float tan_half_fov = std::tan(settings.fov_y_deg * 0.5f * kPi / 180.f);

  RTCIntersectContext context;
  rtcInitIntersectContext(&context);

  std::vector<RTCRayHit> rayhits(settings.width);
  std::vector<Vect3d> image(static_cast<std::size_t>(settings.width) * static_cast<std::size_t>(settings.height));

  for (int row = 0; row < settings.height; row++)
    {
    for (int col = 0; col < settings.width; col++)
      {
      float sx = (2.f * (static_cast<float>(col) + 0.5f) / static_cast<float>(settings.width) - 1.f) * aspect * tan_half_fov;
      float sy = (1.f - 2.f * (static_cast<float>(row) + 0.5f) / static_cast<float>(settings.height)) * tan_half_fov;
      Vect3f dir = Normalize(Add(forward, Add(Scale(right, sx), Scale(up, sy))));
      InitRayHit(rayhits[static_cast<std::size_t>(col)], scene.camera.origin, dir);
      }

    rtcIntersect1M(rtc_scene, &context, rayhits.data(), settings.width, sizeof(RTCRayHit));

    for (int col = 0; col < settings.width; col++)
      {
      Vect3f color = rayhits[static_cast<std::size_t>(col)].hit.geomID != RTC_INVALID_GEOMETRY_ID
                         ? ShadeSurface(rayhits[static_cast<std::size_t>(col)], rtc_scene, materials, scene.lights)
                         : Vf(kBackgroundR, kBackgroundG, kBackgroundB);
      image[ImageIndex(settings.width, row, col)] = Vd(color.x, color.y, color.z);
      }
    }

  ppm_data.assign(static_cast<std::size_t>(settings.width) * static_cast<std::size_t>(settings.height) * 3u, 0.0);
  for (int row = 0; row < settings.height; row++)
    for (int col = 0; col < settings.width; col++)
      {
      std::size_t image_index = ImageIndex(settings.width, row, col);
      std::size_t pixel_index = 3u * image_index;
      ppm_data[pixel_index + 0] = image[image_index].x;
      ppm_data[pixel_index + 1] = image[image_index].y;
      ppm_data[pixel_index + 2] = image[image_index].z;
      }

  rtcReleaseScene(rtc_scene);
  rtcReleaseDevice(device);
  return true;
  }
}
