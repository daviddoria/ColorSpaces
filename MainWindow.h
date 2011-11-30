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

// Custom
#include "DisplayPoints.h"

// Qt
#include "ui_MainWindow.h"
#include <QTimer>

// VTK
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
  void on_btnStop_clicked();
  void on_btnStep_clicked();
  void on_btnTransition_clicked();
  void Step();

  void on_radFromRGB_clicked();
  void on_radFromHSV_clicked();
  void on_radFromCIELab_clicked();
  void on_radToRGB_clicked();
  void on_radToHSV_clicked();
  void on_radToCIELab_clicked();

  void on_sldSpeed_valueChanged(int);
  void on_sldSteps_valueChanged(int);
protected:

  QTimer timer;
  
  void CreateColors();

  void SetupFromGUI();
  
  void SetupRGBCube();
  void SetupHSVCylinder();
  void SetupCIELab();
  
  vtkPoints* CurrentPoints;
  vtkPoints* NextPoints;
  DisplayPoints TransitionPoints;
  DisplayPoints RGBPoints;
  DisplayPoints HSVPoints;
  DisplayPoints CIELabPoints;

  vtkSmartPointer<vtkUnsignedCharArray> Colors;
  vtkSmartPointer<vtkRenderer> Renderer;

  unsigned int Spacing;

  float Transition; // This is the 'time' variable in the simulation
  unsigned int MaxNumberOfSteps;
  unsigned int MaxSpeed;
  unsigned int CurrentStep;

};

void OutputBounds(const std::string& name, double bounds[6]);

#endif
