/*
Copyright (C) 2011 David Doria, daviddoria@gmail.com

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt
#include "ui_MainWindow.h"

#include <vtkSmartPointer.h>

// Forward declarations
class vtkPolyDataMapper;
class vtkActor;
class vtkRenderer;
class vtkVertexGlyphFilter;
class vtkPoints;
class vtkPolyData;
class vtkUnsignedCharArray;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);

public slots:
  void on_pushButton_clicked();
protected:

  void CreateColors();
  
  void SetupRGBCube();
  void SetupHSVCylinder();
  
  vtkSmartPointer<vtkPoints> CurrentPoints;
  vtkSmartPointer<vtkPolyData> CurrentPolyData;
  vtkSmartPointer<vtkVertexGlyphFilter> CurrentVertexGlyphFilter;
  vtkSmartPointer<vtkPolyDataMapper> CurrentMapper;
  vtkSmartPointer<vtkActor> CurrentActor;

  vtkSmartPointer<vtkPoints> RGBPoints;
  vtkSmartPointer<vtkPolyData> RGBPolyData;
  vtkSmartPointer<vtkVertexGlyphFilter> RGBVertexGlyphFilter;
  vtkSmartPointer<vtkPolyDataMapper> RGBMapper;
  vtkSmartPointer<vtkActor> RGBActor;

  vtkSmartPointer<vtkPoints> HSVPoints;
  vtkSmartPointer<vtkPolyData> HSVPolyData;
  vtkSmartPointer<vtkVertexGlyphFilter> HSVVertexGlyphFilter;
  vtkSmartPointer<vtkPolyDataMapper> HSVMapper;
  vtkSmartPointer<vtkActor> HSVActor;
  

  vtkSmartPointer<vtkUnsignedCharArray> Colors;
  vtkSmartPointer<vtkRenderer> Renderer;

  unsigned int Spacing;

  float Transition; // This is the 'time' variable in the simulation
  unsigned int NumberOfSteps;
  unsigned int CurrentStep;

};

#endif
