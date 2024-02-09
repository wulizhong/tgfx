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

#include "FilterImage.h"
#include "SubsetImage.h"
#include "gpu/processors/FragmentProcessor.h"

namespace tgfx {
std::shared_ptr<Image> FilterImage::MakeFrom(std::shared_ptr<Image> source,
                                             std::shared_ptr<Filter> filter, Point* offset,
                                             const Rect* clipRect) {
  if (source == nullptr) {
    return nullptr;
  }
  if (filter == nullptr) {
    return source;
  }
  auto bounds = Rect::MakeWH(source->width(), source->height());
  bounds = filter->filterBounds(bounds);
  if (bounds.isEmpty()) {
    return nullptr;
  }
  if (clipRect != nullptr) {
    if (!bounds.intersect(*clipRect)) {
      return nullptr;
    }
    bounds.roundOut();
  }
  if (offset != nullptr) {
    offset->x = bounds.left;
    offset->y = bounds.top;
  }
  return MakeFrom(std::move(source), std::move(filter), bounds);
}

std::shared_ptr<Image> FilterImage::MakeFrom(std::shared_ptr<Image> source,
                                             std::shared_ptr<Filter> filter, const Rect& bounds) {
  auto image =
      std::shared_ptr<FilterImage>(new FilterImage(std::move(source), std::move(filter), bounds));
  image->weakThis = image;
  return image;
}

FilterImage::FilterImage(std::shared_ptr<Image> source, std::shared_ptr<Filter> filter,
                         const Rect& bounds)
    : TransformImage(std::move(source)), filter(std::move(filter)), bounds(bounds) {
}

std::shared_ptr<Image> FilterImage::onCloneWith(std::shared_ptr<Image> newSource) const {
  return FilterImage::MakeFrom(std::move(newSource), filter);
}

std::shared_ptr<Image> FilterImage::onMakeSubset(const Rect& subset) const {
  auto newBounds = subset;
  newBounds.offset(bounds.x(), bounds.y());
  return FilterImage::MakeFrom(source, filter, newBounds);
}

std::unique_ptr<FragmentProcessor> FilterImage::onMakeFragmentProcessor(const DrawArgs& args,
                                                                        const Matrix* localMatrix,
                                                                        TileMode tileModeX,
                                                                        TileMode tileModeY) const {
  auto matrix = SubsetImage::ConcatLocalMatrix(bounds, localMatrix);
  return filter->onMakeFragmentProcessor(source, args, AddressOf(matrix), tileModeX, tileModeY);
}
}  // namespace tgfx
