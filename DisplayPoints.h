
#ifndef DisplayPoints_H
#define DisplayPoints_H

#include <vtkSmartPointer.h>

// Forward declarations
class vtkPolyDataMapper;
class vtkActor;
class vtkVertexGlyphFilter;
class vtkPoints;
class vtkPolyData;

class DisplayPoints
{
public:
  DisplayPoints();
  vtkSmartPointer<vtkPoints> Points;
  vtkSmartPointer<vtkPolyData> PolyData;
  vtkSmartPointer<vtkVertexGlyphFilter> VertexGlyphFilter;
  vtkSmartPointer<vtkPolyDataMapper> Mapper;
  vtkSmartPointer<vtkActor> Actor;
};

#endif
