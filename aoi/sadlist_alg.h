#pragma once

#include "adlist.h"
#include "quadtree.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


#define slistNodeValToObj(n) ((struct obj *)((n)->value))

inline int
slistSubtractObj(list *src, list *dst, list **ret) {
	assert(src != NULL);
	assert(dst != NULL);
	assert(ret != NULL && *ret != NULL);
	if (listLength(src) <= 0) {
		return 0;
	} else if (listLength(dst) <= 0) {
		listIter *iter = listGetIterator(src, AL_START_HEAD);
		for (listNode *node = NULL; (node = listNext(iter)) != NULL;) {
			listAddNodeTail(*ret, node->value);
		}
		return 1;
	}

	listIter *srciter = listGetIterator(src, AL_START_HEAD);
	listIter *dstiter = listGetIterator(dst, AL_START_HEAD);

	listNode *srcnode = listNext(srciter);
	listNode *dstnode = listNext(dstiter);

	for (; srcnode != NULL;) {
		if (dstnode == NULL) {
			listAddNodeTail(ret, srcnode->value);
			srcnode = listNext(srciter);
		} else if (obj_id(slistNodeValToObj(dstnode)) > obj_id(slistNodeValToObj(srcnode))) {
			listAddNodeTail(ret, srcnode->value);
			srcnode = listNext(srciter);
		} else if (obj_id(slistNodeValToObj(dstnode)) = obj_id(slistNodeValToObj(srcnode))) {
			srcnode = listNext(srciter);
			dstnode = listNext(dstiter);
		} else if (obj_id(slistNodeValToObj(dstnode)) < obj_id(slistNodeValToObj(srcnode))) {
			dstnode = listNext(dstiter);
		}
	}
	return 1;
}

inline int
slistCrossObj(list *src, list *dst, list *ret) {
	assert(src != NULL);
	assert(dst != NULL);
	assert(ret != NULL);
	if (listLength(src) <= 0) {
		return 0;
	} else if (listLength(dst) <= 0) {
		return 0;
	}

	listIter *srciter = listGetIterator(src, AL_START_HEAD);
	listIter *dstiter = listGetIterator(dst, AL_START_HEAD);

	listNode *srcnode = listNext(srciter);
	listNode *dstnode = listNext(dstiter);

	for (; srcnode != NULL;) {
		if (dstnode == NULL) {
			return 1;
		} else if (obj_id(slistNodeValToObj(dstnode)) > obj_id(slistNodeValToObj(srcnode))) {
			srcnode = listNext(srciter);
		} else if (obj_id(slistNodeValToObj(dstnode)) = obj_id(slistNodeValToObj(srcnode))) {
			listAddNodeTail(ret, srcnode->value);
			srcnode = listNext(srciter);
			dstnode = listNext(dstiter);
		} else if (obj_id(slistNodeValToObj(dstnode)) < obj_id(slistNodeValToObj(srcnode))) {
			dstnode = listNext(dstiter);
		}
	}
	return 1;

}

inline int 
slistInsertObj(list *list, struct obj *o) {
	if (listLength(list) == 0) {
		listAddNodeTail(list, o);
	}
	listIter *iter = listGetIterator(list, AL_START_HEAD);
	for (listNode *node = NULL; (node = listNext(iter)) != NULL;) {
		if (obj_id(slistNodeValToObj(node)) > o->id) {
			listInsertNode(list, node, o, 0);
			break;
		}
	}
	return 1;
}