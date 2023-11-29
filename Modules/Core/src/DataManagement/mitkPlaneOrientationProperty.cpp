/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlaneOrientationProperty.h"

namespace mitk
{
  PlaneOrientationProperty::PlaneOrientationProperty()
  {
    this->AddDecorationTypes();
    this->SetValue(static_cast<IdType>(PLANE_DECORATION_NONE));
  }

  PlaneOrientationProperty::PlaneOrientationProperty(const IdType &value)
  {
    this->AddDecorationTypes();
    if (this->IsValidEnumerationValue(value))
    {
      this->SetValue(value);
    }
    else
    {
      this->SetValue(static_cast<IdType>(PLANE_DECORATION_NONE));
    }
  }

  PlaneOrientationProperty::PlaneOrientationProperty(const std::string &value)
  {
    this->AddDecorationTypes();
    if (this->IsValidEnumerationValue(value))
    {
      this->SetValue(value);
    }
    else
    {
      this->SetValue(static_cast<IdType>(PLANE_DECORATION_NONE));
    }
  }

  int PlaneOrientationProperty::GetPlaneDecoration() { return static_cast<int>(this->GetValueAsId()); }
  void PlaneOrientationProperty::SetPlaneDecorationToNone()
  {
    this->SetValue(static_cast<IdType>(PLANE_DECORATION_NONE));
  }

  void PlaneOrientationProperty::SetPlaneDecorationToPositiveOrientation()
  {
    this->SetValue(static_cast<IdType>(PLANE_DECORATION_POSITIVE_ORIENTATION));
  }

  void PlaneOrientationProperty::SetPlaneDecorationToNegativeOrientation()
  {
    this->SetValue(static_cast<IdType>(PLANE_DECORATION_NEGATIVE_ORIENTATION));
  }

  void PlaneOrientationProperty::AddDecorationTypes()
  {
    this->AddEnum("No plane decoration", static_cast<IdType>(PLANE_DECORATION_NONE));
    this->AddEnum("Arrows in positive direction", static_cast<IdType>(PLANE_DECORATION_POSITIVE_ORIENTATION));
    this->AddEnum("Arrows in negative direction", static_cast<IdType>(PLANE_DECORATION_NEGATIVE_ORIENTATION));
  }

  bool PlaneOrientationProperty::AddEnum(const std::string &name, const IdType &id)
  {
    return Superclass::AddEnum(name, id);
  }

  itk::LightObject::Pointer PlaneOrientationProperty::InternalClone() const
  {
    itk::LightObject::Pointer result(new Self(*this));
    result->UnRegister();
    return result;
  }

} // namespace
