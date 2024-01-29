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

#include <optional>
#include "tgfx/core/BlendMode.h"
#include "tgfx/core/Font.h"
#include "tgfx/core/Image.h"
#include "tgfx/core/Paint.h"
#include "tgfx/core/Path.h"
#include "tgfx/core/SamplingOptions.h"
#include "tgfx/core/Shape.h"
#include "tgfx/core/TextBlob.h"

namespace tgfx {
class Surface;
class SurfaceOptions;
class RenderContext;
class Texture;
class TextureProxy;
struct CanvasState;

/**
 * Canvas provides an interface for drawing, and how the drawing is clipped and transformed. Canvas
 * contains a stack of opacity, blend mode, matrix and clip values. Each Canvas draw call transforms
 * the geometry of the object by the concatenation of all matrix values in the stack. The
 * transformed geometry is clipped by the intersection of all of clip values in the stack.
 */
class Canvas {
 public:
  explicit Canvas(Surface* surface);

  /**
   * Retrieves the context associated with this Surface.
   */
  Context* getContext() const;

  /**
   * Returns the Surface this canvas draws into.
   */
  Surface* getSurface() const;

  /**
   * Returns the SurfaceOptions associated with the Canvas. Returns nullptr if the Canvas is not
   * created from a Surface.
   */
  const SurfaceOptions* surfaceOptions() const;

  /**
   * Saves alpha, blend mode, matrix, and clip. Calling restore() discards changes to them,
   * restoring them to their state when save() was called. Saved Canvas state is put on a stack,
   * multiple calls to save() should be balanced by an equal number of calls to restore().
   */
  void save();

  /**
   * Removes changes to alpha, blend mode, matrix and clips since Canvas state was last saved. The
   * state is removed from the stack. Does nothing if the stack is empty.
   */
  void restore();

  /**
   * Returns the current total matrix.
   */
  Matrix getMatrix() const;

  /**
   * Replaces transformation with specified matrix. Unlike concat(), any prior matrix state is
   * overwritten.
   * @param matrix  matrix to copy, replacing existing Matrix
   */
  void setMatrix(const Matrix& matrix);

  /**
   * Sets Matrix to the identity matrix. Any prior matrix state is overwritten.
   */
  void resetMatrix();

  /**
   * Replaces the current Matrix with matrix premultiplied with the existing one. This has the
   * effect of transforming the drawn geometry by matrix, before transforming the result with the
   * existing Matrix.
   */
  void concat(const Matrix& matrix);

  /**
   * Returns the current global alpha.
   */
  float getAlpha() const;

  /**
   * Replaces the global alpha with specified newAlpha.
   */
  void setAlpha(float newAlpha);

  /**
   * Returns the current global blend mode.
   */
  BlendMode getBlendMode() const;

  /**
   * Replaces the global blend mode with specified new blend mode.
   */
  void setBlendMode(BlendMode blendMode);

  /**
   * Returns the current total clip.
   */
  Path getTotalClip() const;

  /**
   * Replaces clip with the intersection of clip and rect. The resulting clip is aliased; pixels are
   * fully contained by the clip. The rect is transformed by the current Matrix before it is
   * combined with clip.
   */
  void clipRect(const Rect& rect);

  /**
   * Replaces clip with the intersection of clip and path. The path is transformed by Matrix before
   * it is combined with clip.
   */
  void clipPath(const Path& path);

  /**
   * Fills clip with color. This has the effect of replacing all pixels contained by clip with
   * color.
   */
  void clear(const Color& color = Color::Transparent());

  /**
   * Draws a rectangle with specified paint, using current alpha, blend mode, clip and
   * matrix.
   */
  void drawRect(const Rect& rect, const Paint& paint);

  /**
   * Draws a path with using current clip, matrix and specified paint.
   */
  void drawPath(const Path& path, const Paint& paint);

  /**
   * Draws a shape with using current clip, matrix and specified paint.
   */
  void drawShape(std::shared_ptr<Shape> shape, const Paint& paint);

  /**
   * Draws an image, with its top-left corner at (left, top), using current clip, matrix and
   * optional paint. If image->hasMipmaps() is true, uses FilterMode::Linear and MipMapMode::Linear
   * as the sampling options. Otherwise, uses FilterMode::Linear and MipMapMode::None as the
   * sampling options.
   */
  void drawImage(std::shared_ptr<Image> image, float left, float top, const Paint* paint = nullptr);

  /**
   * Draws a Image, with its top-left corner at (0, 0), using current alpha, clip and matrix
   * premultiplied with existing Matrix. If image->hasMipmaps() is true, uses FilterMode::Linear
   * and MipMapMode::Linear as the sampling options. Otherwise, uses FilterMode::Linear and
   * MipMapMode::None as the sampling options.
   */
  void drawImage(std::shared_ptr<Image> image, const Matrix& matrix, const Paint* paint = nullptr);

  /**
   * Draws an image, with its top-left corner at (0, 0), using current clip, matrix and optional
   * paint. If image->hasMipmaps() is true, uses FilterMode::Linear and MipMapMode::Linear as the
   * sampling options. Otherwise, uses FilterMode::Linear and MipMapMode::None as the sampling
   * options.
   */
  void drawImage(std::shared_ptr<Image> image, const Paint* paint = nullptr);

  /**
   * Draws an image, with its top-left corner at (0, 0), using current clip, matrix, sampling
   * options and optional paint.
   */
  void drawImage(std::shared_ptr<Image> image, SamplingOptions sampling,
                 const Paint* paint = nullptr);

  /**
   * Draws text, with origin at (x, y), using clip, matrix, font, and paint. The text must be in
   * utf-8 encoding. This function uses the default character-to-glyph mapping from the Typeface in
   * font. It does not perform typeface fallback for characters not found in the Typeface. Glyphs
   * are positioned based on their default advances.
   */
  void drawSimpleText(const std::string& text, float x, float y, const Font& font,
                      const Paint& paint);

  /**
   * Draw an array of glyphs with specified font, using current alpha, blend mode, clip and Matrix.
   */
  void drawGlyphs(const GlyphID glyphIDs[], const Point positions[], size_t glyphCount,
                  const Font& font, const Paint& paint);

  // TODO(pengweilv): Support blend mode, atlas as source, colors as destination, colors can be
  //  nullptr.
  void drawAtlas(std::shared_ptr<Image> atlas, const Matrix matrix[], const Rect tex[],
                 const Color colors[], size_t count, SamplingOptions sampling = SamplingOptions());

  /**
   * Triggers the immediate execution of all pending draw operations.
   */
  void flush();

 private:
  Surface* surface = nullptr;
  std::shared_ptr<Surface> _clipSurface = nullptr;
  uint32_t clipID = 0;
  std::shared_ptr<CanvasState> state = nullptr;
  std::vector<std::shared_ptr<CanvasState>> savedStateList = {};

  bool nothingToDraw(const Paint& paint) const;
  std::shared_ptr<TextureProxy> getClipTexture();
  std::pair<std::optional<Rect>, bool> getClipRect();
  std::unique_ptr<FragmentProcessor> getClipMask(const Rect& deviceBounds, Rect* scissorRect);
  Rect clipLocalBounds(Rect localBounds);
  std::unique_ptr<FragmentProcessor> getImageProcessor(std::shared_ptr<Image> image,
                                                       SamplingOptions sampling,
                                                       const Rect& clipBounds,
                                                       std::shared_ptr<Shader> shader);
  void drawMask(const Rect& bounds, std::shared_ptr<TextureProxy> mask, const Paint& paint);
  void drawColorGlyphs(const GlyphID glyphIDs[], const Point positions[], size_t glyphCount,
                       const Font& font, const Paint& paint);
  void drawMaskGlyphs(std::shared_ptr<TextBlob> textBlob, const Paint& paint);
  void fillPath(const Path& path, const Paint& paint);
  bool drawAsClear(const Path& path, const Paint& paint);
  void drawOp(std::unique_ptr<DrawOp> op, const Paint& paint, bool aa = false);
  Color getInputColor(const Paint& paint);
  bool getProcessors(const Paint& paint, DrawOp* drawOp);
};
}  // namespace tgfx
