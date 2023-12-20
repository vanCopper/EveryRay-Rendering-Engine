#pragma once
#include <memory>
#include <typeindex>
#include <type_traits>

namespace EveryRay_Core
{
    class RenderGraphBlackboard
    {
    public:
        RenderGraphBlackboard() = default;
        ~RenderGraphBlackboard() = default;
        RenderGraphBlackboard(const RenderGraphBlackboard&) = delete;
        RenderGraphBlackboard& operator=(const RenderGraphBlackboard&) = delete;

        template<typename T>
        T& Add(T&& data)
        {
            return Create<std::remove_cvref_t<T>>(std::forward<T>(data));
        }

        template<typename T, typename... Args>
        typename std::enable_if<std::is_constructible<T, Args...>::value, T>::type
        Create(Args&&... args)
        {
            static_assert(std::is_trivial_v<T> && std::is_standard_layout_v<T>);
            board_data[typeid(T)] = std::make_unique<uint8_t[]>(sizeof(T));
            T* data_entry = reinterpret_cast<T*>(board_data[typeid(T)].get());
            *data_entry = T{ std::forward<Args>(args)... };
            return *data_entry;
        }

        template<typename T>
        T const* TryGet() const
        {
            auto it = board_data.find(typeid(T));
            if (it != board_data.end())
            {
                return reinterpret_cast<T const*>(it->second.get());
            }else
            {
                return nullptr;
            }
        }

        template<typename T>
        T const& Get() const
        {
            T const* p_data = TryGet<T>();
            ADRIA_ASSERT(p_data != nullptr);
            return *p_data;
        }

        void Clear()
        {
            board_data.clear();
        }
    private:
        std::unordered_map<std::type_index, std::unique_ptr<uint8_t[]>> board_data;
    };

    using RGBlackboard = RenderGraphBlackboard;
}
