// Copyright (c) 2021 - present Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef TREE_NODE_BLUE_H
#define TREE_NODE_BLUE_H

#include "tree_node.h"

/*****************************************************
 * CS_BLUESTEIN
 *****************************************************/
class BluesteinNode : public InternalNode
{
    friend class NodeFactory;

protected:
    explicit BluesteinNode(TreeNode* p)
        : InternalNode(p)
    {
        scheme = CS_BLUESTEIN;
    }
#if !GENERIC_BUF_ASSIGMENT
    void AssignBuffers_internal(TraverseState&   state,
                                OperatingBuffer& flipIn,
                                OperatingBuffer& flipOut,
                                OperatingBuffer& obOutBuf) override;
#endif
    void AssignParams_internal() override;
    void BuildTree_internal() override;
};

/*****************************************************
 * Component of Bluestein
 * Chirp, XXXMul
 *****************************************************/
class BluesteinComponentNode : public LeafNode
{
    friend class NodeFactory;

protected:
    BluesteinComponentNode(TreeNode* p, ComputeScheme s)
        : LeafNode(p, s)
    {
        // first PAD MUL: in=parent_in, out=bluestein, must be out-of-place
        // last  RES MUL: in=bluestein, out=parent_out, must be out-of-place
        // other components, must be blue -> blue
        if(scheme == CS_KERNEL_PAD_MUL || scheme == CS_KERNEL_RES_MUL)
            allowInplace = false;
        else
            allowOutofplace = false;

        // RES_MUL must not output to B buffer, while others must output to B buffer
        if(scheme == CS_KERNEL_RES_MUL)
            allowedOutBuf = OB_USER_IN | OB_USER_OUT | OB_TEMP | OB_TEMP_CMPLX_FOR_REAL;
        else
        {
            allowedOutBuf        = OB_TEMP_BLUESTEIN;
            allowedOutArrayTypes = {rocfft_array_type_complex_interleaved};
        }
    }

    void SetupGPAndFnPtr_internal(DevFnCall& fnPtr, GridParam& gp) override;
};

#endif // TREE_NODE_BLUE_H
