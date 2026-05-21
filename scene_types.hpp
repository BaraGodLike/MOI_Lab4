#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

namespace renderlab
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kShadowBias = 1e-3f;
constexpr float kBackgroundR = 0.02f;
constexpr float kBackgroundG = 0.03f;
constexpr float kBackgroundB = 0.05f;

struct Vect3f
  {
  float x;
  float y;
  float z;

  Vect3f() : x(0), y(0), z(0) {}
  Vect3f(float px, float py, float pz) : x(px), y(py), z(pz) {}

  Vect3f operator+(const Vect3f &other) const
    {
    return Vect3f(x + other.x, y + other.y, z + other.z);
    }

  Vect3f operator-(const Vect3f &other) const
    {
    return Vect3f(x - other.x, y - other.y, z - other.z);
    }

  Vect3f operator-() const
    {
    return Vect3f(-x, -y, -z);
    }

  Vect3f operator*(float scale) const
    {
    return Vect3f(x * scale, y * scale, z * scale);
    }

  float Dot(const Vect3f &other) const
    {
    return x * other.x + y * other.y + z * other.z;
    }

  float Length() const
    {
    return std::sqrt(Dot(*this));
    }

  Vect3f Normalize() const
    {
    float len = Length();
    return len > 0.f ? (*this) * (1.f / len) : Vect3f();
    }
  };

inline Vect3f operator*(float scale, const Vect3f &value)
  {
  return value * scale;
  }

using Point3f = Vect3f;

struct Vect3d
  {
  double x;
  double y;
  double z;

  Vect3d() : x(0), y(0), z(0) {}
  Vect3d(double px, double py, double pz) : x(px), y(py), z(pz) {}
  };

struct Vect3u
  {
  unsigned x;
  unsigned y;
  unsigned z;

  Vect3u() : x(0), y(0), z(0) {}
  Vect3u(unsigned px, unsigned py, unsigned pz) : x(px), y(py), z(pz) {}
  };

struct Matrix43f
  {
  float sx;
  float sy;
  float sz;
  float tx;
  float ty;
  float tz;

  Matrix43f(float psx = 1.f, float psy = 1.f, float psz = 1.f)
      : sx(psx), sy(psy), sz(psz), tx(0.f), ty(0.f), tz(0.f)
    {
    }

  void PointTransform(Point3f &point) const
    {
    point.x = point.x * sx + tx;
    point.y = point.y * sy + ty;
    point.z = point.z * sz + tz;
    }
  };

struct Material
  {
  Vect3f diffuse;
  Vect3f specular;
  float shininess;
  };

enum LightType
  {
  LIGHT_POINT = 0,
  LIGHT_DIRECTIONAL = 1
  };

struct Light
  {
  LightType type;
  Vect3f color;
  Point3f position;
  Vect3f direction;
  float intensity;
  };

struct RenderSettings
  {
  int width;
  int height;
  const char *ppm_file;
  float fov_y_deg;
  };

struct Camera
  {
  Point3f origin;
  Point3f target;
  Vect3f up;
  };

enum OmitFace
  {
  OMIT_NONE = 0,
  OMIT_X_POS = 1,
  OMIT_X_NEG = 2,
  OMIT_Y_POS = 4,
  OMIT_Y_NEG = 8,
  OMIT_Z_POS = 16,
  OMIT_Z_NEG = 32
  };

struct BoxObject
  {
  Point3f origin;
  Vect3f size;
  int omit_faces;
  Matrix43f transform;
  Material material;
  };

struct SphereObject
  {
  Point3f center;
  float radius;
  unsigned depth;
  Matrix43f transform;
  Material material;
  };

struct SceneDefinition
  {
  Camera camera;
  std::vector<BoxObject> boxes;
  std::vector<SphereObject> spheres;
  std::vector<Light> lights;
  };

inline Vect3f Vf(float x, float y, float z) { return Vect3f(x, y, z); }
inline Point3f Pf(float x, float y, float z) { return Point3f(x, y, z); }
inline Vect3d Vd(double x, double y, double z) { return Vect3d(x, y, z); }
inline Vect3f Add(const Vect3f &a, const Vect3f &b) { return a + b; }
inline Vect3f Sub(const Vect3f &a, const Vect3f &b) { return a - b; }
inline Vect3f Scale(const Vect3f &v, float s) { return v * s; }
inline Vect3f Mul(const Vect3f &a, const Vect3f &b) { return Vf(a.x * b.x, a.y * b.y, a.z * b.z); }
inline float Dot(const Vect3f &a, const Vect3f &b) { return a.Dot(b); }
inline Vect3f Cross(const Vect3f &a, const Vect3f &b)
  {
  return Vf(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
  }
inline float Length(const Vect3f &v) { return v.Length(); }
inline Vect3f Normalize(const Vect3f &v) { return v.Normalize(); }
inline float Sat(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }
inline Vect3f ClampPositive(const Vect3f &v)
  {
  return Vf(v.x < 0 ? 0 : v.x, v.y < 0 ? 0 : v.y, v.z < 0 ? 0 : v.z);
  }
inline std::size_t ImageIndex(int width, int row, int col)
  {
  return static_cast<std::size_t>(row) * static_cast<std::size_t>(width) + static_cast<std::size_t>(col);
  }
}
