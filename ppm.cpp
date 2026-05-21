#include <cmath>
#include <cstdio>

/// \~english
/// Write PPM image file
/// @tparam FLOAT Type of floating point data
/// @param[in] file_name Name of the output PPM file
/// @param[in] columns Number of columns in the image
/// @param[in] rows Number of rows in the image
/// @param[in] data Pointer to RGB data array (3 values per pixel: R, G, B)
/// @return 0 on success, -1 on failure (e.g., cannot open file)
///
/// \~russian
/// Р—Р°РїРёСЃР°С‚СЊ С„Р°Р№Р» РёР·РѕР±СЂР°Р¶РµРЅРёСЏ PPM
/// @tparam FLOAT РўРёРї РґР°РЅРЅС‹С… СЃ РїР»Р°РІР°СЋС‰РµР№ С‚РѕС‡РєРѕР№
/// @param[in] file_name РРјСЏ РІС‹С…РѕРґРЅРѕРіРѕ С„Р°Р№Р»Р° PPM
/// @param[in] columns РљРѕР»РёС‡РµСЃС‚РІРѕ СЃС‚РѕР»Р±С†РѕРІ РІ РёР·РѕР±СЂР°Р¶РµРЅРёРё
/// @param[in] rows РљРѕР»РёС‡РµСЃС‚РІРѕ СЃС‚СЂРѕРє РІ РёР·РѕР±СЂР°Р¶РµРЅРёРё
/// @param[in] data РЈРєР°Р·Р°С‚РµР»СЊ РЅР° РјР°СЃСЃРёРІ РґР°РЅРЅС‹С… RGB (3 Р·РЅР°С‡РµРЅРёСЏ РЅР° РїРёРєСЃРµР»СЊ: R, G, B)
/// @return 0 РїСЂРё СѓСЃРїРµС…Рµ, -1 РїСЂРё РЅРµСѓРґР°С‡Рµ (РЅР°РїСЂРёРјРµСЂ, РЅРµ СѓРґР°РµС‚СЃСЏ РѕС‚РєСЂС‹С‚СЊ С„Р°Р№Р»)
template <class FLOAT>
int WritePPM(const char *file_name, int columns, int rows, FLOAT *data)
  {
  FLOAT imax(0), msum(0), isum(0), scale(1), dh(0);
  FILE *f = fopen(file_name, "w");
  FLOAT h[1024];
  int i128(0);

  if (f == NULL)
    return -1;

  for (int i = 0; i < 1024; i++)
    h[i] = 0;
  // Find maximum
  for (int ir = 0; ir < rows; ir++)
    for (int ic = 0; ic < columns; ic++)
      {
      FLOAT r(data[3 * (ir * columns + ic) + 0]);
      FLOAT g(data[3 * (ir * columns + ic) + 1]);
      FLOAT b(data[3 * (ir * columns + ic) + 2]);
      FLOAT rgb(r >= g ? (r >= b ? r : b) : (g >= b ? g : b));

      isum += rgb;
      if (rgb > imax)
        imax = rgb;
      }
  if (imax == 0)
    {
    // No image
    fprintf(f, "P3\n%d %d\n255\n", columns, rows);
    for (int ir = 0; ir < rows; ir++)
      for (int ic = 0; ic < columns; ic++)
        fprintf(f, "0 0 0\n");
    fclose(f);
    return 0;
    }
  // Tone mapping
  dh = (FLOAT)(imax / 1024. * (1. + 1. / 1024.));
  for (int ir = 0; ir < rows; ir++)
    for (int ic = 0; ic < columns; ic++)
      {
      FLOAT r(data[3 * (ir * columns + ic) + 0]);
      FLOAT g(data[3 * (ir * columns + ic) + 1]);
      FLOAT b(data[3 * (ir * columns + ic) + 2]);
      FLOAT rgb(r >= g ? (r >= b ? r : b) : (g >= b ? g : b));
      int irgb;

      irgb = (int)(rgb / dh);
      if (irgb > 1023)
        irgb = 1023;
      h[irgb] += rgb;
      }
  for (int i = 0; i < 1024; i++)
    {
    if (msum > isum / 2)
      {
      i128 = i;
      break;
      }
    msum += h[i];
    }
  scale = (FLOAT)(0.5 / (dh * (i128 + 0.5)));

  // Write image
  fprintf(f, "P3\n%d %d\n255\n", columns, rows);
  for (int ir = 0; ir < rows; ir++)
    for (int ic = 0; ic < columns; ic++)
      {
      FLOAT rf(scale * data[3 * (ir * columns + ic) + 0]);
      FLOAT gf(scale * data[3 * (ir * columns + ic) + 1]);
      FLOAT bf(scale * data[3 * (ir * columns + ic) + 2]);
      rf = rf < 0 ? 0 : rf;
      gf = gf < 0 ? 0 : gf;
      bf = bf < 0 ? 0 : bf;
      int r((int)(255 * (std::pow(rf, (FLOAT)(1. / 2.2)) > 1 ? 1 : std::pow(rf, (FLOAT)(1. / 2.2)))));
      int g((int)(255 * (std::pow(gf, (FLOAT)(1. / 2.2)) > 1 ? 1 : std::pow(gf, (FLOAT)(1. / 2.2)))));
      int b((int)(255 * (std::pow(bf, (FLOAT)(1. / 2.2)) > 1 ? 1 : std::pow(bf, (FLOAT)(1. / 2.2)))));

      fprintf(f, "%d %d %d\n", r, g, b);
      }
  fclose(f);
  return 0;
  }

template int WritePPM<float>(const char *file_name, int columns, int rows,
                             float *data);
template int WritePPM<double>(const char *file_name, int columns, int rows,
                              double *data);