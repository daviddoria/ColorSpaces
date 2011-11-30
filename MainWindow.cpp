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
#include "Conversions.h"

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

// Qt
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent)
{
  // Setup the GUI and connect all of the signals and slots
  setupUi(this);

  QButtonGroup* groupFrom = new QButtonGroup;
  groupFrom->addButton(radFromRGB);
  groupFrom->addButton(radFromHSV);
  groupFrom->addButton(radFromCIELab);
  radFromRGB->setChecked(true);

  QButtonGroup* groupTo = new QButtonGroup;
  groupTo->addButton(radToRGB);
  groupTo->addButton(radToHSV);
  groupTo->addButton(radToCIELab);
  radToHSV->setChecked(true);
  
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->Colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  
  this->Colors->SetNumberOfComponents(3);
  this->Colors->SetName ("Colors");

  this->Transition = 0.0f;
  
  this->Spacing = 10;
  this->MaxNumberOfSteps = 100;
  this->MaxSpeed = 1000;
  this->CurrentStep = 0;
  
  this->RGBPoints.PolyData->GetPointData()->SetScalars(this->Colors);
  this->TransitionPoints.PolyData->GetPointData()->SetScalars(this->Colors);
  this->HSVPoints.PolyData->GetPointData()->SetScalars(this->Colors);
  this->CIELabPoints.PolyData->GetPointData()->SetScalars(this->Colors);

  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  CreateColors();
  SetupRGBCube();
  SetupHSVCylinder();
  SetupCIELab();

  this->Renderer->AddViewProp(this->TransitionPoints.Actor);

  connect(&timer, SIGNAL(timeout()), this, SLOT(Step()));
  SetupFromGUI();
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
  std::cout << "SetupRGBCube" << std::endl;
  this->RGBPoints.Points->Reset();
  this->RGBPoints.Points->Squeeze();
  for(unsigned int i = 0 ;i < this->Colors->GetNumberOfTuples(); ++i)
    {
    unsigned char color[3];
    this->Colors->GetTupleValue(i, color);
    this->RGBPoints.Points->InsertNextPoint(color[0], color[1], color[2]);
    }
}

void MainWindow::SetupCIELab()
{
  std::cout << "SetupCIELab" << std::endl;
  this->CIELabPoints.Points->Reset();
  this->CIELabPoints.Points->Squeeze();
  for(unsigned int i = 0 ;i < this->Colors->GetNumberOfTuples(); ++i)
    {
    unsigned char color[3];
    this->Colors->GetTupleValue(i, color);

    float cielab[3];
    RGBtoCIELab(color, cielab);

    float L = cielab[0];
    float a = cielab[1];
    float b = cielab[2];

//     float z = v*0.01f; // The spacing of the Z/V slices of the cylinder are way too far apart without this scaling
//     float x = r*cos(theta * 2.0f * vtkMath::Pi());
//     float y = r*sin(theta * 2.0f * vtkMath::Pi());
    float xyz[3] = {L,a,b};
    this->CIELabPoints.Points->InsertNextPoint(xyz);
    }

  // Translate and scale the points to they are in the same position and magnitude as the RGB cube
  double cielabBounds[6];
  this->CIELabPoints.Points->GetBounds(cielabBounds);

  double rgbBounds[6];
  this->RGBPoints.Points->GetBounds(rgbBounds);

  OutputBounds("rgbBounds", rgbBounds);

  float scale[3];
  for(unsigned int i = 0 ; i < 3; ++i)
    {
    scale[i] = (rgbBounds[2*i + 1] - rgbBounds[2*i])/(cielabBounds[2*i + 1] - cielabBounds[2*i]);
    }
  OutputBounds("cielabBounds", cielabBounds);

  std::cout << "Scale: " << scale[0] << " " << scale[1] << " " << scale[2] << std::endl;
  
  vtkSmartPointer<vtkTransform> scaleTransform = vtkSmartPointer<vtkTransform>::New();
  scaleTransform->Scale(scale);
  
  vtkSmartPointer<vtkTransformPolyDataFilter> scaleTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  scaleTransformFilter->SetInputConnection(this->CIELabPoints.PolyData->GetProducerPort());
  scaleTransformFilter->SetTransform(scaleTransform);
  scaleTransformFilter->Update();

  double scaledBounds[6];
  scaleTransformFilter->GetOutput()->GetBounds(scaledBounds);

  OutputBounds("scaledBounds", scaledBounds);
  
  float translation[3];
  for(unsigned int i = 0 ; i < 3; ++i)
    {
    translation[i] = rgbBounds[2*i] - scaledBounds[2*i];
    }
    
  vtkSmartPointer<vtkTransform> translateTransform = vtkSmartPointer<vtkTransform>::New();
  translateTransform->Translate(translation);

  vtkSmartPointer<vtkTransformPolyDataFilter> translateTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  translateTransformFilter->SetInputConnection(scaleTransformFilter->GetOutputPort());
  translateTransformFilter->SetTransform(translateTransform);
  translateTransformFilter->Update();
  
  std::cout << "translation: " << translation[0] << " " << translation[1] << " " << translation[2] << std::endl;

  this->CIELabPoints.Points->DeepCopy(translateTransformFilter->GetOutput()->GetPoints());

  double newBounds[6];
  this->CIELabPoints.Points->GetBounds(newBounds);
  OutputBounds("newBounds", newBounds);

  this->CIELabPoints.Points->Modified();
  this->CIELabPoints.PolyData->Modified();
  //transformFilter->RemoveAllInputs();
}

void MainWindow::SetupHSVCylinder()
{
  std::cout << "SetupHSVCylinder" << std::endl;
  this->HSVPoints.Points->Reset();
  this->HSVPoints.Points->Squeeze();
  for(unsigned int i = 0 ;i < this->Colors->GetNumberOfTuples(); ++i)
    {
    unsigned char color[3];
    this->Colors->GetTupleValue(i, color);
    float floatRGB[3] = {color[0], color[1], color[2]};

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
    this->HSVPoints.Points->InsertNextPoint(xyz);
    }

  // Translate and scale the points to they are in the same position and magnitude as the RGB cube
  double hsvBounds[6];
  this->HSVPoints.Points->GetBounds(hsvBounds);

  double rgbBounds[6];
  this->RGBPoints.Points->GetBounds(rgbBounds);

  float translation[3];
  float scale[3];
  for(unsigned int i = 0 ; i < 3; ++i)
    {
    //scale[i] = (rgbBounds[2*i] - rgbBounds[2*i + 1])/(hsvBounds[2*i] - hsvBounds[2*i + 1]);
    scale[i] = (rgbBounds[2*i + 1] - rgbBounds[2*i])/(hsvBounds[2*i + 1] - hsvBounds[2*i]);
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
  transformFilter->SetInputConnection(this->HSVPoints.PolyData->GetProducerPort());
  transformFilter->SetTransform(transform);
  transformFilter->Update();

  this->HSVPoints.Points->DeepCopy(transformFilter->GetOutput()->GetPoints());
  
  this->HSVPoints.Points->Modified();
  this->HSVPoints.PolyData->Modified();
  transformFilter->RemoveAllInputs();
}

void MainWindow::on_btnTransition_clicked()
{
  if(this->CurrentPoints == this->NextPoints)
    {
    return;
    }
  int value = this->MaxSpeed - static_cast<float>(this->sldSpeed->value())/100.f * static_cast<float>(this->MaxSpeed);
  std::cout << "Speed value: " << value << std::endl;
  this->timer.start(value);
}

void MainWindow::on_btnStop_clicked()
{
  this->timer.stop();
}

void MainWindow::on_btnStep_clicked()
{
  if(this->CurrentPoints == this->NextPoints)
    {
    return;
    }
  Step();
}

void MainWindow::Step()
{
  this->CurrentStep++;
  std::cout << "Current step: " << this->CurrentStep << std::endl;
  
  this->Transition = static_cast<float>(this->CurrentStep)/static_cast<float>(static_cast<float>(this->sldSteps->value())/100. * this->MaxNumberOfSteps);
  std::cout << "Transition: " << this->Transition << std::endl;
  if(this->Transition > 1.0f)
    {
    this->timer.stop();
    this->CurrentStep = 0;
    return;
    }
  for(unsigned int pointId = 0; pointId < this->CurrentPoints->GetNumberOfPoints(); ++pointId)
    {
    double newPoint[3];
    double current[3];
    this->CurrentPoints->GetPoint(pointId, current);
    //std::cout << "rgb: " << rgb[0] << " " << rgb[1] << " " << rgb[2] << std::endl;
    double next[3];
    this->NextPoints->GetPoint(pointId, next);
    //std::cout << "hsv: " << hsv[0] << " " << hsv[1] << " " << hsv[2] << std::endl;
    for(unsigned int component = 0; component < 3; component++)
      {
      newPoint[component] = current[component] + (next[component] - current[component]) * this->Transition;
      }
    //std::cout << "newPoint: " << newPoint[0] << " " << newPoint[1] << " " << newPoint[2] << std::endl;
    this->TransitionPoints.Points->SetPoint(pointId, newPoint);

    //double verify[3];
    //this->CurrentPoints.Points->GetPoint(pointId, verify);
    //std::cout << "verify: " << verify[0] << " " << verify[1] << " " << verify[2] << std::endl;
    }

  //std::cout << "Number of current points: " << this->CurrentPoints.Points->GetNumberOfPoints() << std::endl;
    
  this->TransitionPoints.Points->Modified();
  //this->PolyData->Modified();
  //this->VertexGlyphFilter->Update();
  this->qvtkWidget->GetRenderWindow()->Render();
}

void MainWindow::SetupFromGUI()
{
  if(radFromRGB->isChecked())
    {
    this->CurrentPoints = this->RGBPoints.Points;
    this->TransitionPoints.Points->DeepCopy(this->RGBPoints.Points);
    }
  else if(radFromHSV->isChecked())
    {
    this->CurrentPoints = this->HSVPoints.Points;
    this->TransitionPoints.Points->DeepCopy(this->HSVPoints.Points);
    }
  else if(radFromCIELab->isChecked())
    {
    this->CurrentPoints = this->CIELabPoints.Points;
    this->TransitionPoints.Points->DeepCopy(this->CIELabPoints.Points);
    }

  if(radToRGB->isChecked())
    {
    this->NextPoints = this->RGBPoints.Points;
    }
  else if(radToHSV->isChecked())
    {
    this->NextPoints = this->HSVPoints.Points;
    }
  else if(radToCIELab->isChecked())
    {
    this->NextPoints = this->CIELabPoints.Points;
    }
}

void MainWindow::on_radFromRGB_clicked()
{
  this->CurrentPoints = this->RGBPoints.Points;
  this->TransitionPoints.Points->DeepCopy(this->RGBPoints.Points);
  this->qvtkWidget->GetRenderWindow()->Render();
}

void MainWindow::on_radFromHSV_clicked()
{
  this->CurrentPoints = this->HSVPoints.Points;
  this->TransitionPoints.Points->DeepCopy(this->HSVPoints.Points);
  this->qvtkWidget->GetRenderWindow()->Render();
}

void MainWindow::on_radFromCIELab_clicked()
{
  this->CurrentPoints = this->CIELabPoints.Points;
  this->TransitionPoints.Points->DeepCopy(this->CIELabPoints.Points);
  this->qvtkWidget->GetRenderWindow()->Render();
}

void MainWindow::on_radToRGB_clicked()
{
  this->NextPoints = this->RGBPoints.Points;
}

void MainWindow::on_radToHSV_clicked()
{
  this->NextPoints = this->HSVPoints.Points;
}

void MainWindow::on_radToCIELab_clicked()
{
  this->NextPoints = this->CIELabPoints.Points;
}

void MainWindow::on_sldSpeed_valueChanged(int value)
{

}

void MainWindow::on_sldSteps_valueChanged(int value)
{

}

void OutputBounds(const std::string& name, double bounds[6])
{
  std::cout << name << " xmin: " << bounds[0] << " "
            << name << " xmax: " << bounds[1] << std::endl
            << name << " ymin: " << bounds[2] << " "
            << name << " ymax: " << bounds[3] << std::endl
            << name << " zmin: " << bounds[4] << " "
            << name << " zmax: " << bounds[5] << std::endl;
}