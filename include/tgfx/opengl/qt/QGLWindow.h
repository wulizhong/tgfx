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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-copy"
#include <QQuickItem>
#include <QSGTexture>
#pragma clang diagnostic pop
#include "QGLDevice.h"
#include "tgfx/gpu/DoubleBufferedWindow.h"

namespace tgfx {
class Texture;
class GLRenderTarget;

class QGLWindow : public DoubleBufferedWindow {
 public:
  ~QGLWindow() override;

  /**
   * Creates a new QGLWindow from specified QQuickItem and shared context.
   * Note: Due to the fact that QOffscreenSurface is backed by a QWindow on some platforms,
   * cross-platform applications must ensure that this method is only called on the main (GUI)
   * thread. The returned QGLWindow is then safe to be used on other threads after calling
   * moveToThread(), but the initialization and destruction must always happen on the main (GUI)
   * thread.
   */
  static std::shared_ptr<QGLWindow> MakeFrom(QQuickItem* quickItem,
                                             QOpenGLContext* sharedContext = nullptr);

  /**
   * Changes the thread affinity for this object and its children.
   */
  void moveToThread(QThread* targetThread);

  /**
   * Returns the current QSGTexture for displaying.
   */
  QSGTexture* getTexture();

 protected:
  std::shared_ptr<Surface> onCreateSurface(Context* context) override;
  void onSwapSurfaces(Context*) override;

 private:
  std::mutex locker = {};
  bool textureInvalid = true;
  QQuickItem* quickItem = nullptr;
  QSGTexture* outTexture = nullptr;

  QGLWindow(std::shared_ptr<Device> device, QQuickItem* quickItem);
  void invalidateTexture();
};
}  // namespace tgfx
