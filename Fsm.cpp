#include "Fsm.h"

State::State(void (*on_enter)(), void (*on_state)(), void (*on_exit)())
    : on_enter(on_enter),
      on_state(on_state),
      on_exit(on_exit)
{
}

Fsm::Fsm(State *initial_state)
    : m_current_state(initial_state),
      m_transitions(nullptr),
      m_num_transitions(0),
      m_num_timed_transitions(0),
      m_initialized(false)
{
}

Fsm::~Fsm()
{
  free(m_transitions);
  free(m_timed_transitions);
  m_transitions = nullptr;
  m_timed_transitions = nullptr;
}

void Fsm::add_transition(State *state_from, State *state_to, int event, void (*on_transition)())
{
  if (!state_from || !state_to)
    return;

  Transition transition = Fsm::create_transition(state_from, state_to, event, on_transition);
  Transition *new_transitions = (Transition *)realloc(m_transitions, (m_num_transitions + 1) * sizeof(Transition));

  if (new_transitions)
  {
    m_transitions = new_transitions;
    m_transitions[m_num_transitions] = transition;
    m_num_transitions++;
  }
}

void Fsm::add_timed_transition(State *state_from, State *state_to, unsigned long interval, void (*on_transition)())
{
  if (!state_from || !state_to)
    return;

  Transition transition = Fsm::create_transition(state_from, state_to, 0, on_transition);

  TimedTransition timed_transition;
  timed_transition.transition = transition;
  timed_transition.start = 0;
  timed_transition.interval = interval;

  TimedTransition *new_timed_transitions = (TimedTransition *)realloc(m_timed_transitions, (m_num_timed_transitions + 1) * sizeof(TimedTransition));

  if (new_timed_transitions)
  {
    m_timed_transitions = new_timed_transitions;
    m_timed_transitions[m_num_timed_transitions] = timed_transition;
    m_num_timed_transitions++;
  }
}

Fsm::Transition Fsm::create_transition(State *state_from, State *state_to, int event, void (*on_transition)())
{
  Transition t;
  t.state_from = state_from;
  t.state_to = state_to;
  t.event = event;
  t.on_transition = on_transition;
  return t;
}

void Fsm::trigger(int event)
{
  if (m_initialized)
  {
    for (int i = 0; i < m_num_transitions; ++i)
    {
      if (m_transitions[i].state_from == m_current_state && m_transitions[i].event == event)
      {
        Fsm::make_transition(&(m_transitions[i]));
        return;
      }
    }
  }
}

void Fsm::check_timed_transitions()
{
  unsigned long now = millis();
  for (int i = 0; i < m_num_timed_transitions; ++i)
  {
    TimedTransition *transition = &m_timed_transitions[i];
    if (transition->transition.state_from == m_current_state)
    {
      if (transition->start == 0)
      {
        transition->start = now;
      }
      else if (now - transition->start >= transition->interval)
      {
        Fsm::make_transition(&(transition->transition));
        transition->start = 0;
      }
    }
  }
}

void Fsm::run_machine()
{
  if (!m_initialized)
  {
    m_initialized = true;
    if (m_current_state->on_enter)
      m_current_state->on_enter();
  }

  if (m_current_state->on_state)
    m_current_state->on_state();

  Fsm::check_timed_transitions();
}

void Fsm::make_transition(Transition *transition)
{
  if (transition->state_from->on_exit)
    transition->state_from->on_exit();
  if (transition->on_transition)
    transition->on_transition();
  if (transition->state_to->on_enter)
    transition->state_to->on_enter();

  m_current_state = transition->state_to;

  unsigned long now = millis();
  for (int i = 0; i < m_num_timed_transitions; ++i)
  {
    TimedTransition *ttransition = &m_timed_transitions[i];
    if (ttransition->transition.state_from == m_current_state)
      ttransition->start = now;
  }
}
