#pragma once

#include <string>
#include <memory>
#include "common/dma/dma_chain_read.h"
#include "game/graphics/opengl_renderer/Shader.h"
#include "game/graphics/texture/TexturePool.h"
#include "game/graphics/opengl_renderer/Profiler.h"
#include "game/graphics/opengl_renderer/Loader.h"

/*!
 * Matches the bucket-id enum in GOAL
 */
enum class BucketId {
  BUCKET0 = 0,
  BUCKET1 = 1,
  SKY_DRAW = 3,
  TFRAG_TEX_LEVEL0 = 5,
  TFRAG_LEVEL0 = 6,
  TIE_LEVEL0 = 9,
  MERC_TFRAG_TEX_LEVEL0 = 10,
  TFRAG_TEX_LEVEL1 = 12,
  TFRAG_LEVEL1 = 13,
  TIE_LEVEL1 = 16,
  MERC_TFRAG_TEX_LEVEL1 = 17,
  SHRUB_TEX_LEVEL0 = 19,
  SHRUB_TEX_LEVEL1 = 25,
  ALPHA_TEX_LEVEL0 = 31,
  TFRAG_TRANS0_AND_SKY_BLEND_LEVEL0 = 32,
  TFRAG_DIRT_LEVEL0 = 34,
  TFRAG_ICE_LEVEL0 = 36,
  ALPHA_TEX_LEVEL1 = 38,
  TFRAG_TRANS1_AND_SKY_BLEND_LEVEL1 = 39,
  TFRAG_DIRT_LEVEL1 = 41,
  TFRAG_ICE_LEVEL1 = 43,
  MERC_AFTER_ALPHA = 45,
  PRIS_TEX_LEVEL0 = 48,
  MERC_PRIS_LEVEL0 = 49,
  PRIS_TEX_LEVEL1 = 51,
  MERC_PRIS_LEVEL1 = 52,
  MERC_EYES_AFTER_PRIS = 54,
  MERC_AFTER_PRIS = 55,
  WATER_TEX_LEVEL0 = 57,
  MERC_WATER_LEVEL0 = 58,
  WATER_TEX_LEVEL1 = 60,
  MERC_WATER_LEVEL1 = 61,
  // ...
  PRE_SPRITE_TEX = 65,  // maybe it's just common textures?
  SPRITE = 66,
  DEBUG_DRAW_0 = 67,
  DEBUG_DRAW_1 = 68,
  MAX_BUCKETS = 69
};

struct LevelVis {
  bool valid = false;
  u8 data[2048];
};

/*!
 * The main renderer will contain a single SharedRenderState that's passed to all bucket renderers.
 * This allows bucket renders to share textures and shaders.
 */
constexpr int EYE_TEX_WIDTH = 64;
constexpr int EYE_TEX_HEIGHT = 352;
struct SharedRenderState {
  explicit SharedRenderState(std::shared_ptr<TexturePool> _texture_pool)
      : texture_pool(_texture_pool) {}
  ShaderLibrary shaders;
  std::shared_ptr<TexturePool> texture_pool;
  Loader loader;

  u32 buckets_base = 0;  // address of buckets array.
  u32 next_bucket = 0;   // address of next bucket that we haven't started rendering in buckets
  u32 default_regs_buffer = 0;  // address of the default regs chain.

  void* ee_main_memory = nullptr;
  u32 offset_of_s7;
  bool dump_playback = false;

  bool use_sky_cpu = true;
  bool use_occlusion_culling = true;
  bool render_debug = false;

  void reset();
  bool has_camera_planes = false;
  LevelVis occlusion_vis[2];
  math::Vector4f camera_planes[4];
};

/*!
 * Interface for bucket renders. Each bucket will have its own BucketRenderer.
 */
class BucketRenderer {
 public:
  BucketRenderer(const std::string& name, BucketId my_id) : m_name(name), m_my_id(my_id) {}
  virtual void render(DmaFollower& dma,
                      SharedRenderState* render_state,
                      ScopedProfilerNode& prof) = 0;
  std::string name_and_id() const;
  virtual ~BucketRenderer() = default;
  bool& enabled() { return m_enabled; }
  virtual bool empty() const { return false; }
  virtual void draw_debug_window() = 0;
  virtual void serialize(Serializer&) {}

 protected:
  std::string m_name;
  BucketId m_my_id;
  bool m_enabled = true;
};

class RenderMux : public BucketRenderer {
 public:
  RenderMux(const std::string& name,
            BucketId my_id,
            std::vector<std::unique_ptr<BucketRenderer>> renderers);
  void render(DmaFollower& dma, SharedRenderState* render_state, ScopedProfilerNode& prof) override;
  void draw_debug_window() override;
  void serialize(Serializer& ser) override;

 private:
  std::vector<std::unique_ptr<BucketRenderer>> m_renderers;
  int m_render_idx = 0;
  std::vector<std::string> m_name_strs;
  std::vector<const char*> m_name_str_ptrs;
};

/*!
 * Renderer that makes sure the bucket is empty and ignores it.
 */
class EmptyBucketRenderer : public BucketRenderer {
 public:
  EmptyBucketRenderer(const std::string& name, BucketId my_id);
  void render(DmaFollower& dma, SharedRenderState* render_state, ScopedProfilerNode& prof) override;
  bool empty() const override { return true; }
  void draw_debug_window() override {}
};

class SkipRenderer : public BucketRenderer {
 public:
  SkipRenderer(const std::string& name, BucketId my_id);
  void render(DmaFollower& dma, SharedRenderState* render_state, ScopedProfilerNode& prof) override;
  bool empty() const override { return true; }
  void draw_debug_window() override {}
};