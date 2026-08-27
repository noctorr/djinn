#include "VulkanResource.hpp"
#include "VulkanDebug.hpp"
#include "FileReader.hpp"

#include <algorithm>
#include <iterator>

VkResult Djinn::Instance::initInstance() noexcept
{
    if ( volkInitialize() != VK_SUCCESS )
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const VkApplicationInfo appInfo
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Djinn - Game Engine Editor",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "Djinn",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = vulkanAPIVersion
    };

    uint32_t instExtCount { 0u };
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&instExtCount);

    if ( !extensions )
    {
        return VK_INCOMPLETE;
    }

    const std::vector<const char*> requestedLayers { "VK_LAYER_KHRONOS_Validation" };

    const VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VKDebug::debugCallback
    };

    const VkInstanceCreateInfo instanceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &debugCreateInfo,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
        .ppEnabledLayerNames = requestedLayers.data(),
        .enabledExtensionCount = instExtCount,
        .ppEnabledExtensionNames = extensions
    };

    const VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    [[unlikely]] if ( result != VK_SUCCESS )
    {
        return result;
    } else [[likely]]
    {
        volkLoadInstance(instance);

        if ( !SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface) )
        {
            SDL_ReportError(SDL_GetError());
            return VK_ERROR_INITIALIZATION_FAILED;
        } else
        {
            pInstance = &instance;
            return result;
        }
    }
}

static bool UTL_SupportVK14               { false };
static uint32_t UTL_HighestVKAPISupport   {   0u  };

static constexpr double UTL_MEMORY_CONSTANT = 1024.0 * 1024.0 * 1024.0;

#if defined(__GNUC__) && !defined(__clang__)
[[gnu::optimize("O3")]]
#endif
#ifdef __GNUC__
[[gnu::hot]]
#endif
[[nodiscard]] uint8_t UTL_ScoreDeviceCapability(
    const VkPhysicalDeviceProperties2&       deviceProps,
    const VkPhysicalDeviceFeatures2&         deviceFeatures,
    const VkPhysicalDeviceMemoryProperties2& deviceMemoryProps
) noexcept
{
    uint8_t Score{};

    if (
        deviceProps.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ||
        deviceProps.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
    ) {
        Score += 5;
    } else if (
        deviceProps.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU
    ) {
        Score += 2;
    }

    if (
        deviceFeatures.features.robustBufferAccess
    ) {
        Score += 3;
    }

    if (
        deviceFeatures.features.multiDrawIndirect
    ) {
        Score += 3;
    }

    if (
        deviceFeatures.features.samplerAnisotropy
    ) {
        Score += 5;
    }

    double gigaBytes { 0.0 };
    for ( const VkMemoryHeap& memHeap : deviceMemoryProps.memoryProperties.memoryHeaps )
    {
        if (
            memHeap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
        ) {
            const double gB_size = memHeap.size / UTL_MEMORY_CONSTANT;
            gigaBytes = (gB_size > gigaBytes) ? gB_size : gigaBytes;
        }
    }

    if ( gigaBytes >= 8.0 && deviceProps.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU ) {
        Score += 4;
    }

    return Score;
}

[[nodiscard]] bool Djinn::PhysicalDevice::initPhysicalDevice() noexcept
{
    uint32_t physDeviceCount { 0u };
    vkEnumeratePhysicalDevices(*pInstance, &physDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physDeviceCount);
    vkEnumeratePhysicalDevices(*pInstance, &physDeviceCount, physicalDevices.data());

    if (physDeviceCount)
    {
        std::vector<uint8_t> Scores(physDeviceCount);
        for ( const VkPhysicalDevice& physDevice : physicalDevices )
        {
            VkPhysicalDeviceProperties2          deviceProps
            {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
            };
            VkPhysicalDeviceFeatures2            deviceFeatures
            {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
            };
            VkPhysicalDeviceMemoryProperties2    deviceMemProps
            {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2
            };

            vkGetPhysicalDeviceProperties2       (physDevice, &deviceProps);
            vkGetPhysicalDeviceFeatures2         (physDevice, &deviceFeatures);
            vkGetPhysicalDeviceMemoryProperties2 (physDevice, &deviceMemProps);

            Scores.push_back(UTL_ScoreDeviceCapability(
                deviceProps,
                deviceFeatures,
                deviceMemProps
            ));

            /** @deprecated
            if (
                deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
            ) {
                physicalDevice = physDevice;
                physDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            } else if (
                deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
                physDeviceType == VK_PHYSICAL_DEVICE_TYPE_CPU
            ) {
                physicalDevice = physDevice;
                physDeviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
            }
            */
        }

        #ifdef DEBUG_MODE__
        VKDebug::event("Physical device has been set.");
        #endif


        auto Index = std::distance(Scores.begin(), std::max_element(Scores.begin(), Scores.end()));

        device = physicalDevices[Index];
        pPhysicalDevice = &device;
        return true;
    } else
    {
        VKDebug::bad_event("There is zero physical devices that is supported by Vulkan on the User.");
        return false;
    }
}

[[nodiscard]] bool Djinn::Device::initDevice() noexcept
{
    float queuePriority = 1.f;
    std::vector<uint32_t> queueFamilies { gfxQueueFamIdx };

    VkDeviceQueueCreateInfo gfxQueueInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = gfxQueueFamIdx,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    VkPhysicalDeviceVulkan14Features supportedFeatures1_4
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = nullptr
    };

    VkPhysicalDeviceVulkan13Features supportedFeatures1_3
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &supportedFeatures1_4
    };

    VkPhysicalDeviceVulkan12Features supportedFeatures1_2
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &supportedFeatures1_3
    };

    VkPhysicalDeviceFeatures2 supportedFeatures
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &supportedFeatures1_2
    };

    vkGetPhysicalDeviceFeatures2(
        *pPhysicalDevice,
        &supportedFeatures
    );

    if (
        !supportedFeatures1_3.dynamicRendering || !supportedFeatures1_3.synchronization2 ||
        !supportedFeatures1_2.timelineSemaphore
    ) {
        SDL_ReportError("Physical Device doesn't meet feature's requirements.");
        return false;
    }

    VkPhysicalDeviceVulkan14Features features14
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = nullptr
    };

    VkPhysicalDeviceVulkan13Features features13
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features14,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE
    };

    VkPhysicalDeviceVulkan12Features features12
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &features13,
        .timelineSemaphore = VK_TRUE
    };

    VkPhysicalDeviceFeatures2 features
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features12
    };

    const std::vector<const char*> deviceExtensions
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const VkDeviceCreateInfo deviceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &gfxQueueInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = nullptr
    };

    if (
        vkCreateDevice(
            *pPhysicalDevice,
            &deviceCreateInfo,
            nullptr,
            &device
        ) != VK_SUCCESS
    ) {
        return false;
    }

    vkGetDeviceQueue(
        device,
        gfxQueueFamIdx,
        0,
        &gfxQueue
    );

    if (!gfxQueue)
    {
        SDL_ReportError(
            "Cannot get the graphics queue."
        );
        return false;
    } else
    {
        return true;
    }
}

[[nodiscard]] bool Djinn::Pipeline::initGFXQueue() noexcept
{
    uint32_t queueFamCount { 0u };
    vkGetPhysicalDeviceQueueFamilyProperties2(*pPhysicalDevice, &queueFamCount, nullptr);
    std::vector<VkQueueFamilyProperties2> queueFamProps(queueFamCount, { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
    vkGetPhysicalDeviceQueueFamilyProperties2(*pPhysicalDevice, &queueFamCount, queueFamProps.data());

    for (
        size_t currentFamIdx { 0uz };
        currentFamIdx < queueFamProps.size();
        currentFamIdx++
    ) {
        VkBool32 hasPresentSupport { VK_FALSE };
        vkGetPhysicalDeviceSurfaceSupportKHR(
            *pPhysicalDevice, currentFamIdx,
            surface, &hasPresentSupport
        );

        const VkQueueFamilyProperties2& props = queueFamProps[currentFamIdx];

        if ( props.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupport )
        {
            gfxQueueFamIdx = currentFamIdx;
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool Djinn::Swapchain::initSwapchain(
    uint32_t width,
    uint32_t height
) noexcept {
    swapchainWidth = width;
    swapchainHeight = height;

    VkSurfaceCapabilitiesKHR surfaceCapabilities {};
    if (
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            *pPhysicalDevice,
            surface,
            &surfaceCapabilities
        ) != VK_SUCCESS
    ) {
        SDL_ReportError(
            "Cannot get the surface capabilities."
        );
        return false;
    }

    const VkSwapchainCreateInfoKHR swapChainCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = surfaceCapabilities.minImageCount,
        .imageFormat = swapChainFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent {
            .width = swapchainWidth,
            .height = swapchainHeight
        },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };

    if (
        vkCreateSwapchainKHR(*pDevice, &swapChainCreateInfo, nullptr, &swapchain) !=
        VK_SUCCESS
    ) {
        SDL_ReportError("Unable to create swapchain.");
        return false;
    } else {
        return true;
    }
}

[[nodiscard]] inline bool Djinn::Device::initVMA() noexcept
{
    VmaVulkanFunctions vmaFuncInfo{};
    VmaAllocatorCreateInfo vmaAllocCreateInfo
    {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = *pPhysicalDevice,
        .device = *pDevice,
        .pVulkanFunctions = &vmaFuncInfo,
        .instance = *pInstance,
        .vulkanApiVersion = vulkanAPIVersion
    };

    vmaImportVulkanFunctionsFromVolk(
        &vmaAllocCreateInfo,
        &vmaFuncInfo
    );

    if (vmaCreateAllocator(&vmaAllocCreateInfo, &vkAllocator) != VK_SUCCESS)
    {
        return false;
    } else
    {
        return true;
    }
}

[[nodiscard]] bool Djinn::Pipeline::initGFXPipeline() noexcept
{
    const VkPipelineLayoutCreateInfo pipelineLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0
    };

    if (
        vkCreatePipelineLayout(
            *pDevice,
            &pipelineLayoutInfo,
            nullptr,
            &pipelineLayout
        ) != VK_SUCCESS
    ) {
        SDL_ReportError("Cannot create the pipeline layout.");
        return false;
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages
    {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShader,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = fragShader,
            .pName = "main"
        }
    };

    const VkPipelineVertexInputStateCreateInfo vertInputInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    const VkPipelineDepthStencilStateCreateInfo depthStencilInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .stencilTestEnable = VK_FALSE
    };

    const VkPipelineViewportStateCreateInfo viewportInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr
    };

    const VkPipelineRasterizationStateCreateInfo rasterInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.f
    };

    const VkPipelineMultisampleStateCreateInfo multiSampleInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    const VkPipelineColorBlendAttachmentState attachState
    {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    const VkPipelineColorBlendStateCreateInfo blendInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachState
    };

    std::vector<VkDynamicState> dynamicState
    {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };

    const VkPipelineDynamicStateCreateInfo dynamicStateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicState.size()),
        .pDynamicStates = dynamicState.data()
    };

    const VkPipelineRenderingCreateInfo renderInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapChainFormat,
        .depthAttachmentFormat = depthFormat
    };

    const VkGraphicsPipelineCreateInfo pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderInfo,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterInfo,
        .pMultisampleState = &multiSampleInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &blendInfo,
        .pDynamicState = &dynamicStateInfo,
        .layout = pipelineLayout,
        .renderPass = VK_NULL_HANDLE
    };

    if (
        vkCreateGraphicsPipelines(
            *pDevice,
            nullptr,
            1,
            &pipelineInfo,
            nullptr,
            &pipeline
        ) != VK_SUCCESS
    ) {
        SDL_ReportError("Could not create graphics pipeline.");
        return false;
    } else {
        return true;
    }
}

[[nodiscard]] bool Djinn::Pipeline::initShaders() noexcept
{
    const std::optional<std::string> sourceFrag = ftl::readFile("../Modules/glsl/default.frag");
    const std::optional<std::string> sourceVert = ftl::readFile("../Modules/glsl/default.vert");

    if (
        sourceFrag == std::nullopt ||
        sourceVert == std::nullopt
    ) {
        return false;
    }

    shaderc::Compiler compiler;
    shaderc::CompileOptions opts;

    opts.SetTargetEnvironment (shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
    opts.SetTargetSpirv       (shaderc_spirv_version_1_6);
    opts.SetOptimizationLevel (shaderc_optimization_level_performance);

    shaderc::CompilationResult fragResult = compiler.CompileGlslToSpv(
        sourceFrag.value(),
        shaderc_shader_kind::shaderc_fragment_shader,
        "default.frag",
        opts
    );

    shaderc::CompilationResult vertResult = compiler.CompileGlslToSpv(
        sourceVert.value(),
        shaderc_shader_kind::shaderc_vertex_shader,
        "defaut.vert",
        opts
    );

    if (
        fragResult.GetCompilationStatus() != shaderc_compilation_status_success ||
        vertResult.GetCompilationStatus() != shaderc_compilation_status_success
    ) {
        VKDebug::bad_event(
            "Shader compilation failed."
        );
        return false;
    }

    std::vector<uint32_t> frag { fragResult.cbegin(), fragResult.cend() };
    std::vector<uint32_t> vert { vertResult.cbegin(), vertResult.cend() };

    const VkShaderModuleCreateInfo vertCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .codeSize = vert.size() * sizeof(uint32_t),
        .pCode = vert.data()
    };

    const VkShaderModuleCreateInfo fragCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .codeSize = frag.size() * sizeof(uint32_t),
        .pCode = frag.data()
    };

    if (
        vkCreateShaderModule(
            *pDevice,
            &vertCreateInfo,
            nullptr,
            &vertShader
        ) != VK_SUCCESS ||
        vkCreateShaderModule(
            *pDevice,
            &fragCreateInfo,
            nullptr,
            &fragShader
        ) != VK_SUCCESS
    ) {
        SDL_ReportError("Error in making Shader Modules.");
        return false;
    } else {

        return true;
    }
}

[[nodiscard]] bool Djinn::Device::initSynchronisation() noexcept
{
    const VkSemaphoreTypeCreateInfo semTypeInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 3
    };

    const VkSemaphoreCreateInfo semCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semTypeInfo
    };

    if (
        vkCreateSemaphore(
            *pDevice,
            &semCreateInfo,
            nullptr,
            pSemaphore
        ) != VK_SUCCESS
    ) {
        SDL_ReportError("Could not create the timeline semaphore.");
        return false;
    }

    for ( Frame& frame : frameResources )
    {
        VkSemaphoreCreateInfo semaphoreInfo
        { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        if (
            vkCreateSemaphore(
                *pDevice,
                &semCreateInfo,
                nullptr,
                &frame.imageAccquiredSemaphore
            ) != VK_SUCCESS
        ) {
            SDL_ReportError("Could not create the per-frame image-accquire semaphore.");
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool Djinn::Device::initCommandBuffers() noexcept
{
    for ( Frame& frame : frameResources )
    {
        VkCommandPoolCreateInfo poolCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = gfxQueueFamIdx
        };

        if (
            vkCreateCommandPool(
                *pDevice,
                &poolCreateInfo,
                nullptr,
                &frame.commandPool
            ) != VK_SUCCESS
        ) {
            SDL_ReportError("Cannot create the command buffer pool.");
            return false;
        }

        VkCommandBufferAllocateInfo allocInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = frame.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        if (
            vkAllocateCommandBuffers(
                *pDevice,
                &allocInfo,
                &frame.commandBuffer
            ) != VK_SUCCESS
        ) {
            SDL_ReportError("Cannot allocate to command buffers.");
            return false;
        }
    }

    return true;
}

void Djinn::Swapchain::destroySwapchain() noexcept
{
    for ( VkImageView imgView : swapchainImageViews )
    {
        vkDestroyImageView(*pDevice, imgView, nullptr);
    }
    swapchainImageViews.clear();

    for ( VkSemaphore& semaphore : renCmpSem )
    {
        vkDestroySemaphore(*pDevice, semaphore, nullptr);
    }
    renCmpSem.clear();

    if (swapchain) {
        vkDestroySwapchainKHR(*pDevice, swapchain, nullptr);
        swapchain = nullptr;
    }

    if (depthImageView) {
        vkDestroyImageView(*pDevice, depthImageView, nullptr);
        vmaDestroyImage(vkAllocator, depthImage, depthImgAllocation);
        depthImageView = nullptr;
    }
}