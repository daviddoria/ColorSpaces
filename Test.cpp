#include "Conversions.h"

#include <iostream>

#include <vtkMath.h>

static void TestHSV();
static void TestCIELab();

int main()
{
  TestHSV();
  //TestCIELab();
  return 0;
}

void TestCIELab()
{
  unsigned char rgb[3] = {176, 43, 0}; // = CIELab 40,52,52
  float cieLab[3];
  RGBtoCIELab(rgb, cieLab);

  std::cout << "CIELab: " << cieLab[0] << " " << cieLab[1] << " " << cieLab[2] << std::endl;

}

void TestHSV()
{
    float rgb[3] = {176, 43, 0};
    
    float hsv[3];

    vtkMath::RGBToHSV(rgb, hsv);
    std::cout << "HSV: " << hsv[0] << " " << hsv[1] << " " << hsv[2] << std::endl;
    std::cout << "HSV: " << 2.0f * vtkMath::Pi() * hsv[0] << " " << hsv[1] << " " << hsv[2] << std::endl;
}
