#pragma once

#include "callback.hpp"

namespace uvpp 
{
    namespace
    {
        inline void free_handle(uv_handle_t* h)
        {
            assert(h);

            if(h->data)
            {
                delete reinterpret_cast<callbacks*>(h->data);
                h->data = nullptr;
            }

            switch(h->type)
            {
                case UV_TCP: 
                    delete reinterpret_cast<uv_tcp_t*>(h);
                    break;

                default:
                    assert(0);
                    throw std::runtime_error("free_handle can't handle this type");
                    break;
            }
        }

    }

    class handle
    {
    public:
        template<typename T>
        handle():
            m_uv_handle(reinterpret_cast<uv_handle_t*>(new T()))
        {
            assert(m_uv_handle);
            m_uv_handle->data = new callbacks();
            assert(m_uv_handle->data);
        }

    public:
        template<typename T=uv_handle_t>
        T* get()
        { 
            return reinterpret_cast<T*>(m_uv_handle); 
        }

        template<typename T=uv_handle_t>
        const T* get() const
        {
            return reinterpret_cast<const T*>(m_uv_handle);
        }

        bool is_active()
        { 
            return uv_is_active(get()) != 0;
        }

        void close(std::function<void()> callback = []{})
        {
            callbacks::store(get()->data, internal::uv_cid_close, callback);
            uv_close(get(),
                [](uv_handle_t* h) {
                    callbacks::invoke<decltype(callback)>(h->data, internal::uv_cid_close);
                    free_handle(h);
                });
        }

    protected:
        uv_handle_t* m_uv_handle;
    };

}

