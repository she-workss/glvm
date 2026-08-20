#include "glvm/graphic_api/imgui_overlay.hpp"

#include "glvm/archetype_ecs/arch_ecs_world.hpp"
#include "glvm/globals.hpp"
#include "glvm/graphic_api/vulkan.hpp"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace glvm::core {
namespace {
// 16k verts, shared line + quad buffer.
constexpr uint32_t kMaxDebugVertices = 1 << 14;

void pushLine(
    std::vector<DebugVertex>& out,
    const Vector<float, 3>& a,
    const Vector<float, 3>& b,
    const Vector<float, 3>& color
) {
    out.push_back({a[0], a[1], a[2], color[0], color[1], color[2]});
    out.push_back({b[0], b[1], b[2], color[0], color[1], color[2]});
}

void pushBox(
    std::vector<DebugVertex>& out,
    const Vector<float, 3> corners[8],
    const Vector<float, 3>& color
) {
    static const unsigned int edges[12][2] = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0},
        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}
    };
    for (const auto& edge : edges) {
        pushLine(out, corners[edge[0]], corners[edge[1]], color);
    }
}

Vector<float, 4> toVec4(const Vector<float, 3>& v, float w) {
    return Vector<float, 4>(v[0], v[1], v[2], w);
}

Vector<float, 3> fromVec4(const Vector<float, 4>& v) {
    return Vector<float, 3>(v[0], v[1], v[2]);
}
} // namespace

ImGuiOverlay::ImGuiOverlay(CVulkanRenderer& renderer) : renderer_(renderer) {}

bool ImGuiOverlay::wantsMouse() const {
    if (!initialized_) {
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

void ImGuiOverlay::init() {
    if (initialized_) {
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    createRenderPass();
    createVertexBuffer();
    createLinePipeline();

    ImGui_ImplWin32_Init(renderer_.Window->GetModernWindowHWND());
    ImGui_ImplVulkan_InitInfo initInfo {};
    initInfo.ApiVersion = VK_API_VERSION_1_0;
    initInfo.Instance = renderer_.instance;
    initInfo.PhysicalDevice = renderer_.physicalDevice;
    initInfo.Device = renderer_.device;
    initInfo.QueueFamily = renderer_.findQueueFamilies(renderer_.physicalDevice)
                               .graphicsFamily.value();
    initInfo.Queue = renderer_.graphicsQueue;
    initInfo.DescriptorPoolSize = 512;
    initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.ImageCount =
        static_cast<uint32_t>(renderer_.swapChainImages.size());
    initInfo.PipelineInfoMain.RenderPass = renderPass_;
    initInfo.PipelineInfoMain.Subpass = 0;

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        throw std::runtime_error("failed to init ImGui vulkan backend!");
    }

    initialized_ = true;
    createSwapChainResources();
}

void ImGuiOverlay::shutdown() {
    if (!initialized_) {
        return;
    }
    vkDeviceWaitIdle(renderer_.device);
    destroySwapChainResources();
    vkDestroyBuffer(renderer_.device, vertexBuffer_, nullptr);
    vkFreeMemory(renderer_.device, vertexBufferMemory_, nullptr);
    vertexBuffer_ = VK_NULL_HANDLE;
    vertexBufferMapped_ = nullptr;

    vkDestroyPipeline(renderer_.device, linePipeline_, nullptr);
    vkDestroyPipelineLayout(renderer_.device, lineLayout_, nullptr);
    vkDestroyRenderPass(renderer_.device, renderPass_, nullptr);
    renderPass_ = VK_NULL_HANDLE;

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
}

void ImGuiOverlay::createSwapChainResources() {
    if (!initialized_) {
        return;
    }
    framebuffers_.resize(renderer_.swapChainImageViews.size());
    for (size_t i = 0; i < framebuffers_.size(); ++i) {
        VkImageView attachment = renderer_.swapChainImageViews[i];
        VkFramebufferCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = renderPass_;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.width = renderer_.swapChainExtent.width;
        info.height = renderer_.swapChainExtent.height;
        info.layers = 1;
        if (vkCreateFramebuffer(
                renderer_.device,
                &info,
                nullptr,
                &framebuffers_[i]
            )
            != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create imgui overlay framebuffer!"
            );
        }
    }
}

void ImGuiOverlay::destroySwapChainResources() {
    for (VkFramebuffer& framebuffer : framebuffers_) {
        vkDestroyFramebuffer(renderer_.device, framebuffer, nullptr);
    }
    framebuffers_.clear();
}

void ImGuiOverlay::newFrame() {
    if (!initialized_) {
        return;
    }
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();

    if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
        showPanel = !showPanel;
    }

    buildPanel();
    buildDebugVertices();

    ImGui::Render();
}

void ImGuiOverlay::recordCommandBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex
) {
    if (!initialized_) {
        return;
    }

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = framebuffers_[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = renderer_.swapChainExtent;
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = nullptr;

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderer_.swapChainExtent.width);
    viewport.height = static_cast<float>(renderer_.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = renderer_.swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    if (lineVertexCount_ > 0) {
        vkCmdBindPipeline(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            linePipeline_
        );
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer_, &offset);
        Matrix<float, 4> viewProj =
            renderer_.viewMatrix * renderer_.projectionMatrix;
        vkCmdPushConstants(
            commandBuffer,
            lineLayout_,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(Matrix<float, 4>),
            &viewProj
        );
        vkCmdDraw(commandBuffer, lineVertexCount_, 1, 0, 0);
    }

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);
}

void ImGuiOverlay::buildPanel() {
    if (!showPanel) {
        return;
    }
    ImGui::Begin("Debug overlay", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Checkbox("Actor bounds", &showActorBounds);
    ImGui::Checkbox("Light frustums", &showLightFrustums);
    ImGui::Checkbox("Spatial grid", &showSpatialGrid);
    ImGui::Checkbox("Shadows", &shadowsEnabled);
    ImGui::Checkbox("Shadow maps", &showShadowMaps);
    if (showShadowMaps) {
        ImGui::RadioButton("Directional", &shadowMapMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Spot", &shadowMapMode, 1);
        int maxLight = (shadowMapMode == 0)
            ? static_cast<int>(renderer_.directionalLightNumber)
            : static_cast<int>(renderer_.spotLightNumber);
        ImGui::SliderInt("Light", &shadowMapLight, 0, std::max(0, maxLight - 1));
        ImGui::Text(
            "Shadow map #%d of %d",
            shadowMapLight,
            std::max(1, maxLight)
        );
    }
    ImGui::Text("Actors: %u", renderer_.actors.size());
    ImGui::Text(
        "Dir lights: %u, Spot lights: %u",
        renderer_.directionalLightNumber,
        renderer_.spotLightNumber
    );
    ImGui::Separator();
    if (ImGui::Button("Hide panel (F1)")) {
        showPanel = false;
    }
    ImGui::End();
}

void ImGuiOverlay::buildDebugVertices() {
    std::vector<DebugVertex> vertices;
    vertices.reserve(kMaxDebugVertices);
    lineVertexCount_ = 0;

    if (showActorBounds) {
        const Vector<float, 3> green = {0.0f, 1.0f, 0.0f};
        const Vector<float, 3> red = {1.0f, 0.0f, 0.0f};
        std::vector<Vector<float, 3>> mins;
        std::vector<Vector<float, 3>> maxs;
        mins.reserve(renderer_.actors.size());
        maxs.reserve(renderer_.actors.size());
        for (size_t i = 0; i < renderer_.actors.size(); ++i) {
            RenderActor actor = renderer_.actors[i];
            if (actor.meshID >= allMeshMaxAbsoluteValues.size()) {
                mins.push_back({0, 0, 0});
                maxs.push_back({0, 0, 0});
                continue;
            }
            const MeshAxisMaxAbsoluteValues& bounds =
                allMeshMaxAbsoluteValues[actor.meshID];
            const Vector<float, 3> center = {
                bounds.origin_offset_x,
                bounds.origin_offset_y,
                bounds.origin_offset_z
            };
            const Vector<float, 3> half =
                {bounds.absolute_x, bounds.absolute_y, bounds.absolute_z};
            Vector<float, 3> localCorners[8] = {
                center + Vector<float, 3>(-half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], half[1], half[2]),
                center + Vector<float, 3>(-half[0], half[1], half[2])
            };
            Vector<float, 3> mn =
                fromVec4(toVec4(localCorners[0], 1.0f) * actor.modelMatrix);
            Vector<float, 3> mx = mn;
            for (int c = 1; c < 8; ++c) {
                Vector<float, 3> w =
                    fromVec4(toVec4(localCorners[c], 1.0f) * actor.modelMatrix);
                for (int a = 0; a < 3; ++a) {
                    mn[a] = std::min(mn[a], w[a]);
                    mx[a] = std::max(mx[a], w[a]);
                }
            }
            mins.push_back(mn);
            maxs.push_back(mx);
        }
        std::vector<bool> collides(renderer_.actors.size(), false);
        for (size_t i = 0; i < renderer_.actors.size(); ++i) {
            for (size_t j = i + 1; j < renderer_.actors.size(); ++j) {
                // Non-strict: the collision system resolves contact by pushing
                // the mover back to exactly touch the target, so overlapping
                // boxes (<) alone misses face-to-face contact.
                if (mins[i][0] <= maxs[j][0] && maxs[i][0] >= mins[j][0]
                    && mins[i][1] <= maxs[j][1] && maxs[i][1] >= mins[j][1]
                    && mins[i][2] <= maxs[j][2] && maxs[i][2] >= mins[j][2]) {
                    collides[i] = true;
                    collides[j] = true;
                }
            }
        }
        for (size_t i = 0; i < renderer_.actors.size(); ++i) {
            if (renderer_.actors[i].meshID >= allMeshMaxAbsoluteValues.size()) {
                continue;
            }
            const MeshAxisMaxAbsoluteValues& bounds =
                allMeshMaxAbsoluteValues[renderer_.actors[i].meshID];
            const Vector<float, 3> center = {
                bounds.origin_offset_x,
                bounds.origin_offset_y,
                bounds.origin_offset_z
            };
            const Vector<float, 3> half =
                {bounds.absolute_x, bounds.absolute_y, bounds.absolute_z};
            Vector<float, 3> localCorners[8] = {
                center + Vector<float, 3>(-half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], half[1], half[2]),
                center + Vector<float, 3>(-half[0], half[1], half[2])
            };
            Vector<float, 3> worldCorners[8];
            for (int c = 0; c < 8; ++c) {
                worldCorners[c] = fromVec4(
                    toVec4(localCorners[c], 1.0f)
                    * renderer_.actors[i].modelMatrix
                );
            }
            pushBox(vertices, worldCorners, collides[i] ? red : green);
        }
    }

    if (showLightFrustums) {
        const Vector<float, 3> yellow = {1.0f, 1.0f, 0.0f};
        for (uint32_t i = 0; i < renderer_.directionalLightNumber; ++i) {
            Vector<float, 3> corners[8];
            Matrix<float, 4> inverseLight =
                inverse_matrix_4x4(renderer_.dirLightSpaceMatrix[i]);
            for (int c = 0; c < 8; ++c) {
                const float s = (c & 4) ? 1.0f : -1.0f; // z (near/far).
                const float u = (c & 2) ? 1.0f : -1.0f; // y.
                const float v = (c & 1) ? 1.0f : -1.0f; // x.
                corners[c] =
                    fromVec4(Vector<float, 4>(u, v, s, 1.0f) * inverseLight);
            }
            pushBox(vertices, corners, yellow);
        }

        const Vector<float, 3> cyan = {0.0f, 1.0f, 1.0f};
        for (uint32_t i = 0; i < renderer_.spotLightNumber; ++i) {
            Vector<float, 3> corners[8];
            Matrix<float, 4> inverseLight =
                inverse_matrix_4x4(renderer_.spotLightSpaceMatrix[i]);
            for (int c = 0; c < 8; ++c) {
                const float s = (c & 4) ? 1.0f : -1.0f;
                const float u = (c & 2) ? 1.0f : -1.0f;
                const float v = (c & 1) ? 1.0f : -1.0f;
                corners[c] =
                    fromVec4(Vector<float, 4>(u, v, s, 1.0f) * inverseLight);
            }
            pushBox(vertices, corners, cyan);
        }
    }

    if (showSpatialGrid) {
        const auto& grid = glvm::ecs::arch::world.spatialGrid;
        const float halfChunk = grid.grid[0][0][0].size * 0.5f;
        const float cross = 1.5f;
        for (uint32_t z = 0; z < grid.depth; ++z) {
            for (uint32_t y = 0; y < grid.height; ++y) {
                for (uint32_t x = 0; x < grid.width; ++x) {
                    const auto& chunk = grid.grid[z][y][x];
                    const size_t count = chunk.entities.size();
                    if (count == 0) {
                        continue;
                    }
                    // Only occupied cells, colored by entity count.
                    const Vector<float, 3> color = count == 1
                        ? Vector<float, 3>(0.0f, 1.0f, 0.0f)
                        : count <= 3 ? Vector<float, 3>(1.0f, 1.0f, 0.0f)
                                     : Vector<float, 3>(1.0f, 0.0f, 0.0f);
                    const Vector<float, 3> center = chunk.position
                        + Vector<float, 3>(halfChunk, halfChunk, halfChunk);
                    pushLine(
                        vertices,
                        center - Vector<float, 3>(cross, 0, 0),
                        center + Vector<float, 3>(cross, 0, 0),
                        color
                    );
                    pushLine(
                        vertices,
                        center - Vector<float, 3>(0, cross, 0),
                        center + Vector<float, 3>(0, cross, 0),
                        color
                    );
                    pushLine(
                        vertices,
                        center - Vector<float, 3>(0, 0, cross),
                        center + Vector<float, 3>(0, 0, cross),
                        color
                    );
                }
            }
        }
    }

    lineVertexCount_ = static_cast<uint32_t>(vertices.size());

    if (!vertices.empty() && vertexBufferMapped_) {
        const size_t bytes = vertices.size() * sizeof(DebugVertex);
        const size_t capacity = kMaxDebugVertices * sizeof(DebugVertex);
        memcpy(vertexBufferMapped_, vertices.data(), std::min(bytes, capacity));
    }
}

void ImGuiOverlay::createRenderPass() {
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = renderer_.swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorReference {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;

    VkSubpassDependency dependencies[2] {};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = 0;

    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies;

    if (vkCreateRenderPass(
            renderer_.device,
            &renderPassInfo,
            nullptr,
            &renderPass_
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create imgui overlay render pass!");
    }
}

void ImGuiOverlay::createLinePipeline() {
    auto createShaderModule = [&](const char* path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error(
                std::string("failed to open shader: ") + path
            );
        }
        file.seekg(0, std::ios::beg);
        std::vector<char> code(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        VkShaderModuleCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule module;
        if (vkCreateShaderModule(renderer_.device, &createInfo, nullptr, &module)
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return module;
    };

    VkShaderModule vert =
        createShaderModule("../../../assets/shaders/debug/debug_vert.spv");
    VkShaderModule frag =
        createShaderModule("../../../assets/shaders/debug/debug_frag.spv");

    VkPipelineShaderStageCreateInfo shaderStages[2] {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vert;
    shaderStages[0].pName = "main";
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = frag;
    shaderStages[1].pName = "main";

    VkVertexInputBindingDescription bindingDescription {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(DebugVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(DebugVertex, x);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(DebugVertex, r);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(Matrix<float, 4>);

    VkPipelineLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pSetLayouts = nullptr;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(
            renderer_.device,
            &layoutInfo,
            nullptr,
            &lineLayout_
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create debug line pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = lineLayout_;
    pipelineInfo.renderPass = renderPass_;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(
            renderer_.device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &linePipeline_
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create debug line pipeline!");
    }

    vkDestroyShaderModule(renderer_.device, vert, nullptr);
    vkDestroyShaderModule(renderer_.device, frag, nullptr);
}

void ImGuiOverlay::createVertexBuffer() {
    const VkDeviceSize bufferSize = kMaxDebugVertices * sizeof(DebugVertex);
    renderer_.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer_,
        vertexBufferMemory_
    );
    vkMapMemory(
        renderer_.device,
        vertexBufferMemory_,
        0,
        bufferSize,
        0,
        &vertexBufferMapped_
    );
}

} // namespace glvm::core
