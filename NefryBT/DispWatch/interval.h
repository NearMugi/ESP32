template <uint32_t time>
class interval {
    uint32_t next_run = 0;

    template <class T>
    void _run(T func) {
      uint32_t now = micros();
      if (next_run < now) {
        func();
        next_run = now + time;
      }
    }

    interval() {}
    
  public:
    template <class T>
    static void run(T func) {
      static interval<time> instance;
      instance._run(func);
    }
};
