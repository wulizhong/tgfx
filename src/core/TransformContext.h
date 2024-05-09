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

#include "core/DrawContext.h"

namespace tgfx {
/**
 * A context that applies a transformation to the state before passing it to the draw context.
 */
class TransformContext : public DrawContext {
 public:
  /**
   * Creates a new transform context that applies the given matrix to the state. Returns nullptr if
   * the matrix is identity or the drawContext is nullptr.
   */
  static std::unique_ptr<TransformContext> Make(DrawContext* drawContext, const Matrix& matrix);

  /**
   * Creates a new transform context that applies the given matrix and clip to the state. Returns
   * nullptr if the matrix is identity and the clip is wide open, or the drawContext is nullptr, or
   * the clip is empty.
   */
  static std::unique_ptr<TransformContext> Make(DrawContext* drawContext, const Matrix& matrix,
                                                const Path& clip);

  explicit TransformContext(DrawContext* drawContext) : drawContext(drawContext) {
  }

  void clear() override {
    drawContext->clear();
  }

  void drawRect(const Rect& rect, const MCState& state, const FillStyle& style) override {
    drawContext->drawRect(rect, transform(state), style);
  }

  void drawRRect(const RRect& rRect, const MCState& state, const FillStyle& style) override {
    drawContext->drawRRect(rRect, transform(state), style);
  }

  void drawPath(const Path& path, const MCState& state, const FillStyle& style,
                const Stroke* stroke) override {
    drawContext->drawPath(path, transform(state), style, stroke);
  }

  void drawImageRect(std::shared_ptr<Image> image, const SamplingOptions& sampling,
                     const Rect& rect, const MCState& state, const FillStyle& style) override {
    drawContext->drawImageRect(std::move(image), sampling, rect, transform(state), style);
  }

  void drawGlyphRun(GlyphRun glyphRun, const MCState& state, const FillStyle& style,
                    const Stroke* stroke) override {
    drawContext->drawGlyphRun(std::move(glyphRun), transform(state), style, stroke);
  }

  void drawPicture(std::shared_ptr<Picture> picture, const MCState& state) override {
    drawContext->drawPicture(std::move(picture), transform(state));
  }

  void drawLayer(std::shared_ptr<Picture> picture, const MCState& state, const FillStyle& style,
                 std::shared_ptr<ImageFilter> filter) override {
    drawContext->drawLayer(std::move(picture), transform(state), style, std::move(filter));
  }

 protected:
  virtual MCState transform(const MCState& state) = 0;

 private:
  DrawContext* drawContext = nullptr;
};
}  // namespace tgfx
