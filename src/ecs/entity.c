#include <ecs/entity.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



#ifdef _MSC_VER
static const __declspec(allocate(".entcomp$a")) entity_component_noconst_t _entity_components_start=NULL;
static const __declspec(allocate(".entcomp$z")) entity_component_noconst_t _entity_components_end=NULL;
static const __declspec(allocate(".enttype$a")) _entity_type_initializer_func_t _entity_type_initializers_start=NULL;
static const __declspec(allocate(".enttype$z")) _entity_type_initializer_func_t _entity_type_initializers_end=NULL;
#define ENTITY_COMPONENTS_START _entity_components_start
#define ENTITY_COMPONENTS_END _entity_components_end
#define ENTITY_TYPE_INITIALIZERS_START _entity_type_initializers_start
#define ENTITY_TYPE_INITIALIZERS_END _entity_type_initializers_end
#else
extern const entity_component_noconst_t __start_entcomp;
extern const entity_component_noconst_t __stop_entcomp;
extern const _entity_type_initializer_func_t __start_enttype;
extern const _entity_type_initializer_func_t __stop_enttype;
#define ENTITY_COMPONENTS_START __start_entcomp
#define ENTITY_COMPONENTS_END __stop_entcomp
#define ENTITY_TYPE_INITIALIZERS_START __start_enttype
#define ENTITY_TYPE_INITIALIZERS_END __stop_enttype
#endif



#define ENTITY_FLAG_CURRENTLY_UPDATING ENTITY_FLAG_INTERNAL0
#define ENTITY_FLAG_DELETE ENTITY_FLAG_INTERNAL1



static entity_type_noconst_t _entity_root_type;
static entity_component_t _entity_components[ENTITY_MAX_COMPONENTS];



void _entity_declare_type_internal(entity_type_noconst_t type,const entity_component_t* components,unsigned int order){
	for (unsigned int i=0;i<ENTITY_MAX_COMPONENTS;i++){
		type->data_offsets[i]=ENTITY_COMPONENT_NOT_PRESENT;
		type->extra_data_offsets[i]=ENTITY_COMPONENT_INVALID_EXTRA_DATA_OFFSET;
	}
	unsigned int function_array_length[ENTITY_COMPONENT_MAX_FUNCTION+1];
	for (unsigned int i=0;i<=ENTITY_COMPONENT_MAX_FUNCTION;i++){
		type->_functions[i]=malloc(sizeof(unsigned short int));
		function_array_length[i]=1;
	}
	unsigned short int size=sizeof(struct _ENTITY);
	unsigned short int extra_data_size=0;
	unsigned short int next_index=ENTITY_COMPONENT_INVALID_INDEX;
	const entity_component_t* end=components;
	while (*components){
		components++;
	}
	do{
		components--;
		entity_component_t component=*components;
		type->data_offsets[component->id]=(size<<16)|next_index;
		next_index=component->id;
		size+=(component->ctx_size+7)&0xfffffff8;
		if (component->type_ctx_size){
			type->extra_data_offsets[component->id]=extra_data_size;
			extra_data_size+=(component->type_ctx_size+7)&0xfffffff8;
		}
		for (unsigned int i=0;i<=ENTITY_COMPONENT_MAX_FUNCTION;i++){
			if (component->_functions[i]){
				function_array_length[i]++;
				type->_functions[i]=realloc(type->_functions[i],function_array_length[i]*sizeof(unsigned short int));
				type->_functions[i][function_array_length[i]-2]=component->id;
			}
		}
	} while (components!=end);
	for (unsigned int i=0;i<=ENTITY_COMPONENT_MAX_FUNCTION;i++){
		type->_functions[i][function_array_length[i]-1]=ENTITY_COMPONENT_INVALID_INDEX;
	}
	if (!_entity_root_type){
		_entity_root_type=type;
		type->_next_type=NULL;
	}
	else if (order<=_entity_root_type->_order){
		type->_next_type=_entity_root_type;
		_entity_root_type=type;
	}
	else{
		entity_type_noconst_t root=_entity_root_type;
		while (root->_next_type&&root->_next_type->_order<order){
			root=root->_next_type;
		}
		type->_next_type=root->_next_type;
		root->_next_type=type;
	}
	type->size=size;
	type->first_component_index=next_index;
	type->_order=order;
	type->extra_data=(extra_data_size?malloc(extra_data_size):NULL);
	const unsigned short int* function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_TYPE_INIT];
	while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX){
		_entity_components[*function_array]->t_init(type,ENTITY_TYPE_GET_COMPONENT_DATA_INDEXED(type,*function_array));
		function_array++;
	}
}



void entity_init(void){
	_entity_root_type=NULL;
	const entity_component_noconst_t* initializer=(const entity_component_noconst_t*)(&ENTITY_COMPONENTS_START);
	unsigned int component_id=0;
	while (initializer<(const entity_component_noconst_t*)(&ENTITY_COMPONENTS_END)){
		if (*initializer){
			if (component_id==ENTITY_MAX_COMPONENTS){
				printf("[ERROR] Too many components\n");
			}
			_entity_components[component_id]=*initializer;
			(*initializer)->id=component_id;
			component_id++;
		}
		initializer++;
	}
	const _entity_type_initializer_func_t* initializer_func=(const _entity_type_initializer_func_t*)(&ENTITY_TYPE_INITIALIZERS_START);
	while (initializer_func<(const _entity_type_initializer_func_t*)(&ENTITY_TYPE_INITIALIZERS_END)){
		if (*initializer_func){
			(*initializer_func)();
		}
		initializer_func++;
	}
}



entity_t entity_create(entity_type_t type){
	entity_type_allocator_t* allocator=type->allocator;
	if ((type->flags&ENTITY_TYPE_FLAG_SINGLETON)&&allocator->used){
		printf("[ERROR] Unable to create entity; type is a singleton\n");
		return NULL;
	}
	entity_t out;
	if (allocator->unused_count){
		allocator->unused_count--;
		out=allocator->unused;
		allocator->unused=out->_next_entity;
	}
	else{
		out=malloc(type->size);
	}
	out->type=type;
	out->index=(allocator->used?allocator->used->index+1:0);
	out->_next_entity=allocator->used;
	out->_prev_entity=NULL;
	if (allocator->used){
		allocator->used->_prev_entity=out;
	}
	allocator->used=out;
	allocator->used_count++;
	const unsigned short int* function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_TYPE_RESIZE];
	while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX){
		_entity_components[*function_array]->t_resize(type,ENTITY_TYPE_GET_COMPONENT_DATA_INDEXED(type,*function_array),allocator->used_count);
		function_array++;
	}
	out->flags=0;
	function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_ENTITY_INIT];
	while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX){
		_entity_components[*function_array]->e_init(out,ENTITY_GET_COMPONENT_DATA_INDEXED(out,*function_array));
		function_array++;
	}
	return out;
}



void entity_delete(entity_t entity){
	if (entity->flags&ENTITY_FLAG_DELETE){
		return;
	}
	entity->flags|=ENTITY_FLAG_DELETE;
	if (entity->flags&ENTITY_FLAG_CURRENTLY_UPDATING){
		return;
	}
	const entity_type_t type=entity->type;
	const unsigned short int* function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_ENTITY_DEINIT];
	while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX){
		_entity_components[*function_array]->e_deinit(entity,ENTITY_GET_COMPONENT_DATA_INDEXED(entity,*function_array));
		function_array++;
	}
	entity_type_allocator_t* allocator=entity->type->allocator;
	allocator->used_count--;
	function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_TYPE_RESIZE];
	while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX){
		_entity_components[*function_array]->t_resize(type,ENTITY_TYPE_GET_COMPONENT_DATA_INDEXED(type,*function_array),allocator->used_count);
		function_array++;
	}
	if (entity==allocator->used){
		allocator->used=entity->_next_entity;
		if (allocator->used){
			allocator->used->_prev_entity=NULL;
		}
	}
	else if (entity==allocator->used->_next_entity){
		entity_t root_entity=allocator->used;
		root_entity->_next_entity=entity->_next_entity;
		root_entity->index=entity->index;
		if (entity->_next_entity){
			entity->_next_entity->_prev_entity=root_entity;
		}
	}
	else{
		entity_t root_entity=allocator->used;
		allocator->used=root_entity->_next_entity;
		allocator->used->_prev_entity=NULL;
		root_entity->_next_entity=entity->_next_entity;
		root_entity->_prev_entity=entity->_prev_entity;
		root_entity->index=entity->index;
		if (entity->_next_entity){
			entity->_next_entity->_prev_entity=root_entity;
		}
		if (entity->_prev_entity){
			entity->_prev_entity->_next_entity=root_entity;
		}
	}
	if (allocator->unused_count>=ENTITY_ALLOCATOR_MAX_UNUSED_OBJECTS){
		free(entity);
	}
	else{
		entity->_next_entity=allocator->unused;
		allocator->unused=entity;
		allocator->unused_count++;
	}
}



void entity_delete_all(_Bool reset_allocator_cache){
	entity_type_t type=_entity_root_type;
	while (type){
		entity_type_allocator_t* allocator=type->allocator;
		while (allocator->used){
			entity_delete(allocator->used);
		}
		if (reset_allocator_cache){
			while (allocator->unused){
				void* next_ptr=allocator->unused->_next_entity;
				free(allocator->unused);
				allocator->unused=next_ptr;
			}
			allocator->unused_count=0;
		}
		type=type->_next_type;
	}
}



void entity_update_all(void){
	entity_type_t type=_entity_root_type;
	while (type){
		if (type->_functions[ENTITY_COMPONENT_FUNCTION_ENTITY_UPDATE][0]!=ENTITY_COMPONENT_INVALID_INDEX){
			entity_t entity=type->allocator->used;
			while (entity){
				entity->flags|=ENTITY_FLAG_CURRENTLY_UPDATING;
				const unsigned short int* function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_ENTITY_UPDATE];
				do{
					if (_entity_components[*function_array]->e_update(entity,ENTITY_GET_COMPONENT_DATA_INDEXED(entity,*function_array))||(entity->flags&ENTITY_FLAG_DELETE)){
						goto _skip_other_components;
					}
					function_array++;
				} while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX);
_skip_other_components:
				void* next_ptr=entity->_next_entity;
				entity->flags&=~ENTITY_FLAG_CURRENTLY_UPDATING;
				if (entity->flags&ENTITY_FLAG_DELETE){
					entity->flags&=~ENTITY_FLAG_DELETE;
					entity_delete(entity);
				}
				entity=next_ptr;
			}
		}
		type=type->_next_type;
	}
}



void entity_render(entity_type_t type){
	if (type->_functions[ENTITY_COMPONENT_FUNCTION_ENTITY_RENDER][0]!=ENTITY_COMPONENT_INVALID_INDEX){
		entity_t entity=type->allocator->used;
		while (entity){
			const unsigned short int* function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_ENTITY_RENDER];
			do{
				if (_entity_components[*function_array]->e_render(entity,ENTITY_GET_COMPONENT_DATA_INDEXED(entity,*function_array),ENTITY_TYPE_GET_COMPONENT_DATA_INDEXED(type,*function_array))){
					goto _skip_other_components;
				}
				function_array++;
			} while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX);
_skip_other_components:
			entity=entity->_next_entity;
		}
	}
	const unsigned short int* function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_TYPE_RENDER];
	while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX){
		_entity_components[*function_array]->t_render(type,ENTITY_TYPE_GET_COMPONENT_DATA_INDEXED(type,*function_array));
		function_array++;
	}
}



void entity_reset_all(void){
	entity_delete_all(1);
	entity_type_t type=_entity_root_type;
	while (type){
		const unsigned short int* function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_TYPE_DEINIT];
		while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX){
			_entity_components[*function_array]->t_deinit(type,ENTITY_TYPE_GET_COMPONENT_DATA_INDEXED(type,*function_array));
			function_array++;
		}
		type=type->_next_type;
	}
	type=_entity_root_type;
	while (type){
		const unsigned short int* function_array=type->_functions[ENTITY_COMPONENT_FUNCTION_TYPE_INIT];
		while (*function_array!=ENTITY_COMPONENT_INVALID_INDEX){
			_entity_components[*function_array]->t_init(type,ENTITY_TYPE_GET_COMPONENT_DATA_INDEXED(type,*function_array));
			function_array++;
		}
		type=type->_next_type;
	}
}



void entity_print_type_info(void){
	printf("Entity types:\n");
	entity_type_t type=_entity_root_type;
	if (!type){
		printf("  <no entity types present>\n");
		return;
	}
	do{
		printf("  %s:\n    Name: '%s'\n    Size: %u B\n    Flags:%s\n    Order: %u\n    Allocator:\n      Used objects: %u\n      Unused objects: %u\n    Components:\n",
			type->name,
			type->name,
			type->size,
			(type->flags&ENTITY_TYPE_FLAG_SINGLETON?" singleton":""),
			type->_order,
			type->allocator->used_count,
			type->allocator->unused_count
		);
		for (unsigned short int i=type->first_component_index;i!=ENTITY_COMPONENT_INVALID_INDEX;i=ENTITY_COMPONENT_OFFSET_GET_NEXT_INDEX(type,i)){
			printf("      %s:\n        Size: %u B\n        Type size: %u B\n        Flags:%s%s%s%s%s%s%s%s%s\n",
				_entity_components[i]->name,
				_entity_components[i]->ctx_size,
				_entity_components[i]->type_ctx_size,
				(_entity_components[i]->ctx_size?" <entity.data>":""),
				(_entity_components[i]->type_ctx_size?" <type.data>":""),
				(_entity_components[i]->e_init?" entity.init":""),
				(_entity_components[i]->e_update?" entity.update":""),
				(_entity_components[i]->e_render?" entity.render":""),
				(_entity_components[i]->e_deinit?" entity.deinit":""),
				(_entity_components[i]->t_init?" type.init":""),
				(_entity_components[i]->t_resize?" type.resize":""),
				(_entity_components[i]->t_render?" type.render":"")
			);
		}
		type=type->_next_type;
	} while (type);
}
