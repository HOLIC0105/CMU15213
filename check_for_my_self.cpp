#include <bits/stdc++.h>
using namespace std;
int isLessOrEqual(int x, int y) {
  int fy = ~y + 1;
  int val = x + fy;
  int s = val >> 31;
  int ans = !s;
  std::cout << val << " " << s << " " << ans << std::endl;
  return !(ans & (x ^ y));
}
signed main() {
  while(true) {
    int x, y, z;
    cin >> x >> y;
    std::cout << isLessOrEqual(x, y) << std::endl;
  }
  return 0;
}