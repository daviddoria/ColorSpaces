#ifndef HELPERS_H
#define HELPERS_H

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageData.h>


class vtkPolyData;

namespace Helpers
{

void CreateTransparentImage(vtkImageData* input, vtkImageData* output);


void OutputImageSize(vtkImageData* image);

}

#endif
