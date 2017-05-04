extern "C" {
#include "adlist.h"
#include "quadtree.h"
#include "wtime.h"
}

#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>

#include <Windows.h>

void getmsg(char *msg) {
	strcpy(msg, "Myworld");
}

class A {
public:
	A() { printf("class A controtor\n"); };
	virtual ~A() { printf("class A destroy\n"); };

	void func1() { printf("funcA1\n"); }
	virtual void func2() { printf("funcA2\n"); }

private:

};

class B : public A {
public:
	B() { printf("class B controtor\n"); }
	~B() { printf("class B destry\n"); }

	void func1() { printf("funcB1\n"); }
	virtual void func2() { printf("funcB2\n"); }
};

void enter_cb(struct obj *a, struct obj *b) {
	printf("%d enter %d\n", obj_id(b), obj_id(a));
}

void leave_cb(struct obj *a, struct obj *b) {
	printf("%d leave %d\n", obj_id(b), obj_id(a));
}

int main() {

	srand(time(NULL));

	struct rect box;
	box.max.x = 100;
	box.max.y = 100;
	box.min.x = 0;
	box.min.y = 0;

	struct quadtree *tree = quadtree_alloc(box, 2, 2);

	uint64_t s = getcurus();

	for (size_t i = 0; i < 4000; i++) {
		struct obj *o = quadtree_create_obj(tree);
		obj_set_etr(o, enter_cb);
		obj_set_lve(o, leave_cb);

		int r = rand();
		struct vector3 pos;
		pos.x = rand() % (100);
		pos.y = rand() % 100;

		float radis = rand() % 10;

		obj_sets(o, obj_enum_swm);

		quadtree_insert(tree, o, pos, radis);
	}

	for (size_t i = 1; i < 1000; i++) {
		struct vector3 pos;
		pos.x = rand() % (100);
		pos.y = rand() % 100;

		float radis = rand() % 10;

		int id = rand() % 100;
		quadtree_update(tree, i, pos, radis);
	}

	uint64_t e = getcurus();
	uint64_t ems = getcurms();

	printf("%ul us\n", e - s);

	/*char *msg = "Hello word";
	getmsg(msg);
	printf(msg);

	A *a = new B();
	a->func1();
	a->func2();
	delete a;*/

	system("pause");

	return 0;
}
