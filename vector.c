//
// Created by Pascal on 7/5/18.
//


struct vector{
    double x,y,z;
};

double sqr(double val){
    return  val*val;
}

struct vector sqrVector(struct vector a){
    struct vector b;
    b.x = a.x*a.x;
    b.y = b.y*b.y;
    b.z = b.z*b.z;
    return b;
}

struct vector add(struct vector a, struct vector b){
    struct vector c;
    c.x = a.x+b.x;
    c.y = a.y+b.y;
    c.z = a.z+b.z;
    return c;
}

struct vector multiply(struct vector a, struct vector b){
    struct vector c;
    c.x = a.x*b.x;
    c.y = a.y*b.y;
    c.z = a.z*b.z;
    return c;
}


