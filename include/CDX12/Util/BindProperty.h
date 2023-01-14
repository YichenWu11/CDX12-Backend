#pragma once

#include <string>
#include <variant>

#include <CDX12/Resource/BufferView.h>
#include <CDX12/Resource/DescHeapAllocView.h>

namespace Chen::CDX12 {
    struct BindProperty {
        std::string name;
        std::variant<
            BufferView,
            DescriptorHeapAllocView>
            prop;

        BindProperty() {}

        template <typename A, typename B>
        requires(
            std::is_constructible_v<decltype(name), A&&> || std::is_constructible_v<decltype(prop), B&&>)
            BindProperty(
                A&& a,
                B&& b) :
            name(std::forward<A>(a)),
            prop(std::forward<B>(b)) {}
    };
} // namespace Chen::CDX12
