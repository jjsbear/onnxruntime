// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <unordered_map>
#include "tensor_allocator.h"
#include "mem_pattern.h"
#include "ort_value_pattern_planner.h"
#include "utils.h"

namespace onnxruntime {
class ExecutionProviders;

class SimpleTensorAllocator : public ITensorAllocator {
 private:
  MemoryPatternGroup mem_patterns_;
  const ExecutionPlanBase& seq_plan_;

 public:
  SimpleTensorAllocator(const ExecutionPlanBase& execution_plan, const SessionState& session_state,
                        std::vector<BufferUniquePtr>& /*weights_buffers*/)
      : ITensorAllocator(session_state),
        seq_plan_(execution_plan) {}

  common::Status FinalizePlan(std::unordered_map<std::string, size_t>& planned_memory_sizes_in_byte) override {
    // There is no memory plan to allocate a big block of memory, so
    // planned memory sizes in different locations are all empty.
    planned_memory_sizes_in_byte = std::unordered_map<std::string, size_t>();
    return Status::OK();
  }
  common::Status GetPreallocatedBuffer(int ort_value_index, const char* name, std::unique_ptr<MemBuffer>& out, AllocatorPtr& alloc_out) override;
  common::Status Trace(int id, const ONNX_NAMESPACE::TensorProto* value) override;
  const MemoryPatternGroup& GetMemPatterns() override {
    return mem_patterns_;
  }
};
}  // namespace onnxruntime
