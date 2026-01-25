#pragma once

namespace CE::RPI
{

	class CORERPI_API MaterialSystem final
	{
		CE_NO_COPY(MaterialSystem)
	public:

		static MaterialSystem& Get();

		void Update(u32 imageIndex);

		auto& GetMaterialsMutex() { return materialsMutex; }

	private:

		MaterialSystem() = default;

		~MaterialSystem() = default;

		void AddMaterial(RPI::Material* material);
		void RemoveMaterial(RPI::Material* material);

		SharedMutex materialsMutex{};
		List<RPI::Material*> materials{};

		friend class Material;
	};

} // namespace CE::RPI
