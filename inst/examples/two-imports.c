// used webassembly.studio to compile it
extern int add(int a, int b);
extern int add2(int a, int b);

__attribute__((visibility("default")))
double sum(double a, double b) {
  return add(a, b) + add2(a, b);
}
