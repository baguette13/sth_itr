#pragma once

#include "BaseState.hpp"

namespace ue
{

class ConnectedState : public BaseState
{
public:
    ConnectedState(Context& context);

    // IBtsEventsHandler interface
    void handleDisconnected() override;
};

}
