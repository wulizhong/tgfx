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

#include "AsyncSource.h"
#include "BufferSource.h"

namespace tgfx {
AsyncSource::AsyncSource(ResourceKey resourceKey, std::shared_ptr<ImageGenerator> imageGenerator,
                         bool mipMapped)
    : EncodedSource(std::move(resourceKey), imageGenerator, mipMapped) {
  imageDecoder = ImageDecoder::MakeFrom(std::move(imageGenerator), !mipMapped);
}

std::shared_ptr<ImageSource> AsyncSource::onMakeDecoded(Context*) const {
  return nullptr;
}

std::shared_ptr<TextureProxy> AsyncSource::onMakeTextureProxy(Context* context,
                                                              uint32_t renderFlags) const {
  return context->proxyProvider()->createTextureProxy(resourceKey, imageDecoder, mipMapped,
                                                      renderFlags);
}
}  // namespace tgfx
