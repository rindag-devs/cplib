#include <iostream>

signed main() {
  int n;
  std::cin >> n;
  int l = 1, r = n;
  while (l < r) {
    int mid = (l + r) >> 1;
    std::cout << "Q " << mid << '\n';
    std::cout.flush();
    char res;
    std::cin >> res;
    if (res == '>')
      r = mid - 1;
    else if (res == '=')
      l = r = mid;
    else
      l = mid + 1;
  }
  std::cout << "A " << l << '\n';
  std::cout.flush();
}
