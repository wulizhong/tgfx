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

#pragma once

#include "gpu/processors/FragmentProcessor.h"

namespace tgfx {
class DeviceSpaceTextureEffect : public FragmentProcessor {
 public:
  static std::unique_ptr<DeviceSpaceTextureEffect> Make(std::shared_ptr<TextureProxy> textureProxy,
                                                        ImageOrigin deviceOrigin);

  std::string name() const override {
    return "DeviceSpaceTextureEffect";
  }

 protected:
  DEFINE_PROCESSOR_CLASS_ID

  DeviceSpaceTextureEffect(std::shared_ptr<TextureProxy> textureProxy, ImageOrigin deviceOrigin);

  bool onIsEqual(const FragmentProcessor& processor) const override;

  size_t onCountTextureSamplers() const override {
    return 1;
  }

  const TextureSampler* onTextureSampler(size_t) const override;

  std::shared_ptr<TextureProxy> textureProxy;
  ImageOrigin deviceOrigin = ImageOrigin::TopLeft;
};
}  // namespace tgfx
