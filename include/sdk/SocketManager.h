#pragma once
#include <list>
#include <cstdint>
#include <string_view>
#include <charconv>

#include "base/Log.h"
#include "ISocketAddress.h"
#include "UDPSocket.h"
#include "ISocketManager.h"
#include "IUDPSocketCreator.h"

namespace fb
{
    class SocketManager final : public ISocketManager, public IUDPSocketCreator
    {
    public:
        inline SocketManager()
        {
            GG_LOG(GG::LogLevel::Debug, "Created new SocketManager");
        }

        __forceinline __int64 getArena() { return 0; }

        inline void Destroy() override
        {
            GG_LOG(GG::LogLevel::Debug, "Destroyed SocketManager");
            CloseSockets();
            delete this;
        }

        inline void Close(UDPSocket *socket) override
        {
            GG_LOG(GG::LogLevel::Debug, "Closing socket");
            m_sockets.remove(socket);
        }

        inline UDPSocket *Connect(const char *address, bool blocking = false) override
        {
            (void)address;
            (void)blocking;
            return nullptr;
        }

        inline UDPSocket *Listen(const char *name, bool blocking = false) override
        {
            std::string_view ip{};
            std::uint16_t port = 0;

            if (!parse_ipv4_and_port(name, ip, port))
            {
                GG_LOG(GG::LogLevel::Error, "Invalid listen address: '%s'", name ? name : "(null)");
                return nullptr;
            }

            auto *socket = new UDPSocket(this);

            const ISocketAddress bind_addr{ip, port};

            if (!socket->Listen(bind_addr, blocking))
            {
                GG_LOG(GG::LogLevel::Error, "Listen failed %.*s:%u",
                       (int)ip.size(), ip.data(), (unsigned)port);
                delete socket;
                return nullptr;
            }

            GG_LOG(GG::LogLevel::Info, "SocketManager listening on %.*s:%u",
                   (int)ip.size(), ip.data(), (unsigned)port);

            m_sockets.push_back(socket);
            return socket;
        }

        inline UDPSocket *CreateSocket() override
        {
            GG_LOG(GG::LogLevel::Info, "SocketManager::CreateSocket()");
            return nullptr;
        }

        inline void CloseSockets()
        {
            for (auto *s : m_sockets)
                if (s)
                    s->Close();
        }

        inline ~SocketManager() = default;

    private:
        static inline bool parse_ipv4_and_port(
            const char *in,
            std::string_view &out_ip,
            std::uint16_t &out_port) noexcept
        {
            if (!in)
                return false;

            std::string_view s(in);
            const auto colon = s.find(':');

            out_ip = (colon == std::string_view::npos) ? s : s.substr(0, colon);
            if (out_ip.empty())
                out_ip = "0.0.0.0";

            if (colon == std::string_view::npos)
                return false;

            const auto port_sv = s.substr(colon + 1);
            if (port_sv.empty())
                return false;

            unsigned port_u = 0;
            const auto *first = port_sv.data();
            const auto *last = port_sv.data() + port_sv.size();

            auto [ptr, ec] = std::from_chars(first, last, port_u, 10);
            if (ec != std::errc{} || ptr != last)
                return false;
            if (port_u > 65535u)
                return false;

            out_port = static_cast<std::uint16_t>(port_u);
            return true;
        }

        std::list<UDPSocket *> m_sockets;
    };
}
