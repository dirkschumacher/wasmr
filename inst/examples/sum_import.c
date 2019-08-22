// used webassembly.studio to compile it
extern double add(double a, double b);

__attribute__((visibility("default")))
double sum(double a, double b) {
  return add(a, b) + 42;
}
