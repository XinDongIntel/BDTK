/*
 * Copyright (c) 2022 Intel Corporation.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <gtest/gtest.h>
#include "ArrowArrayBuilder.h"
#include "exec/plan/parser/TypeUtils.h"

TEST(ArrowArrayBuilderTest, EmptyBatch) {
  // should not build an empty batch.
  EXPECT_THROW({ auto emptyBatch = ArrowArrayBuilder().build(); }, CiderCompileException);

  ArrowSchema* schema = nullptr;
  ArrowArray* array = nullptr;

  std::tie(schema, array) =
      ArrowArrayBuilder().addColumn<int>("col", CREATE_SUBSTRAIT_TYPE(I32), {}).build();

  EXPECT_EQ(0, array->length);
  EXPECT_EQ(1, array->n_children);
}

TEST(ArrowArrayBuilderTest, RowNum) {
  // test add empty row num
  ArrowArray* array = nullptr;
  ArrowSchema* schema = nullptr;

  std::tie(schema, array) = ArrowArrayBuilder()
                                .setRowNum(10)
                                .addColumn<int>("col", CREATE_SUBSTRAIT_TYPE(I32), {})
                                .build();

  EXPECT_EQ(10, array->length);

  std::tie(schema, array) =
      ArrowArrayBuilder()
          .addColumn<int>("col1", CREATE_SUBSTRAIT_TYPE(I32), {})
          .addColumn<int>("col2", CREATE_SUBSTRAIT_TYPE(I32), {1, 2, 3, 4, 5})
          .build();

  EXPECT_EQ(5, array->length);

  // test set row num multiple times.
  EXPECT_THROW({ auto batch3 = ArrowArrayBuilder().setRowNum(10).setRowNum(20).build(); },
               CiderCompileException);

  // test set row num after add Column
  EXPECT_THROW(
      {
        auto batch4 =
            ArrowArrayBuilder()
                .addColumn<int>("col1", CREATE_SUBSTRAIT_TYPE(I32), {1, 2, 3, 4, 5})
                .setRowNum(20)
                .build();
      },
      CiderCompileException);

  // test row num not equal
  EXPECT_THROW(
      {
        auto batch5 =
            ArrowArrayBuilder()
                .addColumn<int>("col1", CREATE_SUBSTRAIT_TYPE(I32), {1, 2, 3, 4, 5})
                .addColumn<int>("col2", CREATE_SUBSTRAIT_TYPE(I32), {1, 2, 3, 4, 5, 6})
                .build();
      },
      CiderCompileException);

  EXPECT_THROW(
      {
        auto batch6 =
            ArrowArrayBuilder()
                .addColumn<int>("col1", CREATE_SUBSTRAIT_TYPE(I32), {1, 2, 3, 4, 5})
                .addColumn<int>("col2", CREATE_SUBSTRAIT_TYPE(I32), {1, 2, 3, 4})
                .build();
      },
      CiderCompileException);
}

TEST(ArrowArrayBuilderTest, OneColumnBatch) {
  ArrowArray* array = nullptr;
  ArrowSchema* schema = nullptr;

  std::vector<int> vec1{1, 2, 3, 4, 5};
  std::tie(schema, array) = ArrowArrayBuilder()
                                .setRowNum(5)
                                .addColumn<int>("int", CREATE_SUBSTRAIT_TYPE(I32), vec1)
                                .build();

  EXPECT_EQ(5, array->length);
  EXPECT_EQ(1, array->n_children);
}

// TEST(ArrowArrayBuilderTest, CiderByteArrayBatch) {
//   std::vector<CiderByteArray> vec;
//   vec.push_back(CiderByteArray(5, reinterpret_cast<const uint8_t*>("aaaaa")));
//   vec.push_back(CiderByteArray(5, reinterpret_cast<const uint8_t*>("bbbbb")));
//   vec.push_back(CiderByteArray(10, reinterpret_cast<const uint8_t*>("aaaaabbbbb")));

//   auto batch =
//       ArrowArrayBuilder()
//           .setRowNum(3)
//           .addColumn<CiderByteArray>("col_str", CREATE_SUBSTRAIT_TYPE(Varchar), vec)
//           .build();
//   EXPECT_EQ(3, batch.row_num());
//   auto ptr = reinterpret_cast<const CiderByteArray*>(batch.column(0));
//   CHECK_EQ(5, ptr[0].len);
//   CHECK_EQ(5, ptr[1].len);
//   CHECK_EQ(10, ptr[2].len);

//   CHECK_EQ(0, std::memcmp("aaaaa", ptr[0].ptr, ptr[0].len));
//   CHECK_EQ(0, std::memcmp("bbbbb", ptr[1].ptr, ptr[1].len));
//   CHECK_EQ(0, std::memcmp("aaaaabbbbb", ptr[2].ptr, ptr[2].len));
//   CHECK_NE(0, std::memcmp("bbbbbaaaaa", ptr[2].ptr, ptr[2].len));
// }

// TEST(ArrowArrayBuilderTest, StringBatch) {
//   auto batch1 = ArrowArrayBuilder()
//                     .setRowNum(5)
//                     .addColumn<std::string>("str", CREATE_SUBSTRAIT_TYPE(String), {})
//                     .build();
//   EXPECT_EQ(5, batch1.row_num());
//   EXPECT_EQ(1, batch1.column_num());

//   auto batch2 = ArrowArrayBuilder()
//                     .setRowNum(5)
//                     .addColumn<std::string>("VarChar", CREATE_SUBSTRAIT_TYPE(Varchar),
//                     {}) .build();
//   EXPECT_EQ(5, batch2.row_num());
//   EXPECT_EQ(1, batch2.column_num());

//   // test even we pass some input data, the data buffer should be nullptr.
//   auto batch3 =
//       ArrowArrayBuilder()
//           .setRowNum(5)
//           .addColumn<std::string>(
//               "VarChar", CREATE_SUBSTRAIT_TYPE(Varchar), {"a", "b", "c", "d", "e"})
//           .build();
//   EXPECT_EQ(5, batch3.row_num());
//   EXPECT_EQ(1, batch3.column_num());
// }

TEST(ArrowArrayBuilderTest, MultiColumnsBatch) {
  std::vector<int> vec1{1, 2, 3, 4, 5};
  std::vector<int> vec2{6, 7, 8, 9, 10};

  ArrowArray* array = nullptr;
  ArrowSchema* schema = nullptr;

  std::tie(schema, array) = ArrowArrayBuilder()
                                .setRowNum(5)
                                .addColumn<int>("col1", CREATE_SUBSTRAIT_TYPE(I32), vec1)
                                .addColumn<int>("col2", CREATE_SUBSTRAIT_TYPE(I32), vec2)
                                .build();

  EXPECT_EQ("col1", std::string(schema->children[0]->name));
  EXPECT_EQ("col2", std::string(schema->children[1]->name));
}

TEST(ArrowArrayBuilderTest, MultiTypesBatch) {
  std::vector<int> vec1{1, 2, 3, 4, 5};
  std::vector<int64_t> vec2{6, 7, 8, 9, 10};
  std::vector<float> vec3{1.1, 2.2, 3.3, 4.4, 5.5};
  std::vector<double> vec4{1.1, 2.2, 3.3, 4.4, 5.5};

  ArrowArray* array = nullptr;
  ArrowSchema* schema = nullptr;

  std::tie(schema, array) =
      ArrowArrayBuilder()
          .setRowNum(5)
          .addColumn<int>("col1", CREATE_SUBSTRAIT_TYPE(I32), vec1)
          .addColumn<int64_t>("col2", CREATE_SUBSTRAIT_TYPE(I64), vec2)
          .addColumn<float>("col3", CREATE_SUBSTRAIT_TYPE(Fp32), vec3)
          .addColumn<double>("col4", CREATE_SUBSTRAIT_TYPE(Fp64), vec4)
          .build();

  EXPECT_EQ("i", std::string(schema->children[0]->format));
  EXPECT_EQ("l", std::string(schema->children[1]->format));
  EXPECT_EQ("f", std::string(schema->children[2]->format));
  EXPECT_EQ("g", std::string(schema->children[3]->format));
}

// TEST(ArrowArrayBuilderTest, ToValueStringTest) {
//   std::vector<int> vec1{1, 2, 3, 4, 5};
//   std::vector<int64_t> vec2{1, 2, 3, 4, 5};
//   std::vector<float> vec3{1.1, 2.2, 3.3, 4.4, 5.5};
//   std::vector<double> vec4{1.1, 2.2, 3.3, 4.4, 5.5};

//   auto batch = ArrowArrayBuilder()
//                    .setRowNum(5)
//                    .addColumn<int>("col1", CREATE_SUBSTRAIT_TYPE(I32), vec1)
//                    .addColumn<int64_t>("col2", CREATE_SUBSTRAIT_TYPE(I64), vec2)
//                    .addColumn<float>("col3", CREATE_SUBSTRAIT_TYPE(Fp32), vec3)
//                    .addColumn<double>("col4", CREATE_SUBSTRAIT_TYPE(Fp64), vec4)
//                    .build();

//   std::string res =
//       "row num: 5, column num: 4.\n"
//       "column type: int32_t 1\t2\t3\t4\t5\t\n"
//       "column type: int64_t 1\t2\t3\t4\t5\t\n"
//       "column type: float 1.1\t2.2\t3.3\t4.4\t5.5\t\n"
//       "column type: double 1.1\t2.2\t3.3\t4.4\t5.5\t\n";
//   EXPECT_EQ(res, batch.toValueString());
// }

// TEST(ArrowArrayBuilderTest, DateTypebatch) {
//   std::vector<CiderDateType> vec1;
//   vec1.push_back(CiderDateType("1970-01-01"));
//   vec1.push_back(CiderDateType("1970-01-02"));
//   vec1.push_back(CiderDateType("2020-01-01"));
//   vec1.push_back(CiderDateType("2030-01-01"));
//   vec1.push_back(CiderDateType("1970-01-01"));

//   auto batch1 =
//       ArrowArrayBuilder()
//           .setRowNum(5)
//           .addTimingColumn<CiderDateType>("col1", CREATE_SUBSTRAIT_TYPE(Date), vec1)
//           .build();

//   EXPECT_EQ(5, batch1.row_num());
//   EXPECT_EQ(1, batch1.column_num());

//   EXPECT_THROW(
//       {
//         std::vector<CiderDateType> vec2;
//         vec2.push_back(CiderDateType("1970-13-01"));
//         auto batch2 =
//             ArrowArrayBuilder()
//                 .addTimingColumn<CiderDateType>("col1", CREATE_SUBSTRAIT_TYPE(Date),
//                 vec2) .build();
//       },
//       CiderCompileException);
// }

// ArrowArrayBuilder will not use null_vector in addColumn context. You should generate
// null value(MIN_VALUE) for target type in value vector.
TEST(ArrowArrayBuilderTest, nullTest) {
  std::vector<int> vec1{1, 2, 3, 4, 5};
  std::vector<int64_t> vec2{1, 2, 3, 4, 5};
  std::vector<float> vec3{1.1, 2.2, 3.3, 4.4, 5.5};
  std::vector<double> vec4{1.1, 2.2, 3.3, 4.4, 5.5};
  std::vector<bool> vec_null{true, false, true, false, true};

  ArrowArray* array = nullptr;
  ArrowSchema* schema = nullptr;
  std::tie(schema, array) =
      ArrowArrayBuilder()
          .setRowNum(5)
          .addColumn<int>("col1", CREATE_SUBSTRAIT_TYPE(I32), vec1, vec_null)
          .addColumn<int64_t>("col2", CREATE_SUBSTRAIT_TYPE(I64), vec2, vec_null)
          .addColumn<float>("col3", CREATE_SUBSTRAIT_TYPE(Fp32), vec3, vec_null)
          .addColumn<double>("col4", CREATE_SUBSTRAIT_TYPE(Fp64), vec4, vec_null)
          .build();

  EXPECT_EQ(0xF5, *(uint8_t*)(array->children[0]->buffers[0]));
}

TEST(ArrowArrayBuilderTest, CiderBatchConstructorTest) {
  std::vector<int> vec1{1, 2, 3, 4, 5};
  std::vector<int64_t> vec2{1, 2, 3, 4, 5};
  std::vector<float> vec3{1.1, 2.2, 3.3, 4.4, 5.5};
  std::vector<double> vec4{1.1, 2.2, 3.3, 4.4, 5.5};
  std::vector<bool> vec_null{true, false, true, false, true};

  ArrowArray* array = nullptr;
  ArrowSchema* schema = nullptr;
  std::tie(schema, array) =
      ArrowArrayBuilder()
          .setRowNum(5)
          .addColumn<int>("col1", CREATE_SUBSTRAIT_TYPE(I32), vec1, vec_null)
          .addColumn<int64_t>("col2", CREATE_SUBSTRAIT_TYPE(I64), vec2, vec_null)
          .addColumn<float>("col3", CREATE_SUBSTRAIT_TYPE(Fp32), vec3, vec_null)
          .addColumn<double>("col4", CREATE_SUBSTRAIT_TYPE(Fp64), vec4, vec_null)
          .build();

  CiderBatch* batch =
      new CiderBatch(schema, array, std::make_shared<CiderDefaultAllocator>());
  EXPECT_EQ(0, batch->getBufferNum());
  EXPECT_EQ(4, batch->getChildrenNum());

  EXPECT_EQ(SQLTypes::kSTRUCT, batch->getCiderType());
  EXPECT_EQ(SQLTypes::kINT, batch->getChildAt(0)->getCiderType());

  EXPECT_EQ(true, batch->isRootOwner());
  EXPECT_EQ(0xF5, *(uint8_t*)(batch->getChildAt(0)->getNulls()));
  EXPECT_EQ(5, batch->getLength());
  EXPECT_EQ(false, batch->isMoved());
  batch->move();
  EXPECT_EQ(true, batch->isMoved());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

  int err{0};
  try {
    err = RUN_ALL_TESTS();
  } catch (const std::exception& e) {
  }
  return err;
}