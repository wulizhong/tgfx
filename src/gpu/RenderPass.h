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
#include "Blend.h"
#include "GpuBuffer.h"
#include "Program.h"
#include "gpu/ProgramInfo.h"
#include "gpu/RenderTarget.h"
#include "gpu/processors/GeometryProcessor.h"
#include "tgfx/core/BlendMode.h"
#include "tgfx/core/Color.h"
#include "tgfx/gpu/Context.h"

namespace tgfx {
/**
 * Geometric primitives used for drawing.
 */
enum class PrimitiveType {
  Triangles,
  TriangleStrip,
};

class RenderPass {
 public:
  explicit RenderPass(Context* context) : _context(context) {
  }

  Context* context() {
    return _context;
  }

  std::shared_ptr<RenderTarget> renderTarget() {
    return _renderTarget;
  }

  std::shared_ptr<Texture> renderTargetTexture() {
    return _renderTargetTexture;
  }

  virtual ~RenderPass() = default;

  void begin();
  void end();
  void bindProgramAndScissorClip(const ProgramInfo* programInfo, const Rect& drawBounds);
  void bindBuffers(std::shared_ptr<GpuBuffer> indexBuffer, std::shared_ptr<GpuBuffer> vertexBuffer);
  void draw(PrimitiveType primitiveType, int baseVertex, int vertexCount);
  void drawIndexed(PrimitiveType primitiveType, int baseIndex, int indexCount);
  void clear(const Rect& scissor, Color color);

 protected:
  virtual bool onBindProgramAndScissorClip(const ProgramInfo* programInfo,
                                           const Rect& drawBounds) = 0;
  virtual void onBindBuffers(std::shared_ptr<GpuBuffer> indexBuffer,
                             std::shared_ptr<GpuBuffer> vertexBuffer) = 0;
  virtual void onDraw(PrimitiveType primitiveType, int baseVertex, int vertexCount) = 0;
  virtual void onDrawIndexed(PrimitiveType primitiveType, int baseIndex, int indexCount) = 0;
  virtual void onClear(const Rect& scissor, Color color) = 0;

  Context* _context = nullptr;
  std::shared_ptr<RenderTarget> _renderTarget = nullptr;
  std::shared_ptr<Texture> _renderTargetTexture = nullptr;
  Program* _program = nullptr;
  std::shared_ptr<GpuBuffer> _indexBuffer;
  std::shared_ptr<GpuBuffer> _vertexBuffer;

 private:
  enum class DrawPipelineStatus { Ok = 0, NotConfigured, FailedToBind };

  void resetActiveBuffers();

  DrawPipelineStatus drawPipelineStatus = DrawPipelineStatus::NotConfigured;
};
}  // namespace tgfx
