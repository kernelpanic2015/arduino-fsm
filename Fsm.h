#ifndef FSM_H
#define FSM_H

#include <Arduino.h>

struct State
{
  State(void (*on_enter)(), void (*on_state)(), void (*on_exit)());
  void (*on_enter)();
  void (*on_state)();
  void (*on_exit)();
};

class Fsm
{
public:
  // Constructor and Destructor
  Fsm(State *initial_state);
  ~Fsm();

  // Public Methods
  void add_transition(State *state_from, State *state_to, int event, void (*on_transition)());
  void add_timed_transition(State *state_from, State *state_to, unsigned long interval, void (*on_transition)());
  void check_timed_transitions();
  void trigger(int event);
  void run_machine();

private:
  struct Transition
  {
    State *state_from;
    State *state_to;
    int event;
    void (*on_transition)();
  };

  struct TimedTransition
  {
    Transition transition;
    unsigned long start;
    unsigned long interval;
  };

  // Private Methods
  static Transition create_transition(State *state_from, State *state_to, int event, void (*on_transition)());
  void make_transition(Transition *transition);

  // Private Members
  State *m_current_state;
  Transition *m_transitions;
  int m_num_transitions;

  TimedTransition *m_timed_transitions;
  int m_num_timed_transitions;
  bool m_initialized;
};

#endif
