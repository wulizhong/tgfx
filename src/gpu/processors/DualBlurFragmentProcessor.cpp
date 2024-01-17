/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making tgfx available.
//
//  Copyright (C) 2023 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
//  in compliance with the License. You may obtain a copy of the License at
//
//      https://opensource.org/licenses/BSD-3-Clause
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "gpu/processors/DualBlurFragmentProcessor.h"

namespace tgfx {
DualBlurFragmentProcessor::DualBlurFragmentProcessor(DualBlurPassMode passMode,
                                                     std::unique_ptr<FragmentProcessor> processor,
                                                     Point blurOffset, Size texelSize)
    : FragmentProcessor(ClassID()), passMode(passMode), blurOffset(blurOffset),
      texelSize(texelSize) {
  registerChildProcessor(std::move(processor));
}

void DualBlurFragmentProcessor::onComputeProcessorKey(BytesKey* bytesKey) const {
  bytesKey->write(static_cast<uint32_t>(passMode));
}

bool DualBlurFragmentProcessor::onIsEqual(const FragmentProcessor& processor) const {
  const auto& that = static_cast<const DualBlurFragmentProcessor&>(processor);
  return passMode == that.passMode && blurOffset == that.blurOffset && texelSize == that.texelSize;
}
}  // namespace tgfx
