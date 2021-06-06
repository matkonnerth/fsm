#pragma once
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
template <typename Derived, typename StateVariant>
class fsm
{
  public:
    const StateVariant &get_state() const { return m_state; }
    StateVariant &get_state() { return m_state; }

    template <typename Event>
    void dispatch(Event &&event)
    {
        Derived &child = static_cast<Derived &>(*this);
        auto new_state = std::visit(
            [&](auto &s) -> std::optional<StateVariant> {
                return child.on_event(s, std::forward<Event>(event));
            },
            m_state);
        if (new_state)
        {
            transitionDone(m_state, *new_state);
            m_state = *std::move(new_state);
        }
        else
        {
            transitionError(m_state);
        }
    }

  private:
    StateVariant m_state;
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