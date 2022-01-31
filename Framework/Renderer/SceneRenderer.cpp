//
// Created by Home on 2022/1/30.
//

#include "SceneRenderer.h"

namespace our_graph {

SceneRenderer::SceneRenderer(const SceneViewFamily *input, Driver *driver) : IRenderer(driver),
                                                                             allocator_(driver),
                                                                             render_graph_(allocator_) {

}

void SceneRenderer::GC() {}



}  // namespace our_graph