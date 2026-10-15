#pragma once
#include <volk/volk.h>
#include <vulkan/vulkan.h>
#include <string>
#include <iostream>
#include <SDL3/SDL.h>
#include <fstream>
#include <sstream>
#include <slang/slang-com-ptr.h>

static inline void chk(VkResult result) {
	if (result != VK_SUCCESS) {
		std::cerr << "Vulkan call returned an error (" << result << ")\n";
		exit(result);
	}
}

static inline void chk(bool result) {
	if (!result) {
		std::cerr << "Call returned an error\n";
		std::cout << SDL_GetError() << std::endl;
		exit(result);
	}
}

std::string readUpShader(const std::string& filePath) {
	std::string shader;

	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Error: cannot open file" << std::endl;
		return "Error: cannot open file";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	shader = buffer.str();

	std::cout << shader << std::endl;
	return shader;
}

VkShaderModule compileSlangToShaderModule(VkDevice device, const std::string& shaderCode, const char* entryPointName) {
	Slang::ComPtr<slang::IGlobalSession> globalSession;
	slang::createGlobalSession(globalSession.writeRef());

	slang::TargetDesc targetDesc = {};
	targetDesc.format = SLANG_SPIRV;
	targetDesc.profile = globalSession->findProfile("spirv");

	slang::SessionDesc sessionDesc = {};
	sessionDesc.targets = &targetDesc;
	sessionDesc.targetCount = 1;

	Slang::ComPtr<slang::ISession> session;
	globalSession->createSession(sessionDesc, session.writeRef());

	Slang::ComPtr<slang::IBlob> diagnostics;
	slang::IModule* module = session->loadModuleFromSourceString("shader", nullptr, shaderCode.c_str(), diagnostics.writeRef());


	if (diagnostics) {
		std::cerr << "Slang Error: " << (char*) diagnostics->getBufferPointer() << std::endl;
	}

	if (!module) {
		return VK_NULL_HANDLE;
	}
	
	Slang::ComPtr<slang::IEntryPoint> entryPoint;
	module->findEntryPointByName(entryPointName, entryPoint.writeRef());
	
	slang::IComponentType* components[] = { module, entryPoint };
	Slang::ComPtr<slang::IComponentType> linkedProgram;
	session->createCompositeComponentType(components, 2, linkedProgram.writeRef(), diagnostics.writeRef());


	Slang::ComPtr<slang::IBlob> spirvBlob;
	

	linkedProgram->getEntryPointCode(0, 0, spirvBlob.writeRef(), diagnostics.writeRef());

	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = spirvBlob->getBufferSize(),
		.pCode = reinterpret_cast<const uint32_t*> (spirvBlob->getBufferPointer())
	};

	VkShaderModule shaderModule;
	chk(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
	return shaderModule;
}
