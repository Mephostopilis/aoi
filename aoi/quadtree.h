#ifndef quadtree_h
#define quadtree_h

#include "math3d.h"
#include "adlist.h"

struct rect {
	struct vector3 min;
	struct vector3 max;
};

struct cicle {
	struct vector3 center;
	float radis;
};

struct range {
	int min_x;
	int min_y;
	int max_x;
	int max_y;
}; // ÎÞÓÃ

#define cicle_radis(x) ((x)->radis)

struct obj;

typedef void(*obj_enter_cb)(struct obj *a, struct obj *b);
typedef void(*obj_leave_cb)(struct obj *a, struct obj *b);

struct obj {
	int              id;
	int              s;   // state
	int              ref;
	struct vector3   pos;
	float            radis;
	void            *ud;
	obj_enter_cb     etr;
	obj_leave_cb     lve;
	struct quadnode *qnode;
	list            *wqnode;
};

#define obj_enum_sn 0
#define obj_enum_sw (1 << 0)
#define obj_enum_sm (1 << 1)
#define obj_enum_swm (obj_enum_sw | obj_enum_sm)

#define ptr2o(x) ((struct obj *)((x) + sizeof(ngx_queue_t)))
#define ptr2q(x) ((ngx_queue_t *)(x))
#define o2ptr(x) ((char *)((x) - sizeof(ngx_queue_t)))
#define q2ptr(x) ((char *)(x))

#define obj_sets(x, ss) ((x)->s |= (ss))
#define obj_gets(x) ((x)->s)
#define obj_iss(x, ss) ((x)->s & (ss))
#define obj_clrs(x) ((x)->s = obj_enum_sn)

#define obj_id(x) ((x)->id)

#define obj_set_etr(x, m) ((x)->etr = (m))
#define obj_set_lve(x, m) ((x)->lve = (m))

#define obj_incr_ref(x, pool) ((x)->ref++)
#define obj_decr_ref(x, pool) ( \
do { \
(x)->ref--; \
if ((x)->ref <= 0) { \
	ngx_queue_add((pool), ptr2q(o2ptr(x))); \
} \
} while (0)) 

#define obj_ref(x) ((x)->ref)

#define obj_set_pos(x, pos) ((x)->pos = (pos))
#define obj_set_radis(x, radis) ((x)->radis = (radis))


typedef void(*quadtree_m_cb)(list *we, list *wl, struct obj *m);
typedef void(*quadtree_w_cb)(struct obj *w, list *me, list *ml);

struct quadnode;
struct quadtree;

struct quadtree *
quadtree_alloc(struct rect rt, float twidth, float theight);

void
quadtree_free(struct quadtree *self);

struct obj *
quadtree_create_obj(struct quadtree *self);

void quadtree_release_obj(struct quadtree *self, struct obj *o);

int
quadtree_insert(struct quadtree *self, struct obj *o, struct vector3 pos, float radis);

struct obj *
quadtree_remove(struct quadtree *self, int id);

void
quadtree_update(struct quadtree *self, int id, struct vector3 pos, float radis);

#endif

