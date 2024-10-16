/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCreateSurfaceTool.h"

#include "mitkCreateSurfaceTool.xpm"

#include "mitkProgressBar.h"
#include "mitkShowSegmentationAsSurface.h"
#include "mitkStatusBar.h"
#include "mitkToolManager.h"

#include "itkCommand.h"
#include <mitkToolManagerProvider.h>
namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, CreateSurfaceTool, "Surface creation tool");
}

mitk::CreateSurfaceTool::CreateSurfaceTool()
{
}

mitk::CreateSurfaceTool::~CreateSurfaceTool()
{
}

const char **mitk::CreateSurfaceTool::GetXPM() const
{
  return mitkCreateSurfaceTool_xpm;
}

const char *mitk::CreateSurfaceTool::GetName() const
{
  return "Surface";
}

std::string mitk::CreateSurfaceTool::GetErrorMessage()
{
  return "No surfaces created for these segmentations:";
}

bool mitk::CreateSurfaceTool::ProcessOneWorkingData(DataNode *node)
{
  if (node)
  {
    Image::Pointer image = dynamic_cast<Image *>(node->GetData());
    if (image.IsNull())
      return false;

    try
    {
      mitk::ShowSegmentationAsSurface::Pointer surfaceFilter = mitk::ShowSegmentationAsSurface::New();

      // attach observer to get notified about result
      itk::SimpleMemberCommand<CreateSurfaceTool>::Pointer goodCommand =
        itk::SimpleMemberCommand<CreateSurfaceTool>::New();
      goodCommand->SetCallbackFunction(this, &CreateSurfaceTool::OnSurfaceCalculationDone);
      surfaceFilter->AddObserver(mitk::ResultAvailable(), goodCommand);
      itk::SimpleMemberCommand<CreateSurfaceTool>::Pointer badCommand =
        itk::SimpleMemberCommand<CreateSurfaceTool>::New();
      badCommand->SetCallbackFunction(this, &CreateSurfaceTool::OnSurfaceCalculationDone);
      surfaceFilter->AddObserver(mitk::ProcessingError(), badCommand);

      DataNode::Pointer nodepointer = node;
      surfaceFilter->SetPointerParameter("Input", image);
      surfaceFilter->SetPointerParameter("Group node", nodepointer);
      surfaceFilter->SetParameter("Show result", true);
      surfaceFilter->SetParameter("Sync visibility", false);
      mitk::ToolManager *m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
      surfaceFilter->SetDataStorage(*m_ToolManager->GetDataStorage());

      ProgressBar::GetInstance()->AddStepsToDo(1);
      StatusBar::GetInstance()->DisplayText("Surface creation started in background...");
      surfaceFilter->StartAlgorithm();
    }
    catch (...)
    {
      return false;
    }
  }

  return true;
}

void mitk::CreateSurfaceTool::OnSurfaceCalculationDone()
{
  ProgressBar::GetInstance()->Progress();
  mitk::ToolManagerProvider::GetInstance()->GetToolManager()->NewNodesGenerated();
}
