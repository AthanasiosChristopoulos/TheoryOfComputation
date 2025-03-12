
#include <stdio.h>

typedef struct Circle {
    double x, y;     // Scalar values for position
    double radius;   // Scalar value for radius

    double (*area)(struct Circle *self);
    double (*perimeter)(struct Circle *self);
	
} Circle;

// Function definitions
double Circle_area(struct Circle *self) {
    return 3.14 * (self->radius * self->radius);
}

double Circle_perimeter(struct Circle *self) {
    return 2 * 3.14 * self->radius;
}

// Constructor-like initializer
Circle ctor_Circle = { .area = Circle_area, .perimeter = Circle_perimeter };

int main() {
    // Creating a Circle instance
    Circle myCircle = ctor_Circle;
    myCircle.x = 0.0;
    myCircle.y = 0.0;
    myCircle.radius = 5.0;

    // Calling methods
    printf("Area: %f\n", myCircle.area(&myCircle));
    printf("Perimeter: %f\n", myCircle.perimeter(&myCircle));

    return 0;
}


