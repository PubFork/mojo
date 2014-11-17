// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TEST_FAKE_LAYER_TREE_HOST_IMPL_CLIENT_H_
#define CC_TEST_FAKE_LAYER_TREE_HOST_IMPL_CLIENT_H_

#include "cc/output/begin_frame_args.h"
#include "cc/trees/layer_tree_host_impl.h"

namespace cc {

class FakeLayerTreeHostImplClient : public LayerTreeHostImplClient {
 public:
  // LayerTreeHostImplClient implementation.
  void UpdateRendererCapabilitiesOnImplThread() override {}
  void DidLoseOutputSurfaceOnImplThread() override {}
  void CommitVSyncParameters(base::TimeTicks timebase,
                             base::TimeDelta interval) override {}
  void SetEstimatedParentDrawTime(base::TimeDelta draw_time) override {}
  void SetMaxSwapsPendingOnImplThread(int max) override {}
  void DidSwapBuffersOnImplThread() override {}
  void DidSwapBuffersCompleteOnImplThread() override {}
  void OnCanDrawStateChanged(bool can_draw) override {}
  void NotifyReadyToActivate() override {}
  void NotifyReadyToDraw() override {}
  void SetNeedsRedrawOnImplThread() override {}
  void SetNeedsRedrawRectOnImplThread(const gfx::Rect& damage_rect) override {}
  void SetNeedsAnimateOnImplThread() override {}
  void DidInitializeVisibleTileOnImplThread() override {}
  void SetNeedsCommitOnImplThread() override {}
  void SetNeedsManageTilesOnImplThread() override {}
  void PostAnimationEventsToMainThreadOnImplThread(
      scoped_ptr<AnimationEventsVector> events) override {}
  bool ReduceContentsTextureMemoryOnImplThread(size_t limit_bytes,
                                               int priority_cutoff) override;
  bool IsInsideDraw() override;
  void RenewTreePriority() override {}
  void PostDelayedScrollbarFadeOnImplThread(const base::Closure& start_fade,
                                            base::TimeDelta delay) override {}
  void DidActivateSyncTree() override {}
  void DidManageTiles() override {}
};

}  // namespace cc

#endif  // CC_TEST_FAKE_LAYER_TREE_HOST_IMPL_CLIENT_H_
