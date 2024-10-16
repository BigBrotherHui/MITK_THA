/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTextAnnotation3D.h"
#include <vtkCamera.h>
#include <vtkFollower.h>
#include <vtkMath.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTextActor3D.h>
#include <vtkTextProperty.h>
#include <vtkVectorText.h>

mitk::TextAnnotation3D::TextAnnotation3D()
{
  mitk::Point3D position;
  position.Fill(0);
  m_followerPosition.Fill(0);
  this->SetPosition3D(position);
  this->SetOffsetVector(position);
  this->SetText("");
  this->SetFontSize(20);
  this->SetColor(1.0, 1.0, 1.0);
}

mitk::TextAnnotation3D::~TextAnnotation3D()
{
  for (BaseRenderer *renderer : m_LSH.GetRegisteredBaseRenderer())
  {
    if (renderer)
    {
      this->RemoveFromBaseRenderer(renderer);
    }
  }
}

mitk::TextAnnotation3D::LocalStorage::~LocalStorage()
{
}

mitk::TextAnnotation3D::LocalStorage::LocalStorage()
{
  // Create some text
  m_textSource = vtkSmartPointer<vtkVectorText>::New();

  // Create a mapper
  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(m_textSource->GetOutputPort());

  // Create a subclass of vtkActor: a vtkFollower that remains facing the camera
  m_follower = vtkSmartPointer<vtkFollower>::New();
  m_follower->SetMapper(mapper);
  m_follower->GetProperty()->SetColor(1, 0, 0); // red
  m_follower->SetScale(1);
}

mitk::Point3D mitk::TextAnnotation3D::GetFollowerPosition(){
  return m_followerPosition;
}

void mitk::TextAnnotation3D::UpdateVtkAnnotation(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  if (ls->IsGenerateDataRequired(renderer, this))
  {
    Point3D pos3d = GetPosition3D();
    vtkRenderer *vtkRender = renderer->GetVtkRenderer();
    if (vtkRender)
    {
      vtkCamera *camera = vtkRender->GetActiveCamera();
      ls->m_follower->SetCamera(camera);
      if (camera != nullptr)
      {
        // calculate the offset relative to the camera's view direction
        Point3D offset = GetOffsetVector();

        Vector3D viewUp;
        camera->GetViewUp(viewUp.GetDataPointer());
        Vector3D cameraDirection;
        camera->GetDirectionOfProjection(cameraDirection.GetDataPointer());
        Vector3D viewRight;
        vtkMath::Cross(cameraDirection.GetDataPointer(), viewUp.GetDataPointer(), viewRight.GetDataPointer());

        pos3d = pos3d + viewRight * offset[0] + viewUp * offset[1] + cameraDirection * offset[2];
        m_followerPosition = pos3d - viewRight*GetFontSize() * GetText().length() / 2;
      }
    }
    ls->m_follower->SetPosition(pos3d.GetDataPointer());
    ls->m_textSource->SetText(GetText().c_str());
    float color[3] = {1, 1, 1};
    float opacity = 1.0;
    GetColor(color);
    GetOpacity(opacity);
    ls->m_follower->GetProperty()->SetColor(color[0], color[1], color[2]);
    ls->m_follower->GetProperty()->SetOpacity(opacity);
    ls->m_follower->SetScale(this->GetFontSize());
    ls->UpdateGenerateDataTime();
  }
}

vtkProp *mitk::TextAnnotation3D::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_follower;
}
