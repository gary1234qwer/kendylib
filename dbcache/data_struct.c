#include "data_struct.h"
#include <stdlib.h>
#include <stdio.h>

static void db_array_destroy(void *arg)
{
	db_array_t a = (db_array_t)arg;
	if(a->data)
	{
		int32_t i = 0;
		for( ; i < a->size; ++i)
			basetype_destroy(&(a->data[i]));
		
		free(a->data);
	}
	free(a);
	printf("a db_array destroy\n");
}

db_array_t db_array_create(int32_t size)
{
	db_array_t a = calloc(1,sizeof(*a));
	a->base.type = DB_ARRAY;
	a->base.ref.mt = 0;
	a->base.ref.refcount = 1;
	a->base.ref.destroyer = db_array_destroy;
	a->size = size;
	a->data = calloc(size,sizeof(*(a->data))); 
	return a;	
}

void db_array_release(db_array_t *a)
{
	ref_decrease((struct refbase*)*a);
	*a = NULL;
}

db_array_t db_array_acquire(db_array_t a1,db_array_t a2)
{
	if(a1 == a2)
		return a1;	
	if(a2)
		ref_increase((struct refbase*)a2);
	if(a1)
		db_array_release(&a1);

	return a2;
}


basetype_t db_array_get(db_array_t a,int32_t index)
{
	if(a->data && index < a->size)
		return a->data[index];
	return NULL;
}

void db_array_set(db_array_t a,int32_t index,basetype_t t)
{
	if(a->data && index < a->size)
	{
		if(a->data[index])
			basetype_destroy(&(a->data[index]));
		a->data[index] = t;	
	}
}

void db_array_clear(db_array_t a)
{
	int32_t i = 0;
	for( ; i < a->size; ++i)
	{
		if(a->data[i])
			basetype_destroy(&(a->data[i]));
	}
	free(a->data);
	a->data = NULL;
}

db_list_t db_list_create()
{
	db_list_t l = calloc(1,sizeof(*l));
	l->l = LINK_LIST_CREATE();
	return l;	
}

void      db_list_destroy(db_list_t *l)
{
	struct db_node *cur;
	while(cur = (struct db_node *)link_list_pop((*l)->l))
	{
		db_array_release(&(cur->array));
		free(cur);
	}
	
	LINK_LIST_DESTROY(&(*l)->l); 
	free(*l);
	*l = NULL;
}

int32_t   db_list_append(db_list_t l,db_array_t a)
{
	struct db_node *n = calloc(1,sizeof(*n));
	n->array = a;
	LINK_LIST_PUSH_BACK(l->l,n);
	//increase the reference
	ref_increase((struct refbase*)a);	
	return link_list_size(l->l);
}

int32_t   db_list_size(db_list_t l)
{
	return link_list_size(l->l);
}

int32_t   db_list_shrink(db_list_t l)
{
	int32_t s = link_list_size(l->l);
	int32_t i = 0;
	for(; i < s; ++i)
	{
		struct db_node *cur = (struct db_node *)link_list_pop(l->l);
		if(cur->array->data == NULL)
		{
			db_array_release(&(cur->array));
			free(cur);
		}else
		{
			LINK_LIST_PUSH_BACK(l->l,cur);
		}
	}
	return link_list_size(l->l);
}
