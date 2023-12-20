#pragma once

#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "RenderGraphBlackboard.h"
#include "RenderGraphResourceId.h"
#include "RenderGraphResourceName.h"
#include "RenderGraphResourcePool.h"
#include "RenderGraphBuilder.h"
#include "../RHI/DX12/ER_RHI_DX12_GPUDescriptorHeapManager.h"

namespace EveryRay_Core
{
    class ER_RHI_DX12_DescriptorHandle;
    enum class ER_RHI_RESOURCE_STATE;
    class RenderGraphPassBase;
    class ER_RHI;
    
    class RenderGraph
    {
        friend class RenderGraphBuilder;
        friend class RenderGraphContext;

        class DependencyLevel
        {
            friend RenderGraph;
        public:
            explicit DependencyLevel(RenderGraph& renderGraph) : rg(renderGraph) {}

            void AddPass(RenderGraphPassBase* pass);
            void Setup();
            void Execute(ER_RHI* rhi);
            
        private:
            RenderGraph& rg;
            std::vector<RenderGraphPassBase*> passes;
            
            std::unordered_set<RGTextureId> texture_creates;
            std::unordered_set<RGTextureId> texture_reads;
            std::unordered_set<RGTextureId> texture_writes;
            std::unordered_set<RGTextureId> texture_destroys;
            std::unordered_map<RGTextureId, ER_RHI_RESOURCE_STATE> texture_state_map;

            std::unordered_set<RGBufferId> buffer_creates;
            std::unordered_set<RGBufferId> buffer_reads;
            std::unordered_set<RGBufferId> buffer_writes;
            std::unordered_set<RGBufferId> buffer_destroys;
            std::unordered_map<RGBufferId, ER_RHI_RESOURCE_STATE> buffer_state_map;
        };
        
    public:
        RenderGraph(RGResourcePool& pool) : pool(pool), rhi(pool.GetRHI()) {}
        ~RenderGraph();

        void Build();
        void Execute();

        template<typename PassData, typename... Args, typename std::enable_if<std::is_constructible<RenderGraphPass<PassData>, Args...>::value, int>::type = 0>
        auto AddPass(Args&&... args) -> RenderGraphPass<PassData>&
        {
            passes.emplace_back(std::make_unique<RenderGraphPass<PassData>>(std::forward<Args>(args)...));
            std::unique_ptr<RenderGraphPassBase>& pass = passes.back();
            pass->id = passes.size() - 1;
            RenderGraphBuilder builder(*this, *pass);
            pass->Setup(builder);
            return *static_cast<RenderGraphPass<PassData>*>(pass.get());
        }

        void ImportTexture(RGResourceName name, ER_RHI_GPUTexture* texture);
        void ImportBuffer(RGResourceName name, ER_RHI_GPUBuffer* buffer);

        void ExportTexture(RGResourceName name, ER_RHI_GPUTexture* texture);
        void ExportBuffer(RGResourceName name, ER_RHI_GPUBuffer* buffer);

        // void DumpRenderGraph(char const* graph_file_name);
        // void DumpDebugData();
        
    private:
        RGResourcePool pool;
        ER_RHI* rhi;
        RGBlackboard blackboard;

        std::vector<std::unique_ptr<RGPassBase>> passes;
        std::vector<std::unique_ptr<ER_RHI_GPUTexture>> textures;
        std::vector<std::unique_ptr<ER_RHI_GPUBuffer>> buffers;

        std::vector<std::vector<uint64_t>> adjacency_lists;
        std::vector<uint64_t> topologically_sorted_passes;
        std::vector<DependencyLevel> dependency_levels;

        std::unordered_map<RGResourceName, RGTextureId> texture_name_id_map;
        std::unordered_map<RGResourceName, RGBufferId> buffer_name_id_map;
        std::unordered_map<RGBufferReadWriteId, RGBufferId> buffer_uav_counter_map;

        mutable std::unordered_map<RGTextureId, std::vector<std::pair<RGTextureDescriptorDesc, RGDescriptorType>>> texture_view_desc_map;
        mutable std::unordered_map<RGTextureId, std::vector<std::pair<ER_RHI_DX12_DescriptorHandle, RGDescriptorType>>> texture_view_map;

        mutable std::unordered_map<RGBufferId, std::vector<std::pair<RGBufferDescriptorDesc, RGDescriptorType>>> buffer_view_desc_map;
        mutable std::unordered_map<RGBufferId, std::vector<std::pair<ER_RHI_DX12_DescriptorHandle, RGDescriptorType>>> buffer_view_map;

        void BuildAdjacencyLists();
        void TopologicalSort();
        void BuildDependencyLevels();
        void CullPasses();
        void CalculateResourceLifetime();
        void DepthFirstSearch(uint64_t i, std::vector<bool>& visited, std::stack<uint64_t>& stack);

        // RGTextureId DeclareTexture(RGResourceName name, RGTextureDesc const& desc);
    };
}
