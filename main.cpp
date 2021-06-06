#include "fsm.h"
namespace door
{
struct Opened
{
};

struct Closed
{
};

struct Locked
{
    int key;
};
struct Close
{
};

using State = std::variant<Opened, Closed, Locked>;

struct Open
{
};

struct Lock
{
    int key;
};

struct Unlock
{
    int key;
};

std::string StateName(const State &state)
{
    std::string name;
    std::visit(overload{[&](const Opened &) { name = "Opened"; },
                        [&](const Closed &) { name = "Closed"; },
                        [&](const Locked &) { name = "Locked"; }},
               state);
    return name;
}

class DoorFsm : public fsm<DoorFsm, State>
{
  public:
    auto on_event(Opened &, const Close &) { return Closed{}; }
    auto on_event(Closed &, const Open &) { return Opened{}; }
    auto on_event(Closed &, const Lock &l) { return Locked{l.key}; }
    auto on_event(Locked &l, const Unlock &u) -> std::optional<State>
    {
        if (u.key == l.key)
        {
            return Closed{};
        }
        return std::nullopt;
    }
    template <typename State, typename Event>
    std::optional<State> on_event(State &, const Event &)
    {
        return std::nullopt;
    }
};
} // namespace Door

int main(int, char *[])
{
    using namespace door;
    
    DoorFsm door{};
    door.dispatch(Close{});
    door.dispatch(Lock{123});
    door.dispatch(Unlock{23});
    door.dispatch(Unlock{123});
    door.dispatch(Open{});
    door.dispatch(Open{});
}