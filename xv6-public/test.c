#include "types.h"
#include "stat.h"
#include "user.h"
#include "wmap.h"

int
main(int argc, char *argv[])
{
  uint address = wmap(0x60000000, 8192, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1);
  char *p = (char *)address;
  printf(1, "address: %p\n", p);
  *p = 'a';
  // for (int i = 0; i < 8000; ++i) {
  //   *(p + i) = 'a';
  // }
  exit();
}
