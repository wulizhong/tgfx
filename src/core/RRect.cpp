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

#include "tgfx/core/RRect.h"

namespace tgfx {

void RRect::setRectXY(const Rect& r, float radiusX, float radiusY) {
  rect = r.makeSorted();
  radii = {radiusX, radiusY};
}

void RRect::setOval(const Rect& oval) {
  rect = oval.makeSorted();
  radii = {rect.width() / 2, rect.height() / 2};
}

void RRect::scale(float scaleX, float scaleY) {
  rect.scale(scaleX, scaleY);
  radii.x *= scaleX;
  radii.y *= scaleY;
}
}  // namespace tgfx
