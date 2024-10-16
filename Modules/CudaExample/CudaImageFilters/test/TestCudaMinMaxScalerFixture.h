/*============================================================================
Copyright (c) Eric Heim
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef TEST_CUDA_MIN_MAX_SCALER_H
#define TEST_CUDA_MIN_MAX_SCALER_H

#include <gtest/gtest.h>

#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkTestingConfig.h>

#include "CudaMinMaxScalerStub.h"
#include "CudaMinMaxScaler.h"
#include "MinMaxScaler.h"
#include <chrono>
namespace mitk
{
namespace cuda_example
{
namespace test
{

class TestCudaMinMaxScalerFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
      const std::string img_path = MITK_DATA_DIR"/Pic3D.nrrd";

        this->m_InputImage = IOUtil::Load<Image>(img_path);
        auto start = std::chrono::high_resolution_clock::now();
        MinMaxScaler::Pointer scaler = MinMaxScaler::New();
        scaler->SetInput(this->m_InputImage);
        scaler->Update();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        // ��ӡʱ���
        std::cout << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;
        this->m_Expected = scaler->GetOutput();
    }

    CudaMinMaxScaler::Pointer GetMinMaxScaler()
    {
        return CudaMinMaxScaler::New(m_Stub);
    }

    CudaMinMaxScalerStub m_Stub;
    Image::Pointer m_InputImage {nullptr};
    Image::Pointer m_Expected {nullptr};
};

}
}
}

#endif
