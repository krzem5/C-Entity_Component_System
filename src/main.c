#include <ecs/entity.h>
#include <stdio.h>



#define ORDER_PLAYER 0
#define ORDER_BALL 1
#define ORDER_LAST 99



static _Bool empty_test_component_update(entity_t entity,void* ctx){
	printf("Delete player\n");
	entity_delete(entity);
	return 1;
}



ENTITY_DECLARE_COMPONENT(empty_test_component,"empty",ENTITY_COMPONENT_NO_DATA,ENTITY_COMPONENT_NO_DATA,
	.e_update=empty_test_component_update
);



typedef struct _RENDER_TEST_CTX{
	unsigned int index;
} render_test_ctx_t;



typedef struct _RENDER_TEST_TYPE_CTX{
	unsigned int count;
} render_test_type_ctx_t;



static unsigned int _next_index=0;



static void render_test_init(entity_t entity,void* ctx){
	render_test_ctx_t* data=ctx;
	data->index=_next_index;
	_next_index++;
}



static _Bool render_test_render(entity_t entity,void* ctx,void* type_ctx){
	render_test_ctx_t* data=ctx;
	printf("%s[%u]: %u\n",entity->type->name,data->index,entity->index);
	return 0;
}



static void render_test_init_type(entity_type_t type,void* type_ctx){
	render_test_type_ctx_t* data=type_ctx;
	data->count=0;
}



static void render_test_resize_type(entity_type_t type,void* type_ctx,unsigned int size){
	render_test_type_ctx_t* data=type_ctx;
	data->count=size;
}



static void render_test_render_type(entity_type_t type,void* type_ctx){
	render_test_type_ctx_t* data=type_ctx;
	printf("%u elements\n",data->count);
}



ENTITY_DECLARE_COMPONENT(render_test_component,"render-test",render_test_ctx_t,render_test_type_ctx_t,
	.t_init=render_test_init_type,
	.t_resize=render_test_resize_type,
	.t_render=render_test_render_type,
	.e_init=render_test_init,
	.e_render=render_test_render
);



typedef struct _CONTROLLER{
	unsigned int controller_data;
} controller_t;



static void controller_init(entity_t entity,void* ctx){
	controller_t* data=ctx;
	data->controller_data=0x12345678;
	printf("[%p]: Controller init\n",entity);
}



ENTITY_DECLARE_COMPONENT(player_controller_component,"player-controller",controller_t,ENTITY_COMPONENT_NO_DATA,
	.e_init=controller_init
);



static void player_deinit(entity_t player,void* ctx){
	printf("Deleting player...\n");
}



ENTITY_DECLARE_COMPONENT(player_core_component,"player-core",ENTITY_COMPONENT_NO_DATA,ENTITY_COMPONENT_NO_DATA,
	.e_deinit=player_deinit
);



typedef struct _POSITION_CTX{
	float x;
	float y;
	float z;
} position_ctx_t;



static void position_init(entity_t player,void* ctx){
	position_ctx_t* data=ctx;
	data->x=0.0f;
	data->y=1.0f;
	data->z=0.0f;
}



ENTITY_DECLARE_COMPONENT(position_component,"position",position_ctx_t,ENTITY_COMPONENT_NO_DATA,
	.e_init=position_init
);



#define ENTITY_POSITION(entity) ENTITY_GET_COMPONENT_DATA_TYPED((entity),position_component,position_ctx_t)



ENTITY_DECLARE_TYPE(player_type,"player",ENTITY_TYPE_FLAG_SINGLETON,ORDER_PLAYER,
	player_core_component,
	position_component,
	empty_test_component,
	player_controller_component
);



ENTITY_DECLARE_COMPONENT(ball_core_component,"ball-core",ENTITY_COMPONENT_NO_DATA,ENTITY_COMPONENT_NO_DATA);



ENTITY_DECLARE_TYPE(type_ordering_test_type,"type-ordering-test",0,ORDER_LAST,
	empty_test_component
);



ENTITY_DECLARE_TYPE(ball_type,"ball",0,ORDER_BALL,
	render_test_component,
	ball_core_component,
	empty_test_component
);



int main(void){
	entity_init();
	entity_t player=entity_create(player_type);
	entity_update_all();
	position_ctx_t* pos=ENTITY_POSITION(player);
	printf("<%.3f %.3f %.3f>\n",pos->x,pos->y,pos->z);
	entity_delete(player);
	entity_create(player_type);
	entity_render(player_type);
	entity_delete(entity_create(type_ordering_test_type));
	entity_print_type_info();
	entity_delete_all(0);
	entity_delete_all(1);
	entity_t test_entities[4];
	for (unsigned int i=0;i<4;i++){
		test_entities[i]=entity_create(ball_type);
	}
	printf("----\n");
	entity_render(ball_type);
	printf("----\n");
	unsigned int index=1;
	entity_delete(test_entities[index]);
	test_entities[index]=NULL;
	entity_render(ball_type);
	printf("----\n");
	for (unsigned int i=0;i<4;i++){
		if (test_entities[i]){
			entity_delete(test_entities[i]);
		}
	}
	entity_delete_all(1);
	return 0;
}
