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

#include "tgfx/core/Canvas.h"
#include "core/DrawContext.h"
#include "core/FillStyle.h"
#include "core/PathRef.h"
#include "gpu/DrawingManager.h"
#include "gpu/ProxyProvider.h"
#include "gpu/ops/FillRectOp.h"
#include "tgfx/core/PathEffect.h"
#include "tgfx/gpu/Surface.h"
#include "utils/SimpleTextShaper.h"
#include "utils/StrokeKey.h"

namespace tgfx {
Canvas::Canvas(DrawContext* drawContext) : drawContext(drawContext) {
}

void Canvas::save() {
  drawContext->save();
}

void Canvas::restore() {
  drawContext->restore();
}

void Canvas::translate(float dx, float dy) {
  drawContext->concat(Matrix::MakeTrans(dx, dy));
}

void Canvas::scale(float sx, float sy) {
  drawContext->concat(Matrix::MakeScale(sx, sy));
}

void Canvas::rotate(float degrees) {
  drawContext->concat(Matrix::MakeRotate(degrees));
}

void Canvas::rotate(float degress, float px, float py) {
  drawContext->concat(Matrix::MakeRotate(degress, px, py));
}

void Canvas::skew(float sx, float sy) {
  drawContext->concat(Matrix::MakeSkew(sx, sy));
}

void Canvas::concat(const Matrix& matrix) {
  drawContext->concat(matrix);
}

Matrix Canvas::getMatrix() const {
  return drawContext->getMatrix();
}

void Canvas::setMatrix(const Matrix& matrix) {
  drawContext->setMatrix(matrix);
}

void Canvas::resetMatrix() {
  drawContext->resetMatrix();
}

Path Canvas::getTotalClip() const {
  return drawContext->getClip();
}

void Canvas::clipRect(const tgfx::Rect& rect) {
  drawContext->clipRect(rect);
}

void Canvas::clipPath(const Path& path) {
  drawContext->clipPath(path);
}

void Canvas::clear(const Color& color) {
  Paint paint;
  paint.setColor(color);
  paint.setBlendMode(BlendMode::Src);
  // TODO: We can't access the surface directly.
  auto surface = drawContext->getSurface();
  auto rect = Rect::MakeWH(surface->width(), surface->height());
  drawRect(rect, paint);
}

void Canvas::drawLine(float x0, float y0, float x1, float y1, const Paint& paint) {
  Path path = {};
  path.moveTo(x0, y0);
  path.lineTo(x1, y1);
  auto realPaint = paint;
  realPaint.setStyle(PaintStyle::Stroke);
  drawPath(path, realPaint);
}

void Canvas::drawRect(const Rect& rect, const Paint& paint) {
  Path path = {};
  path.addRect(rect);
  drawPath(path, paint);
}

void Canvas::drawOval(const Rect& oval, const Paint& paint) {
  Path path = {};
  path.addOval(oval);
  drawPath(path, paint);
}

void Canvas::drawCircle(float centerX, float centerY, float radius, const Paint& paint) {
  Rect rect =
      Rect::MakeLTRB(centerX - radius, centerY - radius, centerX + radius, centerY + radius);
  drawOval(rect, paint);
}

Context* Canvas::getContext() const {
  auto surface = drawContext->getSurface();
  return surface ? surface->getContext() : nullptr;
}

Surface* Canvas::getSurface() const {
  return drawContext->getSurface();
}

const SurfaceOptions* Canvas::surfaceOptions() const {
  auto surface = drawContext->getSurface();
  return surface ? surface->options() : nullptr;
}

static FillStyle CreateFillStyle(const Paint& paint) {
  FillStyle style = {};
  auto shader = paint.getShader();
  Color color = {};
  if (shader && shader->asColor(&color)) {
    color.alpha *= paint.getAlpha();
    style.color = color.premultiply();
    shader = nullptr;
  } else {
    style.color = paint.getColor().premultiply();
  }
  style.shader = shader;
  style.antiAlias = paint.isAntiAlias();
  style.colorFilter = paint.getColorFilter();
  style.maskFilter = paint.getMaskFilter();
  style.blendMode = paint.getBlendMode();
  return style;
}

static FillStyle CreateFillStyle(const Paint* paint) {
  return paint ? CreateFillStyle(*paint) : FillStyle();
}

void Canvas::drawPath(const Path& path, const Paint& paint) {
  if (path.isEmpty() || paint.nothingToDraw()) {
    return;
  }
  auto stroke = paint.getStroke();
  auto style = CreateFillStyle(paint);
  if (stroke && path.isLine()) {
    auto effect = PathEffect::MakeStroke(stroke);
    if (effect != nullptr) {
      auto fillPath = path;
      effect->applyTo(&fillPath);
      if (drawSimplePath(fillPath, style)) {
        return;
      }
    }
  }
  if (!stroke && drawSimplePath(path, style)) {
    return;
  }
  drawContext->drawPath(path, style, stroke);
}

bool Canvas::drawSimplePath(const Path& path, const FillStyle& style) {
  Rect rect = {};
  if (path.asRect(&rect)) {
    drawContext->drawRect(rect, style);
    return true;
  }
  RRect rRect;
  if (path.asRRect(&rRect)) {
    drawContext->drawRRect(rRect, style);
    return true;
  }
  return false;
}

static SamplingOptions GetDefaultSamplingOptions(std::shared_ptr<Image> image) {
  if (image == nullptr) {
    return {};
  }
  auto mipmapMode = image->hasMipmaps() ? MipmapMode::Linear : MipmapMode::None;
  return SamplingOptions(FilterMode::Linear, mipmapMode);
}

void Canvas::drawImage(std::shared_ptr<Image> image, float left, float top, const Paint* paint) {
  drawImage(std::move(image), Matrix::MakeTrans(left, top), paint);
}

void Canvas::drawImage(std::shared_ptr<Image> image, const Matrix& matrix, const Paint* paint) {
  auto sampling = GetDefaultSamplingOptions(image);
  drawImage(std::move(image), sampling, &matrix, paint);
}

void Canvas::drawImage(std::shared_ptr<Image> image, const Paint* paint) {
  auto sampling = GetDefaultSamplingOptions(image);
  drawImage(std::move(image), sampling, nullptr, paint);
}

void Canvas::drawImage(std::shared_ptr<Image> image, SamplingOptions sampling, const Paint* paint) {
  drawImage(std::move(image), sampling, nullptr, paint);
}

void Canvas::drawImage(std::shared_ptr<Image> image, SamplingOptions sampling,
                       const Matrix* extraMatrix, const Paint* paint) {
  if (image == nullptr || (paint && paint->nothingToDraw())) {
    return;
  }
  auto matrix = extraMatrix ? *extraMatrix : Matrix::I();
  auto imageFilter = paint ? paint->getImageFilter() : nullptr;
  if (imageFilter != nullptr) {
    auto offset = Point::Zero();
    image = image->makeWithFilter(std::move(imageFilter), &offset);
    if (image == nullptr) {
      return;
    }
    matrix.preTranslate(offset.x, offset.y);
  }
  auto rect = Rect::MakeWH(image->width(), image->height());
  auto style = CreateFillStyle(paint);
  drawImageRect(rect, std::move(image), sampling, style, matrix);
}

void Canvas::drawImageRect(const Rect& rect, std::shared_ptr<Image> image, SamplingOptions sampling,
                           const FillStyle& style, const Matrix& extraMatrix) {
  auto hasExtraMatrix = !extraMatrix.isIdentity();
  if (hasExtraMatrix) {
    drawContext->save();
    drawContext->concat(extraMatrix);
  }
  drawContext->drawImageRect(rect, std::move(image), sampling, style);
  if (hasExtraMatrix) {
    drawContext->restore();
  }
}

void Canvas::drawSimpleText(const std::string& text, float x, float y, const tgfx::Font& font,
                            const tgfx::Paint& paint) {
  if (text.empty() || paint.nothingToDraw()) {
    return;
  }
  auto glyphRun = SimpleTextShaper::Shape(text, font);
  if (x != 0 || y != 0) {
    drawContext->save();
    translate(x, y);
  }
  auto style = CreateFillStyle(paint);
  drawContext->drawGlyphRun(std::move(glyphRun), style, paint.getStroke());
  if (x != 0 || y != 0) {
    drawContext->restore();
  }
}

void Canvas::drawGlyphs(const GlyphID glyphs[], const Point positions[], size_t glyphCount,
                        const Font& font, const Paint& paint) {
  if (glyphCount == 0 || paint.nothingToDraw()) {
    return;
  }
  GlyphRun glyphRun(font, {glyphs, glyphs + glyphCount}, {positions, positions + glyphCount});
  auto style = CreateFillStyle(paint);
  drawContext->drawGlyphRun(std::move(glyphRun), style, paint.getStroke());
}

void Canvas::drawAtlas(std::shared_ptr<Image> atlas, const Matrix matrix[], const Rect tex[],
                       const Color colors[], size_t count, SamplingOptions sampling,
                       const Paint* paint) {
  // TODO: Support blend mode, atlas as source, colors as destination, colors can be nullptr.
  if (atlas == nullptr || count == 0 || (paint && paint->nothingToDraw())) {
    return;
  }
  auto style = CreateFillStyle(paint);
  for (size_t i = 0; i < count; ++i) {
    auto rect = tex[i];
    auto glyphMatrix = matrix[i];
    glyphMatrix.preTranslate(-rect.x(), -rect.y());
    auto glyphStyle = style;
    if (colors) {
      glyphStyle.color = colors[i].premultiply();
    }
    drawImageRect(rect, atlas, sampling, glyphStyle, glyphMatrix);
  }
}
}  // namespace tgfx
