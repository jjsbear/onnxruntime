// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/framework/data_types.h"
#include "core/graph/model.h"
#include "core/framework/tensorprotoutils.h"
#include "test/test_environment.h"
#include "test_utils.h"
#include "test/util/include/asserts.h"

#include "gtest/gtest.h"

using namespace ONNX_NAMESPACE;
using namespace onnxruntime;

namespace onnxruntime {
namespace test {

void LoadSaveAndCompareModel(const std::string& input_onnx, const std::string& output_onnx, const std::string& external_init_file) {
  std::shared_ptr<Model> model;
  ASSERT_STATUS_OK(Model::Load(input_onnx.c_str(), model, nullptr, DefaultLoggingManager().DefaultLogger()));
  std::remove(output_onnx.c_str());
  std::remove(external_init_file.c_str());
  ASSERT_STATUS_OK(Model::SaveWithExternalInitializers(*model, ToPathString(output_onnx), external_init_file));

  std::shared_ptr<Model> model_from_external;
  ASSERT_STATUS_OK(Model::Load(output_onnx, model_from_external, nullptr, DefaultLoggingManager().DefaultLogger()));

  Graph& graph = model->MainGraph();
  // Perform shape inference on the graph, if this succeeds then it means that we could correctly read the 
  // integer initializers used by reshape and transpose.
  ASSERT_STATUS_OK(graph.Resolve());
  Graph& graph_from_external = model_from_external->MainGraph();

  InitializedTensorSet initializers = graph.GetAllInitializedTensors();
  InitializedTensorSet initializers_from_external = graph_from_external.GetAllInitializedTensors();

  ASSERT_EQ(initializers.size(), initializers_from_external.size());

  // Compare the initializers of the two versions. 
  for (auto i : initializers) {
    const std::string kInitName = i.first;
    const ONNX_NAMESPACE::TensorProto* tensor_proto = i.second;
    const ONNX_NAMESPACE::TensorProto* from_external_tensor_proto = initializers_from_external[kInitName];

    size_t tensor_proto_size = 0;
    std::unique_ptr<uint8_t[]> tensor_proto_data;
    ORT_THROW_IF_ERROR(utils::UnpackInitializerData(*tensor_proto, Path(), tensor_proto_data, tensor_proto_size));

    size_t from_external_tensor_proto_size = 0;
    std::unique_ptr<uint8_t[]> from_external_tensor_proto_data;
    ORT_THROW_IF_ERROR(utils::UnpackInitializerData(*from_external_tensor_proto, Path(), from_external_tensor_proto_data, from_external_tensor_proto_size));

    ASSERT_EQ(tensor_proto_size, from_external_tensor_proto_size);
    EXPECT_EQ(memcmp(tensor_proto_data.get(), from_external_tensor_proto_data.get(), tensor_proto_size), 0);
  }
  // Cleanup.
  ASSERT_EQ(std::remove(output_onnx.c_str()), 0);
  ASSERT_EQ(std::remove(external_init_file.c_str()), 0);
}

TEST(SaveWithExternalInitializers, Mnist) {
  LoadSaveAndCompareModel("testdata/mnist.onnx", "testdata/mnist_with_external_initializers.onnx", "mnist_external_initializers.bin");
}

}  // namespace test
}  // namespace onnxruntime
