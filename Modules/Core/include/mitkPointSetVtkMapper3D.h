/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetVtkMapper3D_h
#define mitkPointSetVtkMapper3D_h

#include "mitkBaseRenderer.h"
#include "mitkVtkMapper.h"
#include <MitkCoreExports.h>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkCellArray;
class vtkPropAssembly;
class vtkAppendPolyData;
class vtkPolyData;
class vtkTubeFilter;
class vtkPolyDataMapper;
class vtkTransformPolyDataFilter;

namespace mitk
{
  class PointSet;

  /**
  * @brief Vtk-based mapper for PointSet
  *
  * Due to the need of different colors for selected
  * and unselected points and the facts, that we also have a contour and
  * labels for the points, the vtk structure is build up the following way:
  *
  * We have two AppendPolyData, one selected, and one unselected and one
  * for a contour between the points. Each one is connected to an own
  * PolyDaraMapper and an Actor. The different color for the unselected and
  * selected state and for the contour is read from properties.
  *
  * "unselectedcolor", "selectedcolor" and "contourcolor" are the strings,
  * that are looked for. Pointlabels are added besides the selected or the
  * deselected points.
  *
  * Then the three Actors are combined inside a vtkPropAssembly and this
  * object is returned in GetProp() and so hooked up into the rendering
  * pipeline.

  * Properties that can be set for point sets and influence the PointSetVTKMapper3D are:
  *

  *   - \b "color": (ColorProperty*) Color of the point set
  *   - \b "Opacity": (FloatProperty) Opacity of the point set
  *   - \b "show contour": (BoolProperty) If the contour of the points are visible
  *   - \b "contourSizeProp":(FloatProperty) Contour size of the points


  The default properties are:

  *   - \b "line width": (IntProperty::New(2), renderer, overwrite )
  *   - \b "pointsize": (FloatProperty::New(1.0), renderer, overwrite)
  *   - \b "selectedcolor": (ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite)  //red
  *   - \b "color": (ColorProperty::New(1.0f, 1.0f, 0.0f), renderer, overwrite)  //yellow
  *   - \b "show contour": (BoolProperty::New(false), renderer, overwrite )
  *   - \b "contourcolor": (ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite)
  *   - \b "contoursize": (FloatProperty::New(0.5), renderer, overwrite )
  *   - \b "close contour": (BoolProperty::New(false), renderer, overwrite )
  *   - \b "show points": (BoolProperty::New(true), renderer, overwrite )
  *   - \b "updateDataOnRender": (BoolProperty::New(true), renderer, overwrite )



  *Other properties looked for are:
  *
  *   - \b "show contour": if set to on, lines between the points are shown
  *   - \b "close contour": if set to on, the open strip is closed (first point
  *       connected with last point)
  *   - \b "pointsize": size of the points mapped (diameter of a sphere, in world woordinates!)
  *   - \b "label": text of the Points to show besides points
  *   - \b "contoursize": size of the contour drawn between the points
  *       (if not set, the pointsize is taken)
  *
  * @ingroup Mapper
  */
  class MITKCORE_EXPORT PointSetVtkMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(PointSetVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      virtual const mitk::PointSet *GetInput();

    // overwritten from VtkMapper3D to be able to return a
    // m_PointsAssembly which is much faster than a vtkAssembly
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;
    void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;

    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    /*
    * \deprecatedSince{2013_12} Use ReleaseGraphicsResources(mitk::BaseRenderer* renderer) instead
    */
    //DEPRECATED(void ReleaseGraphicsResources(vtkWindow *renWin));

    void ReleaseGraphicsResources(mitk::BaseRenderer *renderer) override;

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3D render windows */
    class MITKCORE_EXPORT LocalStorage : public BaseLocalStorage
    {
    public:
        /* constructor */
        LocalStorage();

        /* destructor */
        ~LocalStorage();

        /// All point positions, already in world coordinates
        vtkSmartPointer<vtkPoints> m_WorldPositions;
        /// All connections between two points (used for contour drawing)
        vtkSmartPointer<vtkCellArray> m_PointConnections;

        vtkSmartPointer<vtkAppendPolyData> m_vtkSelectedPointList;
        vtkSmartPointer<vtkAppendPolyData> m_vtkUnselectedPointList;
        vtkSmartPointer<vtkAppendPolyData> m_vtkHidedPointList;
        vtkSmartPointer<vtkAppendPolyData> m_vtkMarkedPointList;

        vtkSmartPointer<vtkPoints> m_VtkPoints;
        vtkSmartPointer<vtkCellArray> m_VtkPointConnections;

        vtkSmartPointer<vtkTransformPolyDataFilter> m_VtkPointsTransformer;

        vtkSmartPointer<vtkPolyDataMapper> m_VtkSelectedPolyDataMapper;
        vtkSmartPointer<vtkPolyDataMapper> m_VtkUnselectedPolyDataMapper;
        vtkSmartPointer<vtkPolyDataMapper> m_VtkHidedPolyDataMapper;
        vtkSmartPointer<vtkPolyDataMapper> m_VtkMarkedPolyDataMapper;

        vtkSmartPointer<vtkActor> m_SelectedActor;
        vtkSmartPointer<vtkActor> m_UnselectedActor;
        vtkSmartPointer<vtkActor> m_HidedActor;
        vtkSmartPointer<vtkActor> m_MarkedActor;
        vtkSmartPointer<vtkActor> m_ContourActor;

        vtkSmartPointer<vtkPropAssembly> m_PointsAssembly;

        // variables to be able to log, how many inputs have been added to PolyDatas
        unsigned int m_NumberOfSelectedAdded;
        unsigned int m_NumberOfUnselectedAdded;
        unsigned int m_NumberOfMarkedAdded;
        unsigned int m_NumberOfHidedAdded;
    };
    //LocalStorageHandler<BaseLocalStorage> m_LSH;
    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;
  protected:
    PointSetVtkMapper3D();

    ~PointSetVtkMapper3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
    void ResetMapper(BaseRenderer *renderer) override;
    virtual void ApplyAllProperties(mitk::BaseRenderer *renderer, vtkActor *actor);
    virtual void CreateContour(vtkPoints *points, vtkCellArray *connections, mitk::BaseRenderer* renderer);
    virtual void CreateVTKRenderObjects(mitk::BaseRenderer* renderer);

    // help for contour between points
    vtkSmartPointer<vtkAppendPolyData> m_vtkTextList;

    // variables to check if an update of the vtk objects is needed
    ScalarType m_PointSize;
    ScalarType m_ContourRadius;
  };

} // namespace mitk

#endif
