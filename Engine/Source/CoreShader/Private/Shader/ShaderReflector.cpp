
#include "CoreShader.h"

#if PLATFORM_DESKTOP
#include "spirv_cross/spirv_reflect.hpp"
#include "spirv_cross/spirv_parser.hpp"
#include <spirv-tools/libspirv.hpp>
#endif

namespace CE
{   

    ShaderReflector::ShaderReflector()
    {
		
    }

    ShaderReflector::~ShaderReflector()
    {

    }


	ShaderReflector::ErrorCode ShaderReflector::ReflectSpirv(const void* byteCode, u32 byteSize, RHI::ShaderStage curStage, ShaderReflection& outReflection, const String& entryPointStr)
	{
#if PLATFORM_DESKTOP
		spirv_cross::CompilerReflection* reflection = new spirv_cross::CompilerReflection((const uint32_t*)byteCode, byteSize / 4);
		defer(&)
		{
			delete reflection;
		};

		auto resources = reflection->get_shader_resources();
		
		if (curStage == RHI::ShaderStage::Vertex)
		{
			// Fetch vertex stage inputs
			for (const spirv_cross::Resource& input : resources.stage_inputs)
			{
				String name = input.name;
				if (!name.StartsWith("in.var."))
				{
					return ERR_InvalidVertexInputNames;
				}

				String shaderSemanticName = name.GetSubstring(7);
				RHI::ShaderSemantic semantic = RHI::ShaderSemantic::Parse(shaderSemanticName);
				outReflection.vertexInputs.Add(semantic.ToString());

				if (semantic.attribute == VertexInputAttribute::Color)
				{
					outReflection.vertexInputTypes.Add(VertexAttributeDataType::UChar4);
				}
				else
				{
					auto baseType = reflection->get_type(input.type_id);
					switch (baseType.basetype)
					{
					case spirv_cross::SPIRType::Int:
						switch (baseType.vecsize)
						{
						case 1:
							outReflection.vertexInputTypes.Add(VertexAttributeDataType::Int);
							break;
						case 2:
							outReflection.vertexInputTypes.Add(VertexAttributeDataType::Int2);
							break;
						case 3:
						case 4:
							outReflection.vertexInputTypes.Add(VertexAttributeDataType::Int4);
							break;
						}
						break;
					case spirv_cross::SPIRType::UInt:
						switch (baseType.vecsize)
						{
						case 1:
							outReflection.vertexInputTypes.Add(VertexAttributeDataType::UInt);
							break;
						case 2:
							outReflection.vertexInputTypes.Add(VertexAttributeDataType::UInt2);
							break;
						case 3:
						case 4:
							outReflection.vertexInputTypes.Add(VertexAttributeDataType::UInt4);
							break;
						}
						break;
					case spirv_cross::SPIRType::Float:
						switch (baseType.vecsize)
						{
						case 1:
							outReflection.vertexInputTypes.Add(VertexAttributeDataType::Float);
							break;
						case 2:
							outReflection.vertexInputTypes.Add(VertexAttributeDataType::Float2);
							break;
						case 3:
						case 4:
							outReflection.vertexInputTypes.Add(VertexAttributeDataType::Float4);
							break;
						}
						break;
					default:
						outReflection.vertexInputTypes.Add(VertexAttributeDataType::Undefined);
						break;
					}
				}
			}
		}

    	if (curStage == RHI::ShaderStage::Compute)
    	{
    		const auto& entryPoint = reflection->get_entry_point(entryPointStr.ToStdString(), spv::ExecutionModelGLCompute);

    		outReflection.invocationSize.x = entryPoint.workgroup_size.x;
    		outReflection.invocationSize.y = entryPoint.workgroup_size.y;
    		outReflection.invocationSize.z = entryPoint.workgroup_size.z;
    	}

		std::function<void(Array<RHI::ShaderStructMember>&, spirv_cross::SPIRType, spirv_cross::TypeID)> reflectStruct = 
			[&](Array<RHI::ShaderStructMember>& outStructMembers, spirv_cross::SPIRType type, spirv_cross::TypeID baseTypeId)
			{
				int numMembers = type.member_types.size();
				for (int i = 0; i < numMembers; i++)
				{
					auto memberTypeId = type.member_types[i];
					String name = reflection->get_member_name(baseTypeId, i);
					auto memberType = reflection->get_type(memberTypeId);

					RHI::ShaderStructMember structMember{};
					structMember.name = name;

					if (memberType.basetype == spirv_cross::SPIRType::Float)
					{
						if (memberType.columns == 1)
						{
							if (memberType.vecsize == 1)
								structMember.dataType = RHI::ShaderStructMemberType::Float;
							else if (memberType.vecsize == 2)
								structMember.dataType = RHI::ShaderStructMemberType::Float2;
							else if (memberType.vecsize == 3)
								structMember.dataType = RHI::ShaderStructMemberType::Float3;
							else if (memberType.vecsize == 4)
								structMember.dataType = RHI::ShaderStructMemberType::Float4;
							else
								continue;
						}
						else if (memberType.columns == 4)
						{
							structMember.dataType = RHI::ShaderStructMemberType::Float4x4;
						}
						else
						{
							continue;
						}
					}
					else if (memberType.basetype == spirv_cross::SPIRType::UInt)
					{
						if (memberType.columns == 1 && memberType.vecsize == 1)
						{
							structMember.dataType = RHI::ShaderStructMemberType::UInt;
						}
						else
						{
							continue;
						}
					}
					else if (memberType.basetype == spirv_cross::SPIRType::Int)
					{
						if (memberType.columns == 1 && memberType.vecsize == 1)
						{
							structMember.dataType = RHI::ShaderStructMemberType::Int;
						}
						else
						{
							continue;
						}
					}
					else if (memberType.basetype == spirv_cross::SPIRType::Struct)
					{
						structMember.dataType = RHI::ShaderStructMemberType::Struct;
						reflectStruct(structMember.nestedMembers, memberType, reflection->get_type(memberTypeId).self);
					}
					else
					{
						continue;
					}

					structMember.arrayCount = 1;
					if (memberType.array.size() > 0)
						structMember.arrayCount = memberType.array[0];

					outStructMembers.Add(structMember);
				}
			};
		
		int numUniformBuffers = resources.uniform_buffers.size();
		for (int i = 0; i < numUniformBuffers; i++) // ConstantBuffer (uniform buffer)
		{
			auto uniformBuffer = resources.uniform_buffers[i];
			
			String name = reflection->get_name(uniformBuffer.id);
			auto id = uniformBuffer.id;
			String internalName = reflection->get_name(uniformBuffer.base_type_id);
			auto type = reflection->get_type(uniformBuffer.type_id);
			int count = 1;
			bool isArray = false;
			if (type.array.size() > 0)
			{
				count = type.array[0];
				isArray = true;
			}

			u32 set = reflection->get_decoration(id, spv::DecorationDescriptorSet);
			u32 binding = reflection->get_decoration(id, spv::DecorationBinding);
			
			RHI::SRGVariableDescriptor variable{};
			variable.bindingSlot = binding;
			variable.name = name;
			variable.type = RHI::ShaderResourceType::ConstantBuffer;
			variable.arrayCount = count;
			variable.shaderStages = curStage;

			if (type.basetype == spirv_cross::SPIRType::Struct)
			{
				reflectStruct(variable.structMembers, type, uniformBuffer.base_type_id);
			}

			auto& entry = outReflection.FindOrAdd((RHI::SRGType)set);
			entry.TryAdd(variable);
		}

		int numStorageBuffers = resources.storage_buffers.size();
		for (int i = 0; i < numStorageBuffers; i++) // StructuredBuffer (Storage Buffers)
		{
			auto storageBuffer = resources.storage_buffers[i];

			String name = reflection->get_name(storageBuffer.id);
			auto id = storageBuffer.id;
			String internalName = reflection->get_name(storageBuffer.base_type_id);

			auto type = reflection->get_type(storageBuffer.type_id);
			int count = 1;
			bool isArray = false;
			if (type.array.size() > 0)
			{
				count = type.array[0];
				isArray = true;
			}

			u32 set = reflection->get_decoration(id, spv::DecorationDescriptorSet);
			u32 binding = reflection->get_decoration(id, spv::DecorationBinding);

			RHI::SRGVariableDescriptor variable{};
			variable.bindingSlot = binding;
			variable.name = name;
			variable.type = RHI::ShaderResourceType::StructuredBuffer;
			variable.arrayCount = count;
			variable.shaderStages = curStage;
			
			if (type.basetype == spirv_cross::SPIRType::Struct)
			{
				reflectStruct(variable.structMembers, type, storageBuffer.base_type_id);
			}

			auto& entry = outReflection.FindOrAdd((RHI::SRGType)set);
			entry.TryAdd(variable);
		}

		int numStorageImages = resources.storage_images.size();
		for (int i = 0; i < numStorageImages; i++) // RWTexture2D, etc
		{
			auto storageImage = resources.storage_images[i];

			String name = reflection->get_name(storageImage.id);
			auto id = storageImage.id;
			String internalName = reflection->get_name(storageImage.id);

			auto type = reflection->get_type(storageImage.type_id);
			int count = 1;
			bool isArray = false;
			if (type.array.size() > 0)
			{
				count = type.array[0];
				isArray = true;
			}

			u32 set = reflection->get_decoration(id, spv::DecorationDescriptorSet);
			u32 binding = reflection->get_decoration(id, spv::DecorationBinding);
			
			RHI::SRGVariableDescriptor variable{};
			variable.bindingSlot = binding;
			variable.name = name;
			if (type.image.dim == spv::Dim2D)
			{
				variable.type = RHI::ShaderResourceType::RWTexture2D;
			}
			else if (type.image.dim == spv::Dim3D)
			{
				variable.type = RHI::ShaderResourceType::RWTexture3D;
			}
			else
			{
				continue; // Invalid type
			}
			variable.type = RHI::ShaderResourceType::RWTexture2D;
			variable.arrayCount = count;
			variable.shaderStages = curStage;

			auto& entry = outReflection.FindOrAdd((RHI::SRGType)set);
			entry.TryAdd(variable);
		}
		
		int numTextures = resources.separate_images.size();
		for (int i = 0; i < numTextures; i++) // Texure1D/2D/3D/Cube (separate texture)
		{
			auto texture = resources.separate_images[i];
			
			String name = reflection->get_name(texture.id);
			auto id = texture.id;
			String internalName = reflection->get_name(texture.base_type_id);

			auto type = reflection->get_type(texture.type_id);
			int count = 1;
			bool isArray = false;
			if (type.array.size() > 0)
			{
				count = type.array[0];
				isArray = true;
			}

			u32 set = reflection->get_decoration(id, spv::DecorationDescriptorSet);
			u32 binding = reflection->get_decoration(id, spv::DecorationBinding);

			RHI:SRGVariableDescriptor variable{};
			variable.bindingSlot = binding;
			variable.name = name;
			variable.shaderStages = curStage;
			
			if (type.image.dim == spv::Dim2D)
			{
				if (type.image.arrayed)
					variable.type = RHI::ShaderResourceType::Texture2DArray;
				else
					variable.type = RHI::ShaderResourceType::Texture2D;
			}
			else if (type.image.dim == spv::Dim1D)
			{
				variable.type = RHI::ShaderResourceType::Texture1D;
			}
			else if (type.image.dim == spv::Dim3D)
			{
				variable.type = RHI::ShaderResourceType::Texture3D;
			}
			else if (type.image.dim == spv::DimCube)
			{
				variable.type = RHI::ShaderResourceType::TextureCube;
			}
			else
			{
				continue; // Invalid type
			}
			variable.arrayCount = count;

			auto& entry = outReflection.FindOrAdd((RHI::SRGType)set);
			entry.TryAdd(variable);
		}

		int numSamplers = resources.separate_samplers.size();
		for (int i = 0; i < numSamplers; i++) // SamplerState (separate sampler)
		{
			auto sampler = resources.separate_samplers[i];

			String name = reflection->get_name(sampler.id);
			auto id = sampler.id;
			String internalName = reflection->get_name(sampler.base_type_id);

			auto type = reflection->get_type(sampler.type_id);
			int count = 1;
			bool isArray = false;
			if (type.array.size() > 0)
			{
				count = type.array[0];
				isArray = true;
			}

			u32 set = reflection->get_decoration(id, spv::DecorationDescriptorSet);
			u32 binding = reflection->get_decoration(id, spv::DecorationBinding);

			RHI::SRGVariableDescriptor variable{};
			variable.bindingSlot = binding;
			variable.name = name;
			variable.type = RHI::ShaderResourceType::SamplerState;
			variable.arrayCount = count;
			variable.shaderStages = curStage;

			auto& entry = outReflection.FindOrAdd((RHI::SRGType)set);
			entry.TryAdd(variable);
		}
		return ERR_Success;
#else
		return ERR_UnsupportedPlatform;
#endif
	}

} // namespace CE

