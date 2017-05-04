// feel good.
#include "quadtree.h"
#include "dict.h"
#include "sadlist_alg.h"
#include "ngx_queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#define malloc malloc
#define free free

struct quadnode {
	int id;
	float x;
	float y;

	struct list     *w;
	struct list     *m;
};

struct quadtree {

	struct quadnode *leaf;
	int              leafsz;

	float            w;
	float            h;
	float            tw;
	float            th;
	int            xmax;
	int            ymax;

	ngx_queue_t freelist;
	int obj_id;

	dictType dt;
	dict * hash;

	quadtree_m_cb m_cb;
	quadtree_w_cb w_cb;
};

static void m_cb(list *we, list *wl, struct obj *m) {
	if (we) {
		listIter *iter = listGetIterator(we, AL_START_HEAD);
		for (listNode *node = NULL; (node = listNext(iter)) != NULL;) {
			struct obj *o = (struct obj *)node->value;
			o->etr(o, m);
		}
	}
	if (wl) {
		listIter *iter = listGetIterator(we, AL_START_HEAD);
		for (listNode *node = NULL; (node = listNext(iter)) != NULL;) {
			struct obj *o = (struct obj *)node->value;
			o->lve(o, m);
		}
	}
}

static void w_cb(struct obj *w, list *me, list *ml) {
	if (me) {
		listIter *iter = listGetIterator(me, AL_START_HEAD);
		for (listNode *node = NULL; (node = listNext(iter)) != NULL;) {
			struct obj *o = (struct obj *)node->value;
			w->etr(w, o);
		}
	}
	if (ml) {
		listIter *iter = listGetIterator(ml, AL_START_HEAD);
		for (listNode *node = NULL; (node = listNext(iter)) != NULL;) {
			struct obj *o = (struct obj *)node->value;
			w->lve(w, o);
		}
	}
}

static unsigned int hashFunction(const void * key) {
	int *o = key;
	int b = 378551;
	int a = 63689;
	unsigned int hash = 0;
	hash = a * (*o) + b;
	return hash;
}

int keyCompare(void *privdata, const void *key1, const void *key2) {
	int *a = key1;
	int *b = key2;
	if ((*a) == (*b)) {
		return 1;
	} else {
		return 0;
	}
}

static int  t2o(int x, int y, int xmax, int ymax) {
	//int l = y - 1 >= 0 ? y - 1 : 0;
	return y * xmax + x;
}

struct quadtree *
	quadtree_alloc(struct rect rt, float twidth, float theight) {
	struct quadtree *tree = (struct quadtree *)malloc(sizeof(*tree));
	memset(tree, 0, sizeof(*tree));

	tree->w = rt.max.x - rt.min.x;
	tree->h = rt.max.y - rt.min.y;
	tree->tw = twidth;
	tree->th = theight;

	tree->xmax = ceil(tree->w / tree->tw);
	tree->ymax = ceil(tree->h / tree->th);

	tree->leaf = malloc(sizeof(struct quadnode) * tree->xmax * tree->ymax);
	tree->leafsz = tree->xmax * tree->ymax;

	for (size_t i = 0; i < tree->ymax; i++) {
		for (size_t j = 0; j < tree->xmax; j++) {
			int idx = t2o(j, i, tree->xmax, tree->ymax);
			struct quadnode * node = &(tree->leaf[idx]);
			node->x = j;
			node->y = i;
			node->id = idx;
			node->w = listCreate();
			node->m = listCreate();
		}
	}

	ngx_queue_init(&tree->freelist);
	tree->obj_id = 0;

	dictType *dt = &tree->dt;
	dt->hashFunction = hashFunction;
	dt->keyCompare = keyCompare;

	tree->hash = dictCreate(dt, NULL);
	//dictExpand(tree->hash, 13);
	tree->m_cb = m_cb;
	tree->w_cb = w_cb;

	return tree;
}

void
quadtree_free(struct quadtree *self) {
	free(self->leaf);

	dictRelease(self->hash);

	free(self);
}

struct obj *
	quadtree_create_obj(struct quadtree *self) {
	if (ngx_queue_empty(&self->freelist)) {
		char *ptr = malloc(sizeof(ngx_queue_t) + sizeof(struct obj));
		memset(ptr, 0, sizeof(ngx_queue_t) + sizeof(struct obj));
		ngx_queue_init(ptr2q(ptr));

		struct obj *o = ptr2o(ptr);
		o->ref = 1;
		o->id = ++self->obj_id;
		return o;
	} else {
		ngx_queue_t *q = ngx_queue_head(&self->freelist);
		char *ptr = q2ptr(q);
		struct obj *o = ptr2o(ptr);
		o->ref = 1;
		return o;
	}
}

void quadtree_release_obj(struct quadtree *self, struct obj *o) {
	assert(o->ref == 0);
	char *ptr = o2ptr(o);
	ngx_queue_add(&(self->freelist), ptr2q(ptr));
}

static list *
quadtree_find_li(struct quadtree *self, struct rect box) {
	list *ret = listCreate();
	float minx = box.min.x / self->tw;
	float miny = box.min.y / self->th;
	float maxx = box.max.x / self->tw;
	float maxy = box.max.y / self->th;

	for (size_t i = miny; i <= maxy; i++) {
		for (size_t j = minx; j <= maxx; j++) {

			int idx = t2o(j, i, self->xmax, self->ymax);
			slistInsertObj(ret, &(self->leaf[idx]));
		}
	}
	return ret;
}

static struct quadnode *
quadtree_find_node(struct quadtree *self, int idx) {
	return &(self->leaf[idx]);
}

static int
quadtree_trans(struct quadtree *self, struct vector3 src) {
	assert(src.x >= 0 && src.y >= 0);
	assert(src.x <= self->w && src.y <= self->h);

	int x, y;
	x = floor(src.x / self->tw);
	y = floor(src.y / self->th);
	return t2o(x, y, self->xmax, self->ymax);
}

static int
quadtree_range_limit(struct quadtree *self, struct vector3 pos, float radis, struct rect *box) {
	/*box->min.x = pos.x - radis;
	box->min.x = box->min.x < 0 ? 0 : box->min.x;
	box->min.y = pos.y - radis;
	box->min.y = box->min.y < 0 ? 0 : box->min.y;

	box->max.x = pos.x + radis;
	box->max.x = box->max.x > self->w ? self->w : box->max.x;
	box->max.y = pos.y + radis;
	box->max.y = box->max.y > self->h ? self->h : box->max.y;*/

	box->min.x = pos.x - radis >= 0 ? pos.x - radis : 0;
	box->min.y = pos.y - radis >= 0 ? pos.y - radis : 0;
	box->max.x = pos.x + radis <= 0 ? pos.x - radis : 0;
	box->max.y = pos.y + radis <= 0 ? pos.y - radis : 0;

	return 1;
}

int
quadtree_insert(struct quadtree *self, struct obj *o, struct vector3 pos, float radis) {
	dictEntry *de = dictFind(self->hash, &(o->id));
	if (de == NULL) {
		dictAdd(self->hash, &(o->id), o);
	}

	if (obj_iss(o, obj_enum_sm)) {
		int idx = quadtree_trans(self, pos);
		struct quadnode *node = quadtree_find_node(self, idx);
		if (listAddNodeTail(node->m, o) == NULL) {
			assert(false);
			return;
		} else {
			obj_incr_ref(o);
			o->qnode = node;
			if (self->m_cb) {
				self->m_cb(node->w, NULL, o);
			}
		}
	}

	if (obj_iss(o, obj_enum_sw)) {
		o->pos = pos;
		o->radis = radis;
		struct rect box;
		quadtree_range_limit(self, pos, radis, &box);

		list *node = quadtree_find_li(self, box);
		listIter *iter = listGetIterator(node, AL_START_HEAD);
		for (listNode *node = NULL; (node = listNext(iter)) != NULL;) {
			struct quadnode *qnode = node->value;
			if (listAddNodeTail(qnode->w, o) == NULL) {
			} else {
				obj_incr_ref(o, self);
			}
		}
		o->wqnode = node;
	}
}

struct obj *
	quadtree_remove(struct quadtree *self, int id) {
	struct obj *o = dictGetVal(dictFind(self->hash, &id));
	assert(o != NULL);

	if (obj_iss(o, obj_enum_sm)) {
		if (self->m_cb) {
			self->m_cb(NULL, o->qnode->w, o);
		}
		listNode *node = listSearchKey(o->qnode->m, o);
		listDelNode(o->qnode->m, node);
		//obj_decr_ref(o, &(self->freelist));
	}

	if (obj_iss(o, obj_enum_sw)) {

		/*struct rect box;
		quadtree_range_limit(self, o->pos, o->radis, &box);
		list *qnode = quadtree_find_li(self, box);*/

		listIter *iter = listGetIterator(o->wqnode, AL_START_HEAD);
		for (listNode *node = NULL; (node = listNext(iter)) != NULL;) {
			struct quadnode *qnode = node->value;

			listNode *node = listSearchKey(o->qnode->w, o);
			listDelNode(o->qnode->w, node);
			//obj_decr_ref(o, &(self->freelist));
		}
	}
}

void
quadtree_update(struct quadtree *self, int id, struct vector3 pos, float radis) {
	struct obj *o = dictGetVal(dictFind(self->hash, &id));
	assert(o != NULL);

	if (obj_iss(o, obj_enum_sw)) {
		obj_set_pos(o, pos);
		struct quadnode *node = quadtree_find_node(self, pos.x, pos.y);
		if (node == o->qnode) {
		} else {
			self->m_cb(node->w, o->qnode->w, o);
			o->qnode = node;
		}
	}

	if (obj_iss(o, obj_enum_sm)) {

		obj_set_pos(o, pos);
		obj_set_radis(o, radis);

		struct rect box;
		quadtree_range_limit(self, pos, radis, &box);
		list *li = quadtree_find_li(self, box);

		list *e = listCreate();
		list *l = listCreate();

		slistSubtractObj(o->wqnode, li, &l);
		slistSubtractObj(li, o->wqnode, &e);

		list *me = listCreate();
		list *ml = listCreate();

		listIter *eiter = listGetIterator(e, AL_START_HEAD);
		for (listNode *enode = NULL; (enode = listNext(eiter)) != NULL;) {

			listIter *miter = listGetIterator(((struct quadnode *)enode->value)->m, AL_START_HEAD);
			for (listNode *mnode = NULL; (mnode = listNext(miter)) != NULL;) {
				//slistInsertObj(list, mnode->value);
				listAddNodeTail(me, mnode->value);
			}
		}

		listIter *liter = listGetIterator(l, AL_START_HEAD);
		for (listNode *lnode = NULL; (lnode = listNext(liter)) != NULL;) {

			listIter *miter = listGetIterator(((struct quadnode *)lnode->value)->m, AL_START_HEAD);
			for (listNode *mnode = NULL; (mnode = listNext(miter)) != NULL;) {
				//slistInsertObj(list, mnode->value);
				listAddNodeTail(me, mnode->value);
			}
		}



		self->w_cb(o, me, ml);

		listRelease(e);
		listRelease(l);
		listRelease(me);
		listRelease(ml);

		listRelease(o->wqnode);
		o->wqnode = li;
	}


}