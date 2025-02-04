// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/dcmtkUtils.h"
#include <absl/strings/string_view.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include "tests/testUtils.h"

TEST(insertBaseImageTagsTest, correctInsert) {
  std::unique_ptr<DcmDataset> dataSet = std::make_unique<DcmDataset>();

  wsiToDicomConverter::DcmtkUtils::insertBaseImageTags("image", 10, 20, 0.0,
                                                       0.0, dataSet.get());

  char* stringValue;
  findElement(dataSet.get(), DCM_SeriesDescription)->getString(stringValue);
  EXPECT_EQ("image", absl::string_view(stringValue));

  Uint32 rows;
  findElement(dataSet.get(), DCM_TotalPixelMatrixRows)->getUint32(rows);
  EXPECT_EQ(10, rows);

  Uint32 columns;
  findElement(dataSet.get(), DCM_TotalPixelMatrixColumns)->getUint32(columns);
  EXPECT_EQ(20, columns);

  EXPECT_EQ(nullptr, findElement(dataSet.get(), DCM_ImagedVolumeWidth));
  EXPECT_EQ(nullptr, findElement(dataSet.get(), DCM_ImagedVolumeHeight));

  wsiToDicomConverter::DcmtkUtils::insertBaseImageTags("image", 10, 20, 10.0,
                                                       20.0, dataSet.get());

  Float32 width;
  findElement(dataSet.get(), DCM_ImagedVolumeWidth)->getFloat32(width);
  EXPECT_EQ(10, width);

  Float32 height;
  findElement(dataSet.get(), DCM_ImagedVolumeHeight)->getFloat32(height);
  ASSERT_EQ(20, height);
}

TEST(insertStaticTagsTest, correctInsert_Level0) {
  std::unique_ptr<DcmDataset> dataSet = std::make_unique<DcmDataset>();

  wsiToDicomConverter::DcmtkUtils::insertStaticTags(dataSet.get(), 0);

  char* stringValue;
  findElement(dataSet.get(), DCM_SOPClassUID)->getString(stringValue);
  EXPECT_EQ(UID_VLWholeSlideMicroscopyImageStorage,
            absl::string_view(stringValue));

  findElement(dataSet.get(), DCM_Modality)->getString(stringValue);
  EXPECT_EQ("SM", absl::string_view(stringValue));

  findElement(dataSet.get(), DCM_ImageType)->getString(stringValue);
  EXPECT_EQ("ORIGINAL\\PRIMARY\\VOLUME\\NONE", absl::string_view(stringValue));

  findElement(dataSet.get(), DCM_ImageOrientationSlide)->getString(stringValue);
  EXPECT_EQ("0\\-1\\0\\-1\\0\\0", absl::string_view(stringValue));

  Uint16 frameNumber;
  findElement(dataSet.get(), DCM_RepresentativeFrameNumber)
      ->getUint16(frameNumber);
  ASSERT_EQ(1, frameNumber);
}

TEST(insertStaticTagsTest, correctInsert_Level1) {
  std::unique_ptr<DcmDataset> dataSet = std::make_unique<DcmDataset>();

  wsiToDicomConverter::DcmtkUtils::insertStaticTags(dataSet.get(), 1);

  char* stringValue;
  findElement(dataSet.get(), DCM_SOPClassUID)->getString(stringValue);
  EXPECT_EQ(UID_VLWholeSlideMicroscopyImageStorage,
            absl::string_view(stringValue));

  findElement(dataSet.get(), DCM_Modality)->getString(stringValue);
  EXPECT_EQ("SM", absl::string_view(stringValue));

  findElement(dataSet.get(), DCM_ImageType)->getString(stringValue);
  EXPECT_EQ("DERIVED\\PRIMARY\\VOLUME\\RESAMPLED",
            absl::string_view(stringValue));

  findElement(dataSet.get(), DCM_ImageOrientationSlide)->getString(stringValue);
  EXPECT_EQ("0\\-1\\0\\-1\\0\\0", absl::string_view(stringValue));

  Uint16 frameNumber;
  findElement(dataSet.get(), DCM_RepresentativeFrameNumber)
      ->getUint16(frameNumber);
  ASSERT_EQ(1, frameNumber);
}

TEST(insertIdsTest, correctInsert) {
  std::unique_ptr<DcmDataset> dataSet = std::make_unique<DcmDataset>();

  wsiToDicomConverter::DcmtkUtils::insertIds("study", "series", dataSet.get());

  EXPECT_NE(nullptr, findElement(dataSet.get(), DCM_SOPInstanceUID));

  char* stringValue;
  findElement(dataSet.get(), DCM_StudyInstanceUID)->getString(stringValue);
  EXPECT_EQ("study", absl::string_view(stringValue));

  findElement(dataSet.get(), DCM_SeriesInstanceUID)->getString(stringValue);
  ASSERT_EQ("series", absl::string_view(stringValue));
}

TEST(insertMultiFrameTagsTest, correctInsert) {
  std::unique_ptr<DcmDataset> dataSet = std::make_unique<DcmDataset>();
  DcmtkImgDataInfo imgInfo;
  imgInfo.rows = 10;
  imgInfo.cols = 10;
  wsiToDicomConverter::DcmtkUtils::insertMultiFrameTags(
      imgInfo, 5, 10, 0, 0, 0, 0, 0, 10, true, "series", dataSet.get());

  char* stringValue;
  findElement(dataSet.get(), DCM_InstanceNumber)->getString(stringValue);
  EXPECT_EQ("1", absl::string_view(stringValue));

  findElement(dataSet.get(), DCM_FrameOfReferenceUID)->getString(stringValue);
  EXPECT_EQ("series.1", absl::string_view(stringValue));

  findElement(dataSet.get(), DCM_DimensionOrganizationType)
      ->getString(stringValue);
  EXPECT_EQ("TILED_FULL", absl::string_view(stringValue));

  Uint32 offset;
  findElement(dataSet.get(), DCM_ConcatenationFrameOffsetNumber)
      ->getUint32(offset);
  EXPECT_EQ(0, offset);

  Uint16 batchNumber;
  findElement(dataSet.get(), DCM_InConcatenationNumber)->getUint16(batchNumber);
  EXPECT_EQ(1, batchNumber);

  Uint16 concatenationTotalNumber;
  findElement(dataSet.get(), DCM_InConcatenationTotalNumber)
      ->getUint16(concatenationTotalNumber);
  EXPECT_EQ(2, concatenationTotalNumber);

  wsiToDicomConverter::DcmtkUtils::insertMultiFrameTags(
      imgInfo, 4, 5, 1, 1, 0, 0, 0, 105, false, "series", dataSet.get());
  findElement(dataSet.get(), DCM_InConcatenationTotalNumber)
      ->getUint16(concatenationTotalNumber);
  EXPECT_EQ(27, concatenationTotalNumber);

  findElement(dataSet.get(), DCM_DimensionOrganizationType)
      ->getString(stringValue);
  EXPECT_EQ("TILED_SPARSE", absl::string_view(stringValue));

  DcmSequenceOfItems* element = reinterpret_cast<DcmSequenceOfItems*>(
      findElement(dataSet.get(), DCM_PerFrameFunctionalGroupsSequence));

  EXPECT_NE(nullptr, element);
  Sint32 column;
  // 3rd item of 1st batch should be in 4th column
  findElement(
      reinterpret_cast<DcmSequenceOfItems*>(
          findElement(element->getItem(3), DCM_PlanePositionSlideSequence))
          ->getItem(0),
      DCM_ColumnPositionInTotalImagePixelMatrix)
      ->getSint32(column);
  EXPECT_EQ(31, column);

  Sint32 row;
  // 2nd item of 1st batch should be in 1st row
  findElement(
      reinterpret_cast<DcmSequenceOfItems*>(
          findElement(element->getItem(1), DCM_PlanePositionSlideSequence))
          ->getItem(0),
      DCM_RowPositionInTotalImagePixelMatrix)
      ->getSint32(row);
  EXPECT_EQ(1, row);

  dataSet = std::make_unique<DcmDataset>();

  wsiToDicomConverter::DcmtkUtils::insertMultiFrameTags(
      imgInfo, 4, 5, 1, 5, 1, 1, 4, 105, false, "series", dataSet.get());
  element = reinterpret_cast<DcmSequenceOfItems*>(
      findElement(dataSet.get(), DCM_PerFrameFunctionalGroupsSequence));

  // 1nd item of 2nd batch should be in 5th column
  findElement(
      reinterpret_cast<DcmSequenceOfItems*>(
          findElement(element->getItem(0), DCM_PlanePositionSlideSequence))
          ->getItem(0),
      DCM_ColumnPositionInTotalImagePixelMatrix)
      ->getSint32(column);
  EXPECT_EQ(41, column);

  // 2nd item of 2nd batch should be in 1st column
  findElement(
      reinterpret_cast<DcmSequenceOfItems*>(
          findElement(element->getItem(1), DCM_PlanePositionSlideSequence))
          ->getItem(0),
      DCM_ColumnPositionInTotalImagePixelMatrix)
      ->getSint32(column);
  EXPECT_EQ(1, column);

  // 1st item of 2nd batch should be in 1st row
  findElement(
      reinterpret_cast<DcmSequenceOfItems*>(
          findElement(element->getItem(0), DCM_PlanePositionSlideSequence))
          ->getItem(0),
      DCM_RowPositionInTotalImagePixelMatrix)
      ->getSint32(row);
  EXPECT_EQ(1, row);

  // 2nd item of 2nd batch should be in 2nd row
  findElement(
      reinterpret_cast<DcmSequenceOfItems*>(
          findElement(element->getItem(1), DCM_PlanePositionSlideSequence))
          ->getItem(0),
      DCM_RowPositionInTotalImagePixelMatrix)
      ->getSint32(row);
  ASSERT_EQ(11, row);
}
