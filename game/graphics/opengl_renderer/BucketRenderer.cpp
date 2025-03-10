#include "BucketRenderer.h"
#include "third-party/imgui/imgui.h"

#include "third-party/fmt/core.h"

std::string BucketRenderer::name_and_id() const {
  return fmt::format("[{:2d}] {}", (int)m_my_id, m_name);
}

EmptyBucketRenderer::EmptyBucketRenderer(const std::string& name, BucketId my_id)
    : BucketRenderer(name, my_id) {}

void EmptyBucketRenderer::render(DmaFollower& dma,
                                 SharedRenderState* render_state,
                                 ScopedProfilerNode& /*prof*/) {
  // an empty bucket should have 4 things:
  // a NEXT in the bucket buffer
  // a CALL that calls the default register buffer chain
  // a CNT then RET to get out of the default register buffer chain
  // a NEXT to get to the next bucket.

  // NEXT
  auto first_tag = dma.current_tag();
  dma.read_and_advance();
  ASSERT(first_tag.kind == DmaTag::Kind::NEXT && first_tag.qwc == 0);

  // CALL
  auto call_tag = dma.current_tag();
  dma.read_and_advance();
  if (!(call_tag.kind == DmaTag::Kind::CALL && call_tag.qwc == 0)) {
    fmt::print("Bucket renderer {} ({}) was supposed to be empty, but wasn't\n", m_my_id, m_name);
  }
  ASSERT(call_tag.kind == DmaTag::Kind::CALL && call_tag.qwc == 0);

  // in the default reg buffer:
  ASSERT(dma.current_tag_offset() == render_state->default_regs_buffer);
  dma.read_and_advance();
  ASSERT(dma.current_tag().kind == DmaTag::Kind::RET);
  dma.read_and_advance();

  // NEXT to next buffer
  auto to_next_buffer = dma.current_tag();
  ASSERT(to_next_buffer.kind == DmaTag::Kind::NEXT);
  ASSERT(to_next_buffer.qwc == 0);
  dma.read_and_advance();

  // and we should now be in the next bucket!
  ASSERT(dma.current_tag_offset() == render_state->next_bucket);
}

SkipRenderer::SkipRenderer(const std::string& name, BucketId my_id) : BucketRenderer(name, my_id) {}

void SkipRenderer::render(DmaFollower& dma,
                          SharedRenderState* render_state,
                          ScopedProfilerNode& /*prof*/) {
  while (dma.current_tag_offset() != render_state->next_bucket) {
    dma.read_and_advance();
  }
}

void SharedRenderState::reset() {
  has_camera_planes = false;
  for (auto& x : occlusion_vis) {
    x.valid = false;
  }
}

RenderMux::RenderMux(const std::string& name,
                     BucketId my_id,
                     std::vector<std::unique_ptr<BucketRenderer>> renderers)
    : BucketRenderer(name, my_id), m_renderers(std::move(renderers)) {
  for (auto& r : m_renderers) {
    m_name_strs.push_back(r->name_and_id());
    m_name_str_ptrs.push_back(m_name_strs.back().data());
  }
}

void RenderMux::render(DmaFollower& dma,
                       SharedRenderState* render_state,
                       ScopedProfilerNode& prof) {
  m_renderers[m_render_idx]->render(dma, render_state, prof);
}

void RenderMux::serialize(Serializer& ser) {
  for (auto& r : m_renderers) {
    r->serialize(ser);
  }
}

void RenderMux::draw_debug_window() {
  ImGui::ListBox("Pick", &m_render_idx, m_name_str_ptrs.data(), m_renderers.size());
  ImGui::Separator();
  m_renderers[m_render_idx]->draw_debug_window();
}