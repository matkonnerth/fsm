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

struct Unlocked
{
};

struct Locked
{
    int key;
};

using ClosedSubState = std::variant<Unlocked, Locked>;
using State = std::variant<Opened, ClosedSubState>;

std::string StateName(const State &state)
{
    return std::visit(overload{[](const Opened &) {
            return "Opened"; },
                               [](const ClosedSubState &) {
            return "Closed"; }},
                      state);
}

std::string StateName(const ClosedSubState& state)
{
        return std::visit(overload{[](const Unlocked &) { return "Unlocked"; },
                                   [](const Locked &) { return "Locked"; }},
                          state);
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

template <typename Derived, typename StateVariant>
class fsm
{
      public:
        const StateVariant &get_state() const { return state_; }
        StateVariant &get_state() { return state_; }

        template <typename Event>
        void dispatch(Event && event)
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
            std::cout << "transition error, stay in state " << StateName(s)
                      << "\n";
        }
};

class Locking : public fsm<Locking, ClosedSubState>
{
      public:
        template <typename State, typename Event>
        std::optional<ClosedSubState> on_event(State &, const Event &)
        {
            return std::nullopt;
        }
        auto on_event(Unlocked &, const Lock &l) { return Locked{l.key}; };
        auto on_event(Locked & l, const Unlock &u)
            ->std::optional<ClosedSubState>
        {
            if (u.key == l.key)
            {
                return Unlocked{};
            }
            return std::nullopt;
        }
};

class Door : public fsm<Door, State>
{
      public:
        auto on_event(Opened &, const Close &)
        {
            return ClosedSubState{Unlocked{}};
        }
        auto on_event(ClosedSubState & c, const Open &)->std::optional<State>
        {
            if (std::holds_alternative<Unlocked>(c))
            {
                return Opened{};
            }
            std::cout << "cannot open, door is locked"
                      << "\n";
            return std::nullopt;
        }
        // forward to substate machine
        auto on_event(ClosedSubState &, const Lock &l)
        {
            locking.dispatch(l);
            return ClosedSubState{locking.get_state()};
        }
        auto on_event(ClosedSubState &, const Unlock &u)
        {
            locking.dispatch(u);
            //hm, think about that
            return ClosedSubState{locking.get_state()};
        }
        template <typename State, typename Event>
        std::optional<State> on_event(State &, const Event &)
        {
            return std::nullopt;
        }

      private:
        Locking locking{};
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