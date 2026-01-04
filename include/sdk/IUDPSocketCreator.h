#pragma once

namespace fb {
    class IUDPSocketCreator {
    public:
        virtual void Close(UDPSocket* socket) = 0;
    };
}