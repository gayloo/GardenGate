#pragma once

#include <memory>

#include "base/Log.h"
#include "core/Offsets.h"
#include "sdk/SocketManager.h"
#include "sdk/ServerSpawnInfo.h"
#include "sdk/ServerSpawnOverrides.h"
#include "sdk/SocketManagerFactory.h"
#include "sdk/SecureReason.h"
#include "sdk/LevelSetupOption.h"

namespace GG
{
    class Game
    {
    public:
        void initialize()
        {
            m_socketManager = new fb::SocketManager();
        }

        void uninitialize();

        void onServerCreate(intptr_t inst, fb::ServerSpawnInfo& info) {
            if (info.isLocalHost) {
                m_hosting = false;
                m_joining = false;
            }
            else {
                m_hosting = true;
            }

            m_serverInst = inst;
        }

        fb::SocketManager* getSocketManager() { return m_socketManager; }

        bool isJoiningOrHosting() { return m_hosting || m_joining; }
        bool isHosting() { return m_hosting; }

        void logServerSpawnInfo(const fb::ServerSpawnInfo& info)
        {
            GG_LOG(LogLevel::Debug, "info->levelSetup->m_name = %s", info.levelSetup.m_name);

            for (int i = 0; i < (int)info.levelSetup.m_inclusionOptions.size(); ++i)
            {
                const auto& opt = info.levelSetup.m_inclusionOptions[i];
                const char* key = opt.m_criterion ? opt.m_criterion : "<null>";
                const char* value = opt.m_value ? opt.m_value : "<null>";

                GG_LOG(LogLevel::Debug, "info->levelSetup->m_inclusionOptions[k:v] = %s:%s", key, value);
            }

            GG_LOG(LogLevel::Debug, "info->tickFrequency = %d", info.tickFrequency);
            GG_LOG(LogLevel::Debug, "info->isSinglePlayer = %d", (int)info.isSinglePlayer);
            GG_LOG(LogLevel::Debug, "info->isLocalHost = %d", (int)info.isLocalHost);
            GG_LOG(LogLevel::Debug, "info->isDedicated = %d", (int)info.isDedicated);
            GG_LOG(LogLevel::Debug, "info->isEncrypted = %d", (int)info.isEncrypted);
            GG_LOG(LogLevel::Debug, "info->isCoop = %d", (int)info.isCoop);
            GG_LOG(LogLevel::Debug, "info->isMenu = %d", (int)info.isMenu);
            GG_LOG(LogLevel::Debug, "info->keepResources = %d", (int)info.keepResources);
        }

        void prepareServerSpawn(intptr_t inst, fb::ServerSpawnInfo& info, fb::ServerSpawnOverrides* spawnOverrides)
        {
            onServerCreate(inst, info);

            if (isJoiningOrHosting())
                spawnOverrides->socketManager = getSocketManager();
        }

        void logClientInitNetwork(bool singleplayer, bool localhost, bool coop, bool hosted)
        {
            GG_LOG(LogLevel::Info, "fb::client::InitNetwork(singleplayer: %d, localhost: %d, coop: %d, hosted: %d)",
                (int)singleplayer, (int)localhost, (int)coop, (int)hosted);
        }

        void injectSocketManagerFactory(intptr_t inst, std::size_t offset)
        {
            *reinterpret_cast<__int64*>(inst + offset) = 
                reinterpret_cast<__int64>(new fb::SocketManagerFactory());
            
            GG_LOG(LogLevel::Debug, "Injected SocketManagerFactory at offset 0x%zX", offset);
        }

        const char* redirectHostingAddress(const char* ipAddress)
        {
            if (isHosting())
                return "127.0.0.1:25200";
            return ipAddress;
        }

        void logClientConnect(const char* ipAddress, const char* serverPassword)
        {
            GG_LOG(LogLevel::Info, "fb::client::ConnectToAddress(ipAddress: %s, serverPassword: %s)", 
                ipAddress, serverPassword);
        }

        const char* adjustPeerAddress(const char* address, std::string_view port)
        {
            if (!isHosting())
                return address;

            if (port == "25200")
                return "0.0.0.0:25200";
            if (port == "25100")
                return "127.0.0.1:25100";
            
            return address;
        }

        void handleDisconnection(fb::SecureReason reason)
        {
            if (reason == fb::SecureReason_Ok)
                return;

            const auto msg = fb::secure_reason_message(reason);

            GG_LOG(LogLevel::Warning, "Disconnected[%d] - %.*s",
                (int)reason, (int)msg.size(), msg.data());

            if (!isHosting())
            {
                if (!fb::show_disconnected_dialog(reason))
                {
                    std::exit(EXIT_FAILURE);
                }
            }
        }

        void logPeerJoined(const char* player)
        {
            GG_LOG(LogLevel::Info, "Peer joined - %s", player);
        }

    private:
        intptr_t m_serverInst{0};
        bool m_hosting{false};
        bool m_joining{false};
        fb::SocketManager* m_socketManager{0};
    };
}

inline std::unique_ptr<GG::Game> g_game = nullptr;
