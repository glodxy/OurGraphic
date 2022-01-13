//
// Created by Glodxy on 2022/1/13.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_PASSNODE_H_
#define OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_PASSNODE_H_
#include "Renderer/RenderGraph/Base/DependencyGraph.h"
namespace our_graph::render_graph {
class RenderGraph;

class PassNode : public  DependencyGraph::Node {
 protected:
  RenderGraph& render_graph_;
};
}
#endif //OUR_GRAPHIC_FRAMEWORK_RENDERER_RENDERGRAPH_BASE_PASSNODE_H_
