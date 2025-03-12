#include <stdio.h>
#include <stdlib.h>
#include "lambdalib.h"
typedef struct Address {
  char* street;
  int number;
  char* city;
   (*setAddress)(struct Address *self, char* s, int n, char* c);
   (*printAddress)(struct Address *self);
} Address;
 printAddress(struct Address *self) {
    write("Address: %s %d, %s\n", street, number, city);

}
 setAddress(struct Address *self, char* s, int n, char* c) {
    street = s;
    number = n;
    city = c;

}

Address ctor_Address = {.printAddress = .printAddress, .setAddress = .setAddress};
typedef struct Circle {
  double x, y;
  double radius;
  double (*area)(struct Circle *self);
  double (*perimeter)(struct Circle *self, double dx, double dy);
} Circle;
double perimeter(struct Circle *self, double dx, double dy) {
    return 2 * 3.14 * radius;

}
double area(struct Circle *self) {
    return 3.14 * (radius ** 2);

}

Circle ctor_Circle = {.perimeter = .perimeter, .area = .area};

