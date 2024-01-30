#include <algorithm>
#include <limits>

#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkImageInterpolator.h>
#include <vtkTetra.h>
#include <vtkMetaImageWriter.h>
#include <vtkUnstructuredGridGeometryFilter.h>
#include <vtkExtractVOI.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageContinuousErode3D.h>
#include <vtkImageLogic.h>
#include <vtkImageMathematics.h>
#include <vtkImageConvolve.h>
#include <vtkImageCast.h>
#include <vtkImageConstantPad.h>

#include <mitkProgressBar.h>

#include "MaterialMappingFilter.h"

MaterialMappingFilter::MaterialMappingFilter()
        : m_PointArrayName("E"),
          m_CellArrayName("E"),
		  m_Method(Method::New)
{
}

void MaterialMappingFilter::GenerateData()
{
	mitk::UnstructuredGrid::Pointer inputGrid = const_cast<mitk::UnstructuredGrid *>(this->GetInput());
	if (inputGrid.IsNull() || m_IntensityImage == nullptr || m_IntensityImage.IsNull())
	{
		return;
	}

	mitk::ProgressBar::GetInstance()->AddStepsToDo(7);

	auto importedVtkImage = const_cast<vtkImageData *>(m_IntensityImage->GetVtkImageData());
	vtkSmartPointer<vtkUnstructuredGrid> vtkInputGrid = inputGrid->GetVtkUnstructuredGrid();

	MITK_INFO("ch.zhaw.materialmapping") << "density functors";
	MITK_INFO("ch.zhaw.materialmapping") << m_BoneDensityFunctor;
	MITK_INFO("ch.zhaw.materialmapping") << m_PowerLawFunctor;

	MITK_INFO("ch.zhaw.materialmapping") << "material mapping parameters";
	MITK_INFO("ch.zhaw.materialmapping") << "peel step: " << m_DoPeelStep;
	MITK_INFO("ch.zhaw.materialmapping") << "image extend: " << m_NumberOfExtendImageSteps;
	MITK_INFO("ch.zhaw.materialmapping") << "minimum element value: " << m_MinimumElementValue;
	MITK_INFO("ch.zhaw.materialmapping") << "method: " << (m_Method == Method::Old ? "old" : "new");

	// Bug in mitk::Image::GetVtkImageData(), Origin is wrong
	// http://bugs.mitk.org/show_bug.cgi?id=5050
	// since the memory is shared between vtk and mitk, manually correcting it will break rendering. For now,
	// we'll create a copy and work with that.
	// TODO: keep an eye on this
	auto mitkOrigin = m_IntensityImage->GetGeometry()->GetOrigin();
	auto vtkImage = vtkSmartPointer<vtkImageData>::New();
	vtkImage->ShallowCopy(importedVtkImage);
	vtkImage->SetOrigin(mitkOrigin[0], mitkOrigin[1], mitkOrigin[2]);

	if (m_VerboseOutput)
	{
		writeMetaImageToVerboseOut("01_ct_input.mhd", vtkImage);
	}

	// we would not gain anything by handling integer type scalars individually, so it's easier to work with floats from the beginning
	auto imageCast = vtkSmartPointer<vtkImageCast>::New();
	imageCast->SetInputData(vtkImage);
	imageCast->SetOutputScalarTypeToFloat();
	imageCast->Update();
	vtkImage = imageCast->GetOutput();
	mitk::ProgressBar::GetInstance()->Progress();

	if (m_VerboseOutput)
	{
		writeMetaImageToVerboseOut("02_ct_casted.mhd", vtkImage);
	}

	auto surface = extractSurface(vtkInputGrid);
	auto voi = extractVOI(vtkImage, surface);

	if (m_VerboseOutput)
	{
		writeMetaImageToVerboseOut("03_ct_voi.mhd", vtkImage);
	}

	inplaceApplyFunctorsToImage(voi);
	mitk::ProgressBar::GetInstance()->Progress();

	// pad with 0 slices
	auto padder = vtkSmartPointer<vtkImageConstantPad>::New();
	auto extent = voi->GetExtent();
	auto border = static_cast<int>(m_NumberOfExtendImageSteps + 1);
	int paddedExtents[6];
	for (auto i = 0; i < 6; ++i)
	{
		paddedExtents[i] = extent[i] + border * (2 * (i % 2) - 1);
	}
	padder->SetInputData(voi);
	padder->SetOutputWholeExtent(paddedExtents);
	padder->SetConstant(0);
	padder->Update();
	voi = padder->GetOutput();

	VtkImage stencil;
	stencil = createStencil(surface, voi);
	mitk::ProgressBar::GetInstance()->Progress();


	MaterialMappingFilter::VtkImage mask;
	if (m_DoPeelStep)
	{
		mask = createPeeledMask(voi, stencil);
	}
	else
	{
		mask = stencil;
	}
	mitk::ProgressBar::GetInstance()->Progress();

	if (m_VerboseOutput)
	{
		writeMetaImageToVerboseOut("04_e_voi.mhd", voi);
		writeMetaImageToVerboseOut("05_stencil.mhd", stencil);
		writeMetaImageToVerboseOut("06_peeled_mask.mhd", mask);
	}

	for (auto i = 0u; i < m_NumberOfExtendImageSteps; ++i)
	{
		switch (m_Method)
		{
		case Method::Old:
			{
				inplaceExtendImageOld(voi, mask, true);
				break;
			}

		case Method::New:
			{
				inplaceExtendImage(voi, mask, true);
				break;
			}
		}

		if (m_VerboseOutput)
		{
			writeMetaImageToVerboseOut("07_peeled_mask_extended_" + std::to_string(i) + ".mhd", mask);
			writeMetaImageToVerboseOut("08_e_voi_extended_" + std::to_string(i) + ".mhd", voi);
		}
	}
	mitk::ProgressBar::GetInstance()->Progress();

    // create ouput
    auto out = vtkSmartPointer<vtkUnstructuredGrid>::New();
    out->DeepCopy(vtkInputGrid);

	auto nodeDataE = interpolateToNodes(vtkInputGrid, voi, m_PointArrayName, m_MinimumElementValue);
	if(m_PointArrayName != "")
	{
		out->GetPointData()->AddArray(nodeDataE);
	}

    mitk::ProgressBar::GetInstance()->Progress();

    if(m_CellArrayName != "")
    {
        auto elementDataE = nodesToElements(vtkInputGrid, nodeDataE, m_CellArrayName);
        out->GetCellData()->AddArray(elementDataE);
    }
    mitk::ProgressBar::GetInstance()->Progress();

	this->GetOutput()->SetVtkUnstructuredGrid(out);
}

MaterialMappingFilter::VtkUGrid MaterialMappingFilter::extractSurface(const VtkUGrid _volMesh) const
{
	auto surfaceFilter = vtkSmartPointer<vtkUnstructuredGridGeometryFilter>::New();
	surfaceFilter->SetInputData(_volMesh);
	surfaceFilter->PassThroughCellIdsOn();
	surfaceFilter->PassThroughPointIdsOn();
	surfaceFilter->MergingOff();
	surfaceFilter->Update();
	return surfaceFilter->GetOutput();
}

MaterialMappingFilter::VtkImage MaterialMappingFilter::extractVOI(const VtkImage _img, const VtkUGrid _surMesh) const
{
	auto voi = vtkSmartPointer<vtkExtractVOI>::New();
	auto spacing = _img->GetSpacing();
	auto origin = _img->GetOrigin();
	auto extent = _img->GetExtent();
	auto bounds = _surMesh->GetBounds();

	auto clamp = [](double x, int a, int b)
		{
			return x < a ? a : (x > b ? b : x);
		};

	auto border = static_cast<int>(m_NumberOfExtendImageSteps + 1);

	int voiExt[6];
	for (auto i = 0; i < 2; ++i)
	{
		for (auto j = 0; j < 3; ++j)
		{
			auto val = (bounds[i + 2 * j] - origin[j]) / spacing[j] + (2 * i - 1) * border; // coordinate -> index
			voiExt[i + 2 * j] = clamp(val, extent[2 * j], extent[2 * j + 1]); // prevent wrap around
		}
	}
	voi->SetVOI(voiExt);
	voi->SetInputData(_img);
	voi->Update();

	// vtkExtractVOI has unexpected output scalar types. see https://github.com/araex/mitk-gem/issues/13
	if (voi->GetOutput()->GetScalarType() != _img->GetScalarType())
	{
		MITK_WARN("ch.zhaw.materialmapping") << "vtkExtractVOI produced an unexpected scalar type. Correcting...";
		auto imageCast = vtkSmartPointer<vtkImageCast>::New();
		imageCast->SetInputData(voi->GetOutput());
		imageCast->SetOutputScalarType(_img->GetScalarType());
		imageCast->Update();
		return imageCast->GetOutput();
	}
	else
	{
		return voi->GetOutput();
	}
}

MaterialMappingFilter::VtkImage MaterialMappingFilter::createStencil(const VtkUGrid _surMesh, const VtkImage _img) const
{
	// configure
	auto gridToPolyDataFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();

	auto polyDataToStencilFilter = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
	polyDataToStencilFilter->SetOutputSpacing(_img->GetSpacing());
	polyDataToStencilFilter->SetOutputOrigin(_img->GetOrigin());

	auto blankImage = vtkSmartPointer<vtkImageData>::New();
	blankImage->CopyStructure(_img);
	blankImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
	unsigned char* p = (unsigned char *) (blankImage->GetScalarPointer());
	for (auto i = 0; i < blankImage->GetNumberOfPoints(); i++)
	{
		p[i] = 0;
	}

	auto stencil = vtkSmartPointer<vtkImageStencil>::New();
	stencil->ReverseStencilOn();
	stencil->SetBackgroundValue(1);

	// pipeline
	gridToPolyDataFilter->SetInputData(_surMesh);
	polyDataToStencilFilter->SetInputConnection(gridToPolyDataFilter->GetOutputPort());
	stencil->SetInputData(blankImage);
	stencil->SetStencilConnection(polyDataToStencilFilter->GetOutputPort());
	stencil->Update();
	return stencil->GetOutput();
}

MaterialMappingFilter::VtkImage MaterialMappingFilter::createPeeledMask(const VtkImage _img, const VtkImage _mask)
{
	// configure
	auto erodeFilter = vtkSmartPointer<vtkImageContinuousErode3D>::New();
	switch (m_Method)
	{
	case Method::Old:
		{
			erodeFilter->SetKernelSize(3, 3, 1);
			break;
		}

	case Method::New:
		{
			erodeFilter->SetKernelSize(3, 3, 3);
			break;
		}
	}

	auto xorLogic = vtkSmartPointer<vtkImageLogic>::New();
	xorLogic->SetOperationToXor();
	xorLogic->SetOutputTrueValue(1);

	// pipeline
	erodeFilter->SetInputData(_mask);
	erodeFilter->Update();
	xorLogic->SetInput1Data(_mask);
	xorLogic->SetInput2Data(erodeFilter->GetOutput());
	xorLogic->Update();

	// extend
	auto imgCopy = vtkSmartPointer<vtkImageData>::New();
	imgCopy->DeepCopy(_img);
	auto erodedMaskCopy = vtkSmartPointer<vtkImageData>::New();
	erodedMaskCopy->DeepCopy(erodeFilter->GetOutput());

	switch (m_Method)
	{
	case Method::Old:
		{
			inplaceExtendImageOld(imgCopy, erodedMaskCopy, true);
			break;
		}

	case Method::New:
		{
			inplaceExtendImage(imgCopy, erodedMaskCopy, true);
			break;
		}
	}

	unsigned char* peelPoints = (unsigned char *) xorLogic->GetOutput()->GetScalarPointer();
	unsigned char* corePoints = (unsigned char *) erodeFilter->GetOutput()->GetScalarPointer();
	float* im = (float *) _img->GetScalarPointer();
	float* tim = (float *) imgCopy->GetScalarPointer();
	for (auto i = 0; i < _img->GetNumberOfPoints(); i++)
	{
		if (peelPoints[i] && im[i] > tim[i])
		{
			corePoints[i] = 1;
		}
	}

	return erodeFilter->GetOutput();
}

void MaterialMappingFilter::inplaceExtendImage(VtkImage _img, VtkImage _mask, bool _maxval)
{
	assert(_img->GetScalarType() == VTK_FLOAT && "Input image scalar type needs to be float!");

	static const double kernel[27] = {
		1 / sqrt(3), 1 / sqrt(2), 1 / sqrt(3),
		1 / sqrt(2), 1, 1 / sqrt(2),
		1 / sqrt(3), 1 / sqrt(2), 1 / sqrt(3),
		1 / sqrt(2), 1, 1 / sqrt(2),
		1, 0, 1,
		1 / sqrt(2), 1, 1 / sqrt(2),
		1 / sqrt(3), 1 / sqrt(2), 1 / sqrt(3),
		1 / sqrt(2), 1, 1 / sqrt(2),
		1 / sqrt(3), 1 / sqrt(2), 1 / sqrt(3)
	};

	auto math = vtkSmartPointer<vtkImageMathematics>::New();
	auto imageconv = vtkSmartPointer<vtkImageConvolve>::New();
	auto maskconv = vtkSmartPointer<vtkImageConvolve>::New();

	// vtkImageMathematics needs both inputs to have the same scalar type
	auto mask_float = vtkSmartPointer<vtkImageData>::New();
	mask_float->CopyStructure(_mask);
	mask_float->AllocateScalars(VTK_FLOAT, 1);
	for (auto i = 0; i < mask_float->GetNumberOfPoints(); i++)
	{
		mask_float->GetPointData()->GetScalars()->SetTuple1(i, _mask->GetPointData()->GetScalars()->GetTuple1(i));
	}

	math->SetOperationToMultiply();
	math->SetInput1Data(_img);
	math->SetInput2Data(mask_float);
	imageconv->SetKernel3x3x3(kernel);
	imageconv->SetInputConnection(math->GetOutputPort());
	imageconv->Update();
	maskconv->SetKernel3x3x3(kernel);
	maskconv->SetInputData(mask_float);
	maskconv->Update();
	auto maskPoints = (unsigned char *) (_mask->GetScalarPointer());
	auto convMaskPoints = (float *) (maskconv->GetOutput()->GetScalarPointer());
	auto convImgPoints = (float *) (imageconv->GetOutput()->GetScalarPointer());
	auto imagePoints = (float *) (_img->GetScalarPointer());

	for (auto i = 0; i < _img->GetNumberOfPoints(); i++)
	{
		if (convMaskPoints[i] && !maskPoints[i])
		{
			auto val = convImgPoints[i] / convMaskPoints[i];
			if (_maxval)
			{
				if (imagePoints[i] < val)
				{
					imagePoints[i] = val;
				}
			}
			else
			{
				imagePoints[i] = val;
			}
			maskPoints[i] = 1;
		}
	}
}

// uses some C code and unsafe function calls
#include "lib/extendsurface3d.c"

void MaterialMappingFilter::inplaceExtendImageOld(VtkImage _img, VtkImage _mask, bool _maxval)
{
	assert(_img->GetScalarType() == VTK_FLOAT && "Input image scalar type needs to be float!");

	int* dim = _img->GetDimensions();
	char* cx = (char *) malloc(dim[0] * dim[1] * dim[2] * sizeof(char));
	float* ct_stack_ext_ptr = (float *) malloc(dim[0] * dim[1] * dim[2] * sizeof(float));

	extendsurface(dim[1], dim[0], dim[2],
	              (float *) (_img->GetScalarPointer()),
	              (char *) (_mask->GetScalarPointer()), ct_stack_ext_ptr, cx);
	float* ct_stack_ptr = (float *) (_img->GetScalarPointer());
	unsigned char* c = (unsigned char *) (_mask->GetScalarPointer());

	for (int i = 0; i < _mask->GetNumberOfPoints(); i++)
	{
		if (_maxval)
		{
			if (ct_stack_ptr[i] < ct_stack_ext_ptr[i])
				ct_stack_ptr[i] = ct_stack_ext_ptr[i];
		}
		else
			ct_stack_ptr[i] = ct_stack_ext_ptr[i];
		c[i] = cx[i];
	}
	free(cx);
	free(ct_stack_ext_ptr);
}

MaterialMappingFilter::VtkDoubleArray MaterialMappingFilter::interpolateToNodes(const VtkUGrid _mesh,
                                                                                const VtkImage _img,
                                                                                std::string _name,
                                                                                double _minElem) const
{
	auto data = vtkSmartPointer<vtkDoubleArray>::New();
	data->SetNumberOfComponents(1);
	data->SetName(_name.c_str());

	auto interpolator = vtkSmartPointer<vtkImageInterpolator>::New();
	interpolator->Initialize(_img);
	interpolator->SetInterpolationModeToLinear();
	interpolator->Update();

	for (auto i = 0; i < _mesh->GetNumberOfPoints(); ++i)
	{
		auto p = _mesh->GetPoint(i);
		auto val = interpolator->Interpolate(p[0], p[1], p[2], 0);
		data->InsertTuple1(i, val > _minElem ? val : _minElem);
	}

	return data;
}

MaterialMappingFilter::VtkDoubleArray MaterialMappingFilter::nodesToElements(const VtkUGrid _mesh,
                                                                             VtkDoubleArray _nodeData,
                                                                             std::string _name) const
{
	auto data = vtkSmartPointer<vtkDoubleArray>::New();
	data->SetNumberOfComponents(1);
	data->SetName(_name.c_str());

	for (auto i = 0; i < _mesh->GetNumberOfCells(); ++i)
	{
		auto cellpoints = _mesh->GetCell(i)->GetPoints();
		auto numberOfNodes = cellpoints->GetNumberOfPoints();

		// get centroid
		double centroid[3] = {0, 0, 0};
		for (auto j = 0; j < numberOfNodes; ++j)
		{
			auto cellpoint = cellpoints->GetPoint(j);
			for (auto k = 0; k < 3; ++k)
			{
				centroid[k] = (centroid[k] * j + cellpoint[k]) / (j + 1);
			}
		}

		// calculate nodal weight = squared distance to centroid
		double minDistance = std::numeric_limits<double>::max();
		std::vector<double> squaredDistances(numberOfNodes);
		for (auto j = 0; j < numberOfNodes; ++j)
		{
			auto cellpoint = cellpoints->GetPoint(j);
			double squaredDistance = 0;
			for (auto k = 0; k < 3; ++k)
			{
				squaredDistance += pow(cellpoint[k] - centroid[k], 2);
			}
			squaredDistance = sqrt(squaredDistance);
			squaredDistances.at(j) = squaredDistance;

			// if a node aligns with the centroid, we set it's weight to the next closest one
			if (squaredDistance == 0)
				squaredDistance = 1;

			if (squaredDistance < minDistance)
				minDistance = squaredDistance;
		}

		// invert weight and normalize
		double value = 0, denom = 0;
		for (auto j = 0; j < numberOfNodes; ++j)
		{
			// normalize the weight so the node closest to the centroid has a weight of 1.0
			auto normalizedWeight = minDistance / squaredDistances.at(j);
			denom += normalizedWeight;
			value += normalizedWeight * _nodeData->GetTuple1(_mesh->GetCell(i)->GetPointId(j));
		}
		auto weightedValue = value / denom;
		data->InsertTuple1(i, weightedValue);
	}

	return data;
}

void MaterialMappingFilter::inplaceApplyFunctorsToImage(MaterialMappingFilter::VtkImage _img)
{
	for (auto i = 0; i < _img->GetNumberOfPoints(); i++)
	{
		auto valCT = _img->GetPointData()->GetScalars()->GetTuple1(i);
		auto valDensity = m_BoneDensityFunctor(valCT);
		auto valEMorgan = m_PowerLawFunctor(std::max(valDensity, 0.0));
		_img->GetPointData()->GetScalars()->SetTuple1(i, valEMorgan);
	}
}

void MaterialMappingFilter::writeMetaImageToVerboseOut(const std::string _filename, vtkSmartPointer<vtkImageData> _img)
{
	vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
	writer->SetFileName((m_VerboseOutputDirectory + "/" + _filename).c_str());
	writer->SetInputData(_img);
	writer->Write();
}
