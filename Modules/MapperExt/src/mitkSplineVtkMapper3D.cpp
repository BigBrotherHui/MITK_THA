/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSplineVtkMapper3D.h"
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <vtkActor.h>
#include <vtkCardinalSpline.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkPropAssembly.h>
#include <vtkPropCollection.h>
#include <vtkProperty.h>
#include <vtkTubeFilter.h>

mitk::SplineVtkMapper3D::LocalStorage::LocalStorage() : m_SplinesAddedToAssembly(false),m_SplinesAvailable(false)
{
  m_SplinesActor = vtkActor::New();
  m_SplineAssembly = vtkPropAssembly::New();
  profileMapper = vtkPolyDataMapper::New();
  m_SplinesActor->SetMapper(profileMapper);
}

mitk::SplineVtkMapper3D::LocalStorage::~LocalStorage()
{
  m_SplinesActor->Delete();
  m_SplineAssembly->Delete();
  profileMapper->Delete();
}

mitk::SplineVtkMapper3D::SplineVtkMapper3D() 
{
    m_SplineResolution = 500;
}

mitk::SplineVtkMapper3D::~SplineVtkMapper3D()
{

}

vtkProp *mitk::SplineVtkMapper3D::GetVtkProp(mitk::BaseRenderer * renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_SplineAssembly;
}

void mitk::SplineVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer * renderer)
{
  vtkLinearTransform *vtktransform = this->GetDataNode()->GetVtkTransform(this->GetTimestep());
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ls->m_SplinesActor->SetUserTransform(vtktransform);
}

void mitk::SplineVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  
  // only update spline if UpdateSpline has not been called from
  // external, e.g. by the SplineMapper2D. But call it the first time when m_SplineUpdateTime = 0 and
  // 0.
  // if (m_SplineUpdateTime < ls->GetLastGenerateDataTime() || m_SplineUpdateTime == 0)
  // {
    this->UpdateSpline(renderer);
    this->ApplyAllProperties(renderer, ls->m_SplinesActor);
  // }
  if (ls->m_SplinesAvailable)
  {
    if (!ls->m_SplinesAddedToAssembly)
    {
      ls->m_SplineAssembly->AddPart(ls->m_SplinesActor);
      ls->m_SplinesAddedToAssembly = true;
    }
  }
  else
  {
    if (ls->m_SplinesAddedToAssembly)
    {
      ls->m_SplineAssembly->RemovePart(ls->m_SplinesActor);
      ls->m_SplinesAddedToAssembly = false;
    }
  }
  
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");
  
  if (!visible)
  {
    ls->m_SplinesActor->VisibilityOff();
    ls->m_SplineAssembly->VisibilityOff();
  }
  else
  {
    ls->m_SplinesActor->VisibilityOn();
    ls->m_SplineAssembly->VisibilityOn();
  
    // remove the PointsAssembly if it was added in superclass. No need to display points and spline!
    if (ls->m_SplineAssembly->GetParts()->IsItemPresent(ls->m_PointsAssembly))
      ls->m_SplineAssembly->RemovePart(ls->m_PointsAssembly);
  }
  // if the properties have been changed, then refresh the properties
  if ((m_SplineUpdateTime < this->m_DataNode->GetPropertyList()->GetMTime()) ||
      (m_SplineUpdateTime < this->m_DataNode->GetPropertyList(renderer)->GetMTime()))
    this->ApplyAllProperties(renderer, ls->m_SplinesActor);
}

void mitk::SplineVtkMapper3D::ApplyAllProperties(BaseRenderer *renderer, vtkActor *actor)
{
  Superclass::ApplyColorAndOpacityProperties(renderer, actor);

  // vtk changed the type of rgba during releases. Due to that, the following convert is done
  double rgba[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // white

  // getting the color from DataNode
  float temprgba[4];
  this->GetDataNode()->GetColor(&temprgba[0], nullptr);
  // convert to rgba, what ever type it has!
  rgba[0] = temprgba[0];
  rgba[1] = temprgba[1];
  rgba[2] = temprgba[2];
  rgba[3] = temprgba[3];
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  // finaly set the color inside the actor
  ls->m_SplinesActor->GetProperty()->SetColor(rgba);

  float lineWidth;
  if (dynamic_cast<mitk::IntProperty *>(this->GetDataNode()->GetProperty("line width")) == nullptr)
    lineWidth = 1.0;
  else
    lineWidth = dynamic_cast<mitk::IntProperty *>(this->GetDataNode()->GetProperty("line width"))->GetValue();
  ls->m_SplinesActor->GetProperty()->SetLineWidth(lineWidth);

  m_SplineUpdateTime.Modified();
}

bool mitk::SplineVtkMapper3D::SplinesAreAvailable(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_SplinesAvailable;
}

vtkPolyData *mitk::SplineVtkMapper3D::GetSplinesPolyData(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  if (ls->m_SplinesAvailable)
    return (dynamic_cast<vtkPolyDataMapper *>(ls->m_SplinesActor->GetMapper()))->GetInput();
  else
    return nullptr;
}

vtkActor *mitk::SplineVtkMapper3D::GetSplinesActor(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  if (ls->m_SplinesAvailable)
    return ls->m_SplinesActor;
  else
    return vtkActor::New();
}

void mitk::SplineVtkMapper3D::UpdateSpline(mitk::BaseRenderer *renderer)
{
  auto input = this->GetInput();
  //  input->Update();//already done in superclass
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  // Number of points on the spline
  unsigned int numberOfOutputPoints = m_SplineResolution;
  unsigned int numberOfInputPoints = input->GetSize();
  if (numberOfInputPoints >= 2)
  {
    ls->m_SplinesAvailable = true;
    vtkCardinalSpline *splineX = vtkCardinalSpline::New();
    vtkCardinalSpline *splineY = vtkCardinalSpline::New();
    vtkCardinalSpline *splineZ = vtkCardinalSpline::New();
    unsigned int index = 0;
    mitk::PointSet::DataType::PointsContainer::Pointer pointsContainer = input->GetPointSet()->GetPoints();
    for (mitk::PointSet::DataType::PointsContainer::Iterator it = pointsContainer->Begin();
         it != pointsContainer->End();
         ++it, ++index)
    // for ( unsigned int i = 0 ; i < numberOfInputPoints; ++i )
    {
      mitk::PointSet::PointType point = it->Value();
      splineX->AddPoint(index, point[0]);
      splineY->AddPoint(index, point[1]);
      splineZ->AddPoint(index, point[2]);
    }
    vtkPoints *points = vtkPoints::New();
    vtkPolyData *profileData = vtkPolyData::New();

    // Interpolate x, y and z by using the three spline filters and
    // create new points
    double t = 0.0f;
    for (unsigned int i = 0; i < numberOfOutputPoints; ++i)
    {
      t = ((((double)numberOfInputPoints) - 1.0f) / (((double)numberOfOutputPoints) - 1.0f)) * ((double)i);
      points->InsertPoint(i, splineX->Evaluate(t), splineY->Evaluate(t), splineZ->Evaluate(t));
    }

    // Create the polyline.
    vtkCellArray *lines = vtkCellArray::New();
    lines->InsertNextCell(numberOfOutputPoints);
    for (unsigned int i = 0; i < numberOfOutputPoints; ++i)
      lines->InsertCellPoint(i);

    profileData->SetPoints(points);
    profileData->SetLines(lines);

    // Add thickness to the resulting line.
    // vtkTubeFilter* profileTubes = vtkTubeFilter::New();
    // profileTubes->SetNumberOfSides(8);
    // profileTubes->SetInput(profileData);
    // profileTubes->SetRadius(.005);
    ls->profileMapper->SetInputData(profileData);
  }
  else
  {
    ls->m_SplinesAvailable = false;
  }
  m_SplineUpdateTime.Modified();
}