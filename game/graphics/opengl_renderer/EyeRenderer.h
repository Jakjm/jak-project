#pragma once

#include <string>
#include "game/graphics/opengl_renderer/BucketRenderer.h"
#include "game/graphics/pipelines/opengl.h"

constexpr int EYE_BASE_BLOCK = 8160;
constexpr int NUM_EYE_PAIRS = 11;
constexpr int SINGLE_EYE_SIZE = 32;

class EyeRenderer : public BucketRenderer {
 public:
  EyeRenderer(const std::string& name, BucketId id);
  void render(DmaFollower& dma, SharedRenderState* render_state, ScopedProfilerNode& prof) override;
  void draw_debug_window() override;

  template <bool DEBUG>
  void handle_eye_dma2(DmaFollower& dma, SharedRenderState* render_state, ScopedProfilerNode& prof);

  TextureRecord* get_eye_tex(SharedRenderState* render_state, int pair_idx, int lr);

 private:
  std::string m_debug;
  float m_average_time_ms = 0;

  bool m_use_bilinear = false;
  bool m_alpha_hack = true;

  u32 m_left[SINGLE_EYE_SIZE * SINGLE_EYE_SIZE];
  u32 m_right[SINGLE_EYE_SIZE * SINGLE_EYE_SIZE];
};
