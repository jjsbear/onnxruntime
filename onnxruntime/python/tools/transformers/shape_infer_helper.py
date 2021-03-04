import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from symbolic_shape_infer import *


class SymbolicShapeInferenceHelper(SymbolicShapeInference):
    def __init__(self,
                 model,
                 dynamic_axis_mapping={
                     "batch_size": 4,
                     "seq_len": 16
                 },
                 verbose=0,
                 int_max=2**31 - 1,
                 auto_merge=False,
                 guess_output_rank=False):
        super().__init__(int_max, auto_merge, guess_output_rank, verbose)

        self.dynamic_axis_mapping_ = dynamic_axis_mapping  # e.g {"batch_size" : 4, "seq_len" : 16}

        all_shapes_inferred = False
        self._preprocess(model)
        while self.run_:
            all_shapes_inferred = self._infer_impl()

        if not all_shapes_inferred:
            raise Exception("Incomplete symbolic shape inference")

    # Override a function in symbolic_shape_infer.py to ensure shape inference by giving the actual value of dynamic axis
    def _get_sympy_shape(self, node, idx):
        sympy_shape = []
        for d in self._get_shape(node, idx):
            if type(d) == str:
                if d in self.dynamic_axis_mapping_.keys():
                    sympy_shape.append(self.dynamic_axis_mapping_[d])
                elif d in self.symbolic_dims_:
                    sympy_shape.append(self.symbolic_dims_[d])
                else:
                    sympy_shape.append(sympy.Symbol(d, integer=True))
            else:
                assert None != d
                sympy_shape.append(d)
        return sympy_shape

    def get_edge_shape(self, edge):
        if edge not in self.known_vi_:
            raise Exception("Cannot retrive edge shape")
        type_proto = self.known_vi_[edge].type
        shape = get_shape_from_type_proto(type_proto)
        for i in range(len(shape)):
            d = shape[i]
            if type(d) == str:
                shape[i] = self.dynamic_axis_mapping_[d]
        return shape

    def compare_shape(self, edge, edge_other):
        return self.get_edge_shape(edge) == self.get_edge_shape(edge_other)

