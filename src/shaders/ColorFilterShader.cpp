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

#include "ColorFilterShader.h"
#include "gpu/processors/FragmentProcessor.h"

namespace tgfx {
std::shared_ptr<Shader> Shader::makeWithColorFilter(
    std::shared_ptr<ColorFilter> colorFilter) const {
  auto strongThis = weakThis.lock();
  if (strongThis == nullptr) {
    return nullptr;
  }
  if (colorFilter == nullptr) {
    return strongThis;
  }
  auto shader = std::make_shared<ColorFilterShader>(std::move(strongThis), std::move(colorFilter));
  shader->weakThis = shader;
  return shader;
}

std::unique_ptr<FragmentProcessor> ColorFilterShader::asFragmentProcessor(
    const FPArgs& args) const {
  auto fp1 = shader->asFragmentProcessor(args);
  if (fp1 == nullptr) {
    return nullptr;
  }
  auto fp2 = colorFilter->asFragmentProcessor();
  if (fp2 == nullptr) {
    return fp1;
  }
  return FragmentProcessor::Compose(std::move(fp1), std::move(fp2));
}
}  // namespace tgfx
