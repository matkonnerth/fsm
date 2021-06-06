#include <iostream>
#include <optional>
#include <variant>

template <class... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

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

using State = std::variant<Opened, Closed, Locked>;

std::string StateName(const State &state)
{
    std::string name;
    std::visit(overload{[&](const Opened &) { name = "Opened"; },
                        [&](const Closed &) { name = "Closed"; },
                        [&](const Locked &) { name = "Locked"; }},
               state);
    return name;
}

struct Close
{
};

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

using ExternalEvent = std::variant<Close, Open, Lock, Unlock>;

template <typename Derived, typename StateVariant>
class fsm
{
  public:
    const StateVariant &get_state() const { return state_; }
    StateVariant &get_state() { return state_; }

    template <typename Event>
    void dispatch(Event &&event)
    {
        Derived &child = static_cast<Derived &>(*this);
        auto new_state = std::visit(
            [&](auto &s) -> std::optional<StateVariant> {
                return child.on_event(s, std::forward<Event>(event));
            },
            state_);
        if (new_state)
        {
            transitionDone(state_, *new_state);
            state_ = *std::move(new_state);
        }
        else
        {
            transitionError(state_);
        }
    }

  private:
    StateVariant state_;
    void transitionDone(const StateVariant &from, const StateVariant &to)
    {
        std::cout << "transition from " << StateName(from) << " to "
                  << StateName(to) << "\n";
    }

    void transitionError(const StateVariant &s)
    {
        std::cout << "transition error, stay in state " << StateName(s) << "\n";
    }
};

class Door : public fsm<Door, State>
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

int main(int, char *[])
{
    Door door{};
    door.dispatch(Close{});
    door.dispatch(Lock{123});
    door.dispatch(Unlock{23});
    door.dispatch(Unlock{123});
    door.dispatch(Open{});
    door.dispatch(Open{});
}