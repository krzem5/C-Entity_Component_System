#ifndef _ECS_ENTITY_H_
#define _ECS_ENTITY_H_
#include <stddef.h>
#include <stdint.h>



#ifdef _MSC_VER
#pragma section(".entcomp$a",read)
#pragma section(".entcomp$b",read)
#pragma section(".entcomp$z",read)
#pragma section(".enttype$a",read)
#pragma section(".enttype$b",read)
#pragma section(".enttype$z",read)
#define _ENTITY_COMPONENT_DATA_SECTION __declspec(allocate(".entcomp$b"))
#define _ENTITY_TYPE_INITIALIZER_DATA_SECTION __declspec(allocate(".enttype$b"))
#else
#define _ENTITY_COMPONENT_DATA_SECTION __attribute__((used,section("entcomp")))
#define _ENTITY_TYPE_INITIALIZER_DATA_SECTION __attribute__((used,section("enttype")))
#endif



#define _ENTITY_DECLARE_COMPONENT_JOIN_(a,b) a##b
#define _ENTITY_DECLARE_COMPONENT_JOIN(a,b) _ENTITY_DECLARE_COMPONENT_JOIN_(a,b)
#define _ENTITY_DECLARE_COMPONENT_NAME(name) _ENTITY_DECLARE_COMPONENT_JOIN(_component_data_,name)
#define ENTITY_DECLARE_COMPONENT(var_name,name,ctx_struct,type_ctx_struct,...) \
	static struct _ENTITY_COMPONENT _ENTITY_DECLARE_COMPONENT_NAME(var_name)={ \
		(name), \
		0xffff, \
		(unsigned short int)sizeof(ctx_struct), \
		(unsigned short int)sizeof(type_ctx_struct), \
		__VA_ARGS__ \
	}; \
	static const _ENTITY_COMPONENT_DATA_SECTION entity_component_t var_name=&_ENTITY_DECLARE_COMPONENT_NAME(var_name)



#define _ENTITY_DECLARE_TYPE_JOIN_(a,b) a##b
#define _ENTITY_DECLARE_TYPE_JOIN(a,b) _ENTITY_DECLARE_TYPE_JOIN_(a,b)
#define _ENTITY_DECLARE_TYPE_NAME(name) _ENTITY_DECLARE_TYPE_JOIN_(_type_data_,name)
#define _ENTITY_DECLARE_TYPE_ALLOCATOR_NAME(name) _ENTITY_DECLARE_TYPE_JOIN_(_type_allocator_,name)
#define _ENTITY_DECLARE_TYPE_INITIALIZER_FUNC_NAME(name) _ENTITY_DECLARE_TYPE_JOIN_(_type_initializer_,name)
#define _ENTITY_DECLARE_TYPE_INITIALIZER_FUNC_PTR_NAME(name) _ENTITY_DECLARE_TYPE_JOIN(_type_initializer_data_ptr_,name)
#define ENTITY_DECLARE_TYPE(var_name,name,flags_,order,...) \
	static entity_type_allocator_t _ENTITY_DECLARE_TYPE_ALLOCATOR_NAME(var_name)={ \
		NULL, \
		NULL, \
		0, \
		0 \
	}; \
	static struct _ENTITY_TYPE _ENTITY_DECLARE_TYPE_NAME(var_name)={ \
		(name), \
		.allocator=&_ENTITY_DECLARE_TYPE_ALLOCATOR_NAME(var_name), \
		.flags=(flags_) \
	}; \
	static void _ENTITY_DECLARE_TYPE_INITIALIZER_FUNC_NAME(var_name)(void){ \
		entity_component_t components[]={__VA_ARGS__,NULL}; \
		_entity_declare_type_internal(&_ENTITY_DECLARE_TYPE_NAME(var_name),components,(order)); \
	}; \
	static const _ENTITY_TYPE_INITIALIZER_DATA_SECTION _entity_type_initializer_func_t _ENTITY_DECLARE_TYPE_INITIALIZER_FUNC_PTR_NAME(var_name)=_ENTITY_DECLARE_TYPE_INITIALIZER_FUNC_NAME(var_name); \
	static const entity_type_t var_name=&_ENTITY_DECLARE_TYPE_NAME(var_name)



#define ENTITY_COMPONENT_NO_DATA struct _ENTITY_COMPONENT_NO_DATA

#define ENTITY_TYPE_HAS_COMPONENT(type,component) ((type)->data_offsets[(component)->id]!=ENTITY_COMPONENT_NOT_PRESENT)
#define ENTITY_HAS_COMPONENT(entity,component) ENTITY_TYPE_HAS_COMPONENT((entity)->type,(component))

#define ENTITY_GET_COMPONENT_DATA_INDEXED(entity,index) ((void*)(((uintptr_t)(entity))+ENTITY_COMPONENT_OFFSET_GET_OFFSET((entity)->type,(index))))
#define ENTITY_GET_COMPONENT_DATA(entity,component) ENTITY_GET_COMPONENT_DATA_INDEXED((entity),(component)->id)
#define ENTITY_GET_COMPONENT_DATA_TYPED(entity,component,type) ((type*)ENTITY_GET_COMPONENT_DATA((entity),(component)))

#define ENTITY_TYPE_GET_COMPONENT_DATA_INDEXED(type,index) ((void*)(((uintptr_t)((type)->extra_data))+(type)->extra_data_offsets[(index)]))
#define ENTITY_TYPE_GET_COMPONENT_DATA(type,component) ENTITY_TYPE_GET_COMPONENT_DATA_INDEXED((type),(component)->id)
#define ENTITY_TYPE_GET_COMPONENT_DATA_TYPED(type,component,type_) ((type_*)ENTITY_TYPE_GET_COMPONENT_DATA((type),(component)))

#define ENTITY_COMPONENT_NOT_PRESENT 0xffffffff
#define ENTITY_COMPONENT_INVALID_INDEX 0xffff
#define ENTITY_COMPONENT_INVALID_EXTRA_DATA_OFFSET 0xffff
#define ENTITY_COMPONENT_OFFSET_GET_OFFSET(type,index) ((type)->data_offsets[(index)]>>16)
#define ENTITY_COMPONENT_OFFSET_GET_NEXT_INDEX(type,index) ((type)->data_offsets[(index)]&0xffff)

#define ENTITY_MAX_COMPONENTS 16

#define ENTITY_ALLOCATOR_MAX_UNUSED_OBJECTS 16

#define ENTITY_FLAG_INTERNAL0 1
#define ENTITY_FLAG_INTERNAL1 2

#define ENTITY_COMPONENT_FUNCTION_ENTITY_INIT 0
#define ENTITY_COMPONENT_FUNCTION_ENTITY_UPDATE 1
#define ENTITY_COMPONENT_FUNCTION_ENTITY_RENDER 2
#define ENTITY_COMPONENT_FUNCTION_ENTITY_DEINIT 3
#define ENTITY_COMPONENT_FUNCTION_TYPE_INIT 4
#define ENTITY_COMPONENT_FUNCTION_TYPE_RESIZE 5
#define ENTITY_COMPONENT_FUNCTION_TYPE_RENDER 6
#define ENTITY_COMPONENT_FUNCTION_TYPE_DEINIT 7
#define ENTITY_COMPONENT_MAX_FUNCTION ENTITY_COMPONENT_FUNCTION_TYPE_DEINIT

#define ENTITY_TYPE_GET_ENTITY_COUNT(type) ((type)->allocate.used_count)
#define ENTITY_TYPE_GET_ENTITY(type) ((type)->allocate.used)
#define ENTITY_TYPE_ITER_ENTITIES(type,var) for (entity_t var=(type)->allocate->used;var;var=var->_next_entity)

#define ENTITY_TYPE_FLAG_SINGLETON 1



struct _ENTITY;
struct _ENTITY_TYPE;



typedef struct _ENTITY_COMPONENT{
	const char* name;
	unsigned short int id;
	unsigned short int ctx_size;
	unsigned short int type_ctx_size;
	union{
		struct{
			void (*e_init)(struct _ENTITY*,void*);
			_Bool (*e_update)(struct _ENTITY*,void*);
			_Bool (*e_render)(struct _ENTITY*,void*,void*);
			void (*e_deinit)(struct _ENTITY*,void*);
			void (*t_init)(const struct _ENTITY_TYPE*,void*);
			void (*t_resize)(const struct _ENTITY_TYPE*,void*,unsigned int);
			void (*t_render)(const struct _ENTITY_TYPE*,void*);
			void (*t_deinit)(const struct _ENTITY_TYPE*,void*);
		};
		void* _functions[7];
	};
}* entity_component_noconst_t;



typedef const struct _ENTITY_COMPONENT* entity_component_t;



struct _ENTITY_COMPONENT_NO_DATA{
	unsigned char _[0];
};



typedef struct _ENTITY_TYPE_ALLOCATOR{
	struct _ENTITY* used;
	struct _ENTITY* unused;
	unsigned int used_count;
	unsigned int unused_count;
} entity_type_allocator_t;



typedef struct _ENTITY_TYPE{
	const char* name;
	struct _ENTITY_TYPE* _next_type;
	entity_type_allocator_t* allocator;
	unsigned short int size;
	unsigned short int first_component_index;
	unsigned int _order;
	unsigned int flags;
	unsigned int data_offsets[ENTITY_MAX_COMPONENTS];
	unsigned short int extra_data_offsets[ENTITY_MAX_COMPONENTS];
	unsigned short int* _functions[ENTITY_COMPONENT_MAX_FUNCTION+1];
	void* extra_data;
}* entity_type_noconst_t;



typedef const struct _ENTITY_TYPE* entity_type_t;



typedef void (*_entity_type_initializer_func_t)(void);



typedef struct _ENTITY{
	entity_type_t type;
	struct _ENTITY* _next_entity;
	struct _ENTITY* _prev_entity;
	unsigned int flags;
	unsigned int index;
}* entity_t;



void _entity_declare_type_internal(entity_type_noconst_t type,const entity_component_t* components,unsigned int order);



void entity_init(void);



entity_t entity_create(entity_type_t type);



void entity_delete(entity_t entity);



void entity_delete_all(_Bool reset_allocator_cache);



void entity_update_all(void);



void entity_render(entity_type_t type);



void entity_reset_all(void);



void entity_print_type_info(void);



#endif
