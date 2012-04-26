#include "DisplayPoints.h"

// VTK
#include <vtkActor.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkVertexGlyphFilter.h>

DisplayPoints::DisplayPoints()
{
  this->Points = vtkSmartPointer<vtkPoints>::New();
  this->PolyData = vtkSmartPointer<vtkPolyData>::New();
  this->VertexGlyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  this->Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->Actor = vtkSmartPointer<vtkActor>::New();

  this->PolyData->SetPoints(this->Points);

  this->VertexGlyphFilter->SetInputData(this->PolyData);

  this->Mapper->SetInputConnection(this->VertexGlyphFilter->GetOutputPort());
  this->Actor->SetMapper(this->Mapper);
}
