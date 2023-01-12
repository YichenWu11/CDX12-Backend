#pragma once

namespace Chen::CDX12 {
	template<typename T>
	T& FrameResource::GetResource(std::string_view name) {
		static_assert(!std::is_reference_v<T>);
		assert(HaveResource(name));
		return std::any_cast<T&>(resourceMap.find(name)->second);
	}

	template<typename T>
	const T& FrameResource::GetResource(std::string_view name) const {
		return const_cast<FrameResource*>(this)->GetResource<T>();
	}

	template<typename T>
	FrameResource& FrameResource::RegisterResource(std::string name, T&& resource) {
		assert(!HaveResource(name));
		resourceMap.emplace(std::move(name), std::any{ std::forward<T>(resource) });
		return *this;
	}
}