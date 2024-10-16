/*============================================================================
Copyright (c) Eric Heim
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <gtest/gtest.h>

#include "TestCudaMinMaxScalerFixture.h"

#include <CudaMinMaxScaler.h>

namespace mitk
{
namespace cuda_example
{
namespace test
{

TEST_F(TestCudaMinMaxScalerFixture, ScaleImage) {
  // Arrange
  CudaMinMaxScaler::Pointer scaler = GetMinMaxScaler();
  auto start = std::chrono::high_resolution_clock::now();
  scaler->SetInput(this->m_InputImage);
  // Act
  scaler->Update();
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  // ��ӡʱ���
  std::cout << "CUDA Time taken by function: " << duration.count() << " milliseconds" << std::endl;
  auto result = scaler->GetOutput();
  // Assert
  EXPECT_TRUE(Equal(*result, *m_Expected, 1.0e-5, true));
}

}
}
}
