/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFastMarchingTool_h_Included
#define mitkFastMarchingTool_h_Included

#include "mitkDataNode.h"
#include "mitkFeedbackContourTool.h"
#include "mitkPointSet.h"
#include "mitkToolCommand.h"
#include <MitkSegmentationExports.h>

#include "mitkMessage.h"

#include "itkImage.h"

// itk filter
#include "itkBinaryThresholdImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkFastMarchingImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class StateMachineAction;
  class InteractionEvent;

  /**
    \brief FastMarching semgentation tool.

    The segmentation is done by setting one or more seed points on the image
    and adapting the time range and threshold. The pipeline is:
      Smoothing->GradientMagnitude->SigmoidFunction->FastMarching->Threshold
    The resulting binary image is seen as a segmentation of an object.

    For detailed documentation see ITK Software Guide section 9.3.1 Fast Marching Segmentation.
  */
  class MITKSEGMENTATION_EXPORT FastMarchingTool : public FeedbackContourTool
  {
    mitkNewMessageMacro(Ready);

  public:
    mitkClassMacro(FastMarchingTool, FeedbackContourTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /* typedefs for itk pipeline */
      typedef float InternalPixelType;
    typedef itk::Image<InternalPixelType, 2> InternalImageType;
    typedef mitk::Tool::DefaultSegmentationDataType OutputPixelType;
    typedef itk::Image<OutputPixelType, 2> OutputImageType;

    typedef itk::BinaryThresholdImageFilter<InternalImageType, OutputImageType> ThresholdingFilterType;
    typedef itk::CurvatureAnisotropicDiffusionImageFilter<InternalImageType, InternalImageType> SmoothingFilterType;
    typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<InternalImageType, InternalImageType> GradientFilterType;
    typedef itk::SigmoidImageFilter<InternalImageType, InternalImageType> SigmoidFilterType;
    typedef itk::FastMarchingImageFilter<InternalImageType, InternalImageType> FastMarchingFilterType;
    typedef FastMarchingFilterType::NodeContainer NodeContainer;
    typedef FastMarchingFilterType::NodeType NodeType;

    /* icon stuff */
    const char **GetXPM() const override;
    const char *GetName() const override;

    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    /// \brief Set parameter used in Threshold filter.
    void SetUpperThreshold(double);

    /// \brief Set parameter used in Threshold filter.
    void SetLowerThreshold(double);

    /// \brief Set parameter used in Fast Marching filter.
    void SetStoppingValue(double);

    /// \brief Set parameter used in Gradient Magnitude filter.
    void SetSigma(double);

    /// \brief Set parameter used in Fast Marching filter.
    void SetAlpha(double);

    /// \brief Set parameter used in Fast Marching filter.
    void SetBeta(double);

    /// \brief Adds the feedback image to the current working image.
    virtual void ConfirmSegmentation();

    /// \brief Set the working time step.
    virtual void SetCurrentTimeStep(int t);

    /// \brief Clear all seed points.
    void ClearSeeds();

    /// \brief Updates the itk pipeline and shows the result of FastMarching.
    void Update();

  protected:
    FastMarchingTool();
    ~FastMarchingTool() override;

    void ConnectActionsAndFunctions() override;

    // virtual float CanHandleEvent( StateEvent const *stateEvent) const;

    void Activated() override;
    void Deactivated() override;
    virtual void Initialize();

    virtual void BuildITKPipeline();

    /// \brief Add point action of StateMachine pattern
    virtual void OnAddPoint(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Delete action of StateMachine pattern
    virtual void OnDelete(StateMachineAction *, InteractionEvent *interactionEvent);

    /// \brief Reset all relevant inputs of the itk pipeline.
    void Reset();

    mitk::ToolCommand::Pointer m_ProgressCommand;

    Image::Pointer m_ReferenceImage;
    Image::Pointer m_ReferenceImageSlice;

    bool m_NeedUpdate;

    int m_CurrentTimeStep;

    mitk::InteractionPositionEvent::Pointer m_PositionEvent;

    float m_LowerThreshold; // used in Threshold filter
    float m_UpperThreshold; // used in Threshold filter
    float m_StoppingValue;  // used in Fast Marching filter
    float m_Sigma;          // used in GradientMagnitude filter
    float m_Alpha;          // used in Sigmoid filter
    float m_Beta;           // used in Sigmoid filter

    NodeContainer::Pointer m_SeedContainer; // seed points for FastMarching

    InternalImageType::Pointer m_ReferenceImageSliceAsITK; // the reference image as itk::Image

    mitk::DataNode::Pointer m_ResultImageNode; // holds the result as a preview image

    mitk::DataNode::Pointer m_SeedsAsPointSetNode; // used to visualize the seed points
    mitk::PointSet::Pointer m_SeedsAsPointSet;

    ThresholdingFilterType::Pointer m_ThresholdFilter;
    SmoothingFilterType::Pointer m_SmoothFilter;
    GradientFilterType::Pointer m_GradientMagnitudeFilter;
    SigmoidFilterType::Pointer m_SigmoidFilter;
    FastMarchingFilterType::Pointer m_FastMarchingFilter;

  private:
    PlaneGeometry::Pointer m_WorkingPlane;
    mitk::ToolManager *m_ToolManager;
  };

} // namespace

#endif
