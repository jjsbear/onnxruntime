// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/session/onnxruntime_cxx_api.h"
#include <vector>

struct Input {
  const char* name = nullptr;
  std::vector<int64_t> dims;
  std::vector<float> values;
};

struct OrtTensorDimensions : std::vector<int64_t> {
  OrtTensorDimensions(Ort::CustomOpApi ort, const OrtValue* value) {
    OrtTensorTypeAndShapeInfo* info = ort.GetTensorTypeAndShape(value);
    std::vector<int64_t>::operator=(ort.GetTensorShape(info));
    ort.ReleaseTensorTypeAndShapeInfo(info);
  }
};

struct MyCustomKernel {
  MyCustomKernel(Ort::CustomOpApi ort, const OrtKernelInfo* /*info*/) : ort_(ort) {
  }

  void Compute(OrtKernelContext* context);

 private:
  Ort::CustomOpApi ort_;
};

struct MyCustomOp : Ort::CustomOpBase<MyCustomOp, MyCustomKernel> {
  explicit MyCustomOp(const char* provider) : provider_(provider) {}

  void* CreateKernel(Ort::CustomOpApi api, const OrtKernelInfo* info) const { return new MyCustomKernel(api, info); };
  const char* GetName() const { return "Foo"; };
  const char* GetExecutionProviderType() const { return provider_; };

  size_t GetInputTypeCount() const { return 2; };
  ONNXTensorElementDataType GetInputType(size_t /*index*/) const {
    // Both the inputs need to be necessarily of float type
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
  };

  size_t GetOutputTypeCount() const { return 1; };
  ONNXTensorElementDataType GetOutputType(size_t /*index*/) const { return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT; };

 private:
  const char* provider_;
};

struct MyCustomKernelMultipleDynamicInputs {
  MyCustomKernelMultipleDynamicInputs(Ort::CustomOpApi ort, const OrtKernelInfo* /*info*/) : ort_(ort) {
  }

  void Compute(OrtKernelContext* context);

 private:
  Ort::CustomOpApi ort_;
};

struct MyCustomOpMultipleDynamicInputs : Ort::CustomOpBase<MyCustomOpMultipleDynamicInputs, MyCustomKernelMultipleDynamicInputs> {
  explicit MyCustomOpMultipleDynamicInputs(const char* provider) : provider_(provider) {}

  void* CreateKernel(Ort::CustomOpApi api, const OrtKernelInfo* info) const { return new MyCustomKernelMultipleDynamicInputs(api, info); };
  const char* GetName() const { return "Foo"; };
  const char* GetExecutionProviderType() const { return provider_; };

  size_t GetInputTypeCount() const { return 2; };
  ONNXTensorElementDataType GetInputType(size_t /*index*/) const {
    // Both the inputs are dynamic typed (i.e.) they can be any type and need not be
    // homogeneous
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;
  };

  size_t GetOutputTypeCount() const { return 1; };
  ONNXTensorElementDataType GetOutputType(size_t /*index*/) const { return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT; };

 private:
  const char* provider_;
};

struct MyCustomKernelWithOptionalInput {
  MyCustomKernelWithOptionalInput(Ort::CustomOpApi ort, const OrtKernelInfo* /*info*/) : ort_(ort) {
  }
  void Compute(OrtKernelContext* context);

 private:
  Ort::CustomOpApi ort_;
};

struct MyCustomOpWithOptionalInput : Ort::CustomOpBase<MyCustomOpWithOptionalInput, MyCustomKernelWithOptionalInput> {
  explicit MyCustomOpWithOptionalInput(const char* provider) : provider_(provider) {}

  void* CreateKernel(Ort::CustomOpApi api, const OrtKernelInfo* info) const { return new MyCustomKernelMultipleDynamicInputs(api, info); };
  const char* GetName() const { return "FooBar"; };
  const char* GetExecutionProviderType() const { return provider_; };

  size_t GetInputTypeCount() const { return 3; };
  ONNXTensorElementDataType GetInputType(size_t /*index*/) const { return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT; };
  OrtCustomOpInputOutputCharacteristic GetInputCharacteristic(size_t index) const {
    // The second input (index == 1) is optional
    if (index == 1)
      return OrtCustomOpInputOutputCharacteristic::INPUT_OUTPUT_OPTIONAL;

    return OrtCustomOpInputOutputCharacteristic::INPUT_OUTPUT_REQUIRED;
  }

  size_t GetOutputTypeCount() const { return 1; };
  ONNXTensorElementDataType GetOutputType(size_t /*index*/) const { return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT; };
  OrtCustomOpInputOutputCharacteristic GetOutputCharacteristic(size_t /*index*/) const {
    return OrtCustomOpInputOutputCharacteristic::INPUT_OUTPUT_REQUIRED;
  }

 private:
  const char* provider_;
};

struct MyCustomKernelWithAttributes {
  MyCustomKernelWithAttributes(Ort::CustomOpApi ort, const OrtKernelInfo* info) : ort_(ort) {
    int_attr_ = ort_.KernelInfoGetAttribute<int64_t>(info, "int_attr");
    float_attr_ = ort_.KernelInfoGetAttribute<float>(info, "float_attr");

    ints_attr_ = ort_.KernelInfoGetAttribute<std::vector<int64_t>>(info, "ints_attr");
    floats_attr_ = ort_.KernelInfoGetAttribute<std::vector<float>>(info, "floats_attr");

    string_arr_ = ort_.KernelInfoGetAttribute<std::string>(info, "string_attr");
  }

  void Compute(OrtKernelContext* context);

 private:
  Ort::CustomOpApi ort_;

  int64_t int_attr_;
  float float_attr_;

  std::vector<int64_t> ints_attr_;
  std::vector<float> floats_attr_;

  std::string string_arr_;
};

struct MyCustomOpWithAttributes : Ort::CustomOpBase<MyCustomOpWithAttributes, MyCustomKernelWithAttributes> {
  explicit MyCustomOpWithAttributes(const char* provider) : provider_(provider) {}

  void* CreateKernel(Ort::CustomOpApi api, const OrtKernelInfo* info) const { return new MyCustomKernelWithAttributes(api, info); };
  const char* GetName() const { return "FooBar_Attr"; };
  const char* GetExecutionProviderType() const { return provider_; };

  size_t GetInputTypeCount() const { return 1; };
  ONNXTensorElementDataType GetInputType(size_t /*index*/) const { return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT; };

  size_t GetOutputTypeCount() const { return 1; };
  ONNXTensorElementDataType GetOutputType(size_t /*index*/) const { return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT; };

 private:
  const char* provider_;
};
