/*
Copyright (C) 2010 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MainWindow.h"

// VTK
#include <vtkActor.h>
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVertexGlyphFilter.h>

// STL
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
{
  // Setup the GUI and connect all of the signals and slots
  setupUi(this);

  this->CurrentPoints = vtkSmartPointer<vtkPoints>::New();
  this->CurrentPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->CurrentVertexGlyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  this->CurrentMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->CurrentActor = vtkSmartPointer<vtkActor>::New();
  
  this->RGBPoints = vtkSmartPointer<vtkPoints>::New();
  this->RGBPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->RGBVertexGlyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  this->RGBMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->RGBActor = vtkSmartPointer<vtkActor>::New();
  
  this->HSVPoints = vtkSmartPointer<vtkPoints>::New();
  this->HSVPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->HSVVertexGlyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  this->HSVMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->HSVActor = vtkSmartPointer<vtkActor>::New();
  
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->Colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  
  this->Colors->SetNumberOfComponents(3);
  this->Colors->SetName ("Colors");

  this->Transition = 0.0f;
  
  this->Spacing = 10;
  this->NumberOfSteps = 20;
  this->CurrentStep = 0;
  
  this->RGBPolyData->SetPoints(this->RGBPoints);
  this->RGBPolyData->GetPointData()->SetScalars(this->Colors);
  
  this->CurrentPolyData->SetPoints(this->CurrentPoints);
  this->CurrentPolyData->GetPointData()->SetScalars(this->Colors);
  
  this->HSVPolyData->SetPoints(this->HSVPoints);
  this->HSVPolyData->GetPointData()->SetScalars(this->Colors);

  this->RGBVertexGlyphFilter->SetInputConnection(this->RGBPolyData->GetProducerPort());
  this->HSVVertexGlyphFilter->SetInputConnection(this->HSVPolyData->GetProducerPort());
  this->CurrentVertexGlyphFilter->SetInputConnection(this->CurrentPolyData->GetProducerPort());
  
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->RGBMapper->SetInputConnection(this->RGBVertexGlyphFilter->GetOutputPort());
  this->HSVMapper->SetInputConnection(this->HSVVertexGlyphFilter->GetOutputPort());
  this->CurrentMapper->SetInputConnection(this->CurrentVertexGlyphFilter->GetOutputPort());
  
  this->HSVActor->SetMapper(this->HSVMapper);
  this->RGBActor->SetMapper(this->RGBMapper);
  this->CurrentActor->SetMapper(this->CurrentMapper);

  CreateColors();
  SetupRGBCube();
  SetupHSVCylinder();
  this->CurrentPoints->DeepCopy(this->RGBPoints);
  //this->CurrentPoints->DeepCopy(this->HSVPoints);

  
  //this->Renderer->AddViewProp(this->HSVActor);
  //this->Renderer->AddViewProp(this->RGBActor);
  this->Renderer->AddViewProp(this->CurrentActor);

  connect(&timer, SIGNAL(timeout()), this, SLOT(Step()));
}

void MainWindow::CreateColors()
{
  for(unsigned int r = 0; r < 256; r += this->Spacing)
    {
    for(unsigned int g = 0; g < 256; g += this->Spacing)
      {
      for(unsigned int b = 0; b < 256; b += this->Spacing)
        {
        unsigned char color[3] = {r,g,b};
        this->Colors->InsertNextTupleValue(color);
        }
      }
    }
}

void MainWindow::SetupRGBCube()
{
  this->RGBPoints->Reset();
  this->RGBPoints->Squeeze();
  for(unsigned int r = 0; r < 256; r += this->Spacing)
    {
    for(unsigned int g = 0; g < 256; g += this->Spacing)
      {
      for(unsigned int b = 0; b < 256; b += this->Spacing)
        {
        this->RGBPoints->InsertNextPoint(r,g,b);
        }
      }
    }
}

void MainWindow::SetupHSVCylinder()
{
  this->HSVPoints->Reset();
  this->HSVPoints->Squeeze();
  for(unsigned int r = 0; r < 256; r += this->Spacing)
    {
    for(unsigned int g = 0; g < 256; g += this->Spacing)
      {
      for(unsigned int b = 0; b < 256; b += this->Spacing)
        {
        float floatRGB[3] = {r,g,b};
        
        float hsv[3];
        vtkMath::RGBToHSV(floatRGB, hsv);

        float h = hsv[0];
        float s = hsv[1];
        float v = hsv[2];

        float r = s; // Radius of cylinder
        float theta = h; // Angle

        float z = v*0.01f; // The spacing of the Z/V slices of the cylinder are way too far apart without this scaling
        float x = r*cos(theta * 2.0f * vtkMath::Pi());
        float y = r*sin(theta * 2.0f * vtkMath::Pi());
        float xyz[3] = {x,y,z};
        this->HSVPoints->InsertNextPoint(xyz);
        }
      }
    }

  // Translate and scale the points to they are in the same position and magnitude as the RGB cube
  double hsvBounds[6];
  this->HSVPoints->GetBounds(hsvBounds);

  double rgbBounds[6];
  this->RGBPoints->GetBounds(rgbBounds);

  float translation[3];
  float scale[3];
  for(unsigned int i = 0 ; i < 3; ++i)
    {
    scale[i] = (rgbBounds[2*i] - rgbBounds[2*i + 1])/(hsvBounds[2*i] - hsvBounds[2*i + 1]);
    translation[i] = rgbBounds[2*i] - hsvBounds[2*i];
    }
//   std::cout  << "xmin: " << bounds[0] << " "
//              << "xmax: " << bounds[1] << std::endl
//              << "ymin: " << bounds[2] << " "
//              << "ymax: " << bounds[3] << std::endl
//              << "zmin: " << bounds[4] << " "
//              << "zmax: " << bounds[5] << std::endl;

  std::cout << "Scale: " << scale[0] << " " << scale[1] << " " << scale[2] << std::endl;
  std::cout << "translation: " << translation[0] << " " << translation[1] << " " << translation[2] << std::endl;
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->Scale(scale);
  transform->Translate(translation);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputConnection(this->HSVPolyData->GetProducerPort());
  transformFilter->SetTransform(transform);
  transformFilter->Update();

  this->HSVPoints->DeepCopy(transformFilter->GetOutput()->GetPoints());
  
  this->HSVPoints->Modified();
  this->HSVPolyData->Modified();
  transformFilter->RemoveAllInputs();
}

void MainWindow::on_btnTransition_clicked()
{
  this->timer.start(500);
}

void MainWindow::on_btnStep_clicked()
{
  Step();
}

void MainWindow::Step()
{
  this->CurrentStep++;
  std::cout << "Current step: " << this->CurrentStep << std::endl;
  
  this->Transition = static_cast<float>(this->CurrentStep)/static_cast<float>(this->NumberOfSteps);
  std::cout << "Transition: " << this->Transition << std::endl;
  if(this->Transition > 1.0f)
    {
    this->timer.stop();
    return;
    }
  for(unsigned int pointId = 0; pointId < this->CurrentPoints->GetNumberOfPoints(); ++pointId)
    {
    double newPoint[3];
    double rgb[3];
    this->RGBPoints->GetPoint(pointId, rgb);
    //std::cout << "rgb: " << rgb[0] << " " << rgb[1] << " " << rgb[2] << std::endl;
    double hsv[3];
    this->HSVPoints->GetPoint(pointId, hsv);
    //std::cout << "hsv: " << hsv[0] << " " << hsv[1] << " " << hsv[2] << std::endl;
    for(unsigned int component = 0; component < 3; component++)
      {
      newPoint[component] = rgb[component] + (hsv[component] - rgb[component]) * this->Transition;
      }
    //std::cout << "newPoint: " << newPoint[0] << " " << newPoint[1] << " " << newPoint[2] << std::endl;
    this->CurrentPoints->SetPoint(pointId, newPoint);

    double verify[3];
    this->CurrentPoints->GetPoint(pointId, verify);
    //std::cout << "verify: " << verify[0] << " " << verify[1] << " " << verify[2] << std::endl;
    }

  std::cout << "Number of current points: " << this->CurrentPoints->GetNumberOfPoints() << std::endl;
    
  this->CurrentPoints->Modified();
  //this->PolyData->Modified();
  //this->VertexGlyphFilter->Update();
  this->qvtkWidget->GetRenderWindow()->Render();
}
