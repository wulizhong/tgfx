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

#include "BlurImageFilter.h"
#include "gpu/SurfaceDrawContext.h"
#include "gpu/TextureSampler.h"
#include "gpu/processors/DualBlurFragmentProcessor.h"

namespace tgfx {
static const float BLUR_LEVEL_1_LIMIT = 10.0f;
static const float BLUR_LEVEL_2_LIMIT = 15.0f;
static const float BLUR_LEVEL_3_LIMIT = 55.0f;
static const float BLUR_LEVEL_4_LIMIT = 120.0f;
static const float BLUR_LEVEL_5_LIMIT = 300.0f;

static const float BLUR_LEVEL_MAX_LIMIT = BLUR_LEVEL_5_LIMIT;

static const int BLUR_LEVEL_1_DEPTH = 1;
static const int BLUR_LEVEL_2_DEPTH = 2;
static const int BLUR_LEVEL_3_DEPTH = 2;
static const int BLUR_LEVEL_4_DEPTH = 3;
static const int BLUR_LEVEL_5_DEPTH = 3;

static const float BLUR_LEVEL_1_SCALE = 1.0f;
static const float BLUR_LEVEL_2_SCALE = 0.8f;
static const float BLUR_LEVEL_3_SCALE = 0.5f;
static const float BLUR_LEVEL_4_SCALE = 0.5f;
static const float BLUR_LEVEL_5_SCALE = 0.5f;

static const float BLUR_STABLE = 10.0f;

static std::tuple<int, float, float> Get(float blurriness) {
  blurriness = blurriness < BLUR_LEVEL_MAX_LIMIT ? blurriness : BLUR_LEVEL_MAX_LIMIT;
  if (blurriness < BLUR_LEVEL_1_LIMIT) {
    return {BLUR_LEVEL_1_DEPTH, BLUR_LEVEL_1_SCALE, blurriness / BLUR_LEVEL_1_LIMIT * 2.0};
  } else if (blurriness < BLUR_LEVEL_2_LIMIT) {
    return {BLUR_LEVEL_2_DEPTH, BLUR_LEVEL_2_SCALE,
            (blurriness - BLUR_STABLE) / (BLUR_LEVEL_2_LIMIT - BLUR_STABLE) * 3.0};
  } else if (blurriness < BLUR_LEVEL_3_LIMIT) {
    return {BLUR_LEVEL_3_DEPTH, BLUR_LEVEL_3_SCALE,
            (blurriness - BLUR_STABLE) / (BLUR_LEVEL_3_LIMIT - BLUR_STABLE) * 5.0};
  } else if (blurriness < BLUR_LEVEL_4_LIMIT) {
    return {BLUR_LEVEL_4_DEPTH, BLUR_LEVEL_4_SCALE,
            (blurriness - BLUR_STABLE) / (BLUR_LEVEL_4_LIMIT - BLUR_STABLE) * 6.0};
  } else {
    return {
        BLUR_LEVEL_5_DEPTH, BLUR_LEVEL_5_SCALE,
        6.0 + (blurriness - BLUR_STABLE * 12.0) / (BLUR_LEVEL_5_LIMIT - BLUR_STABLE * 12.0) * 5.0};
  }
}

std::shared_ptr<ImageFilter> ImageFilter::Blur(float blurrinessX, float blurrinessY,
                                               TileMode tileMode, const Rect* cropRect) {
  if (blurrinessX < 0 || blurrinessY < 0 || (blurrinessX == 0 && blurrinessY == 0)) {
    return nullptr;
  }
  auto x = Get(blurrinessX);
  auto y = Get(blurrinessY);
  return std::make_shared<BlurImageFilter>(
      Point::Make(std::get<2>(x), std::get<2>(y)), std::max(std::get<1>(x), std::get<1>(y)),
      std::max(std::get<0>(x), std::get<0>(y)), tileMode, cropRect);
}

BlurImageFilter::BlurImageFilter(Point blurOffset, float downScaling, int iteration,
                                 TileMode tileMode, const Rect* cropRect)
    : ImageFilter(cropRect), blurOffset(blurOffset), downScaling(downScaling), iteration(iteration),
      tileMode(tileMode) {
}

void BlurImageFilter::draw(Surface* toSurface, std::shared_ptr<Image> image, bool isDown,
                           const Rect* imageBounds, TileMode mode) const {
  auto drawContext = std::make_unique<SurfaceDrawContext>(toSurface);
  auto dstRect = Rect::MakeWH(toSurface->width(), toSurface->height());
  auto textureWidth = imageBounds ? imageBounds->width() : static_cast<float>(image->width());
  auto textureHeight = imageBounds ? imageBounds->height() : static_cast<float>(image->height());
  auto texelSize = Size::Make(0.5f / textureWidth, 0.5f / textureHeight);
  auto localMatrix =
      Matrix::MakeScale(textureWidth / dstRect.width(), textureHeight / dstRect.height());
  if (imageBounds) {
    localMatrix.postTranslate(imageBounds->x(), imageBounds->y());
  }
  ImageFPArgs args(toSurface->getContext(), {}, toSurface->options()->renderFlags(), mode, mode);
  auto processor = FragmentProcessor::MakeFromImage(image, args);
  drawContext->fillRectWithFP(
      dstRect, localMatrix,
      DualBlurFragmentProcessor::Make(isDown ? DualBlurPassMode::Down : DualBlurPassMode::Up,
                                      std::move(processor), blurOffset, texelSize));
}

Rect BlurImageFilter::onFilterBounds(const Rect& srcRect) const {
  auto mul = static_cast<float>(std::pow(2, iteration)) / downScaling;
  return srcRect.makeOutset(blurOffset.x * mul, blurOffset.y * mul);
}

std::unique_ptr<FragmentProcessor> BlurImageFilter::asFragmentProcessor(
    std::shared_ptr<Image> source, const ImageFPArgs& args, const Matrix* localMatrix,
    const Rect* subset) const {
  auto inputBounds = Rect::MakeWH(source->width(), source->height());
  Rect dstBounds = Rect::MakeEmpty();
  if (!applyCropRect(inputBounds, &dstBounds, subset)) {
    return nullptr;
  }
  int tw = static_cast<int>(dstBounds.width() * downScaling);
  int th = static_cast<int>(dstBounds.height() * downScaling);
  std::vector<std::pair<int, int>> upSurfaces;
  upSurfaces.emplace_back(static_cast<int>(dstBounds.width()),
                          static_cast<int>(dstBounds.height()));
  std::shared_ptr<Image> lastImage = nullptr;
  for (int i = 0; i < iteration; ++i) {
    auto surface = Surface::Make(args.context, tw, th);
    if (surface == nullptr) {
      return {};
    }
    upSurfaces.emplace_back(tw, th);
    if (lastImage == nullptr) {
      draw(surface.get(), source, true, &dstBounds, tileMode);
    } else {
      draw(surface.get(), lastImage, true);
    }
    lastImage = surface->makeImageSnapshot();
    tw = std::max(static_cast<int>(static_cast<float>(tw) * downScaling), 1);
    th = std::max(static_cast<int>(static_cast<float>(th) * downScaling), 1);
  }
  for (size_t i = upSurfaces.size() - 1; i > 0; --i) {
    auto width = upSurfaces[i - 1].first;
    auto height = upSurfaces[i - 1].second;
    auto surface = Surface::Make(args.context, width, height);
    if (surface == nullptr) {
      return {};
    }
    draw(surface.get(), lastImage, false);
    lastImage = surface->makeImageSnapshot();
  }
  auto matrix = Matrix::MakeTrans(-dstBounds.x(), -dstBounds.y());
  if (localMatrix != nullptr) {
    matrix.preConcat(*localMatrix);
  }
  return FragmentProcessor::MakeFromImage(lastImage, args, &matrix);
}
}  // namespace tgfx
