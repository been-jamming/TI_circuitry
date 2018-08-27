#define SAVE_SCREEN
#define MIN_AMS 200
#define USE_TI89

#include <tigcclib.h>
#include "display.h"

#define ON_VIEW(x, y) (((x) <= 159) && ((x) >= 0) && ((y) >= 16) && ((y) <= 99))

#define MENU_FILE 1

#define MENU_FILE_NEW 2
#define MENU_FILE_OPEN 3
#define MENU_FILE_SAVE 4
#define MENU_FILE_SAVE_AS 5
//---
#define MENU_ADD 6

#define MENU_ADD_WIRE 7
#define MENU_ADD_AND 8
#define MENU_ADD_OR 9
#define MENU_ADD_NOT 10
#define MENU_ADD_XOR 11
#define MENU_ADD_LOAD 12
//---
#define MENU_QUIT 13

#define MENU_QUIT_QUIT 14

#define NONE 0
#define ADDING_WIRE 1
#define ADDING_AND 2
#define ADDING_OR 3
#define ADDING_NOT 4
#define ADDING_XOR 5

#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3

unsigned char pointer[8] = {
	0b00000000,
	0b00011000,
	0b00011000,
	0b01100110,
	0b01100110,
	0b00011000,
	0b00011000,
	0b00000000
};

unsigned char input_pin_sprite[8] = {
	0b00000000,
	0b01111110,
	0b01111110,
	0b01100110,
	0b01100110,
	0b01111110,
	0b01111110,
	0b00000000
};

unsigned char output_pin_sprite[8] = {
	0b00000000,
	0b00011000,
	0b00111100,
	0b01100110,
	0b01100110,
	0b00111100,
	0b00011000,
	0b00000000
};

unsigned long int and_sprite_north[20] = {
	0b00000000000000000000000000000000,
	0b00000000000000000000000000000000,
	0b11111111111111100000000000000000,
	0b10000000000000010000000000000000,
	0b10000000000000001000000000000000,
	0b10000000000000001000000000000000,
	0b10000000000000000100000000000000,
	0b10000000000000000100000000000000,
	0b10000000000000000100000000000000,
	0b10000000000000000010000000000000,
	0b10000000000000000010000000000000,
	0b10000000000000000100000000000000,
	0b10000000000000000100000000000000,
	0b10000000000000000100000000000000,
	0b10000000000000001000000000000000,
	0b10000000000000001000000000000000,
	0b10000000000000010000000000000000,
	0b11111111111111100000000000000000,
	0b00000000000000000000000000000000,
	0b00000000000000000000000000000000
};



void *kbq;
INT_HANDLER old_int_5 = NULL;
unsigned char cursor_x = 76;
unsigned char cursor_y = 46;
int frame_x;
int frame_y;

circuit_view *global_view;

short menu_command;
int command;
unsigned char state;
unsigned char direction;
coordinate cursor_pos;
coordinate cursor_buffer;
unsigned char disable_horizontal;
unsigned char disable_vertical;

HANDLE menu;
HANDLE menu_run;

short old_key_delay;
short old_key_speed;

void redraw_cursor(){
	Sprite8(cursor_x, cursor_y, 8, pointer, LCD_MEM, SPRT_XOR);
}

circuit create_circuit(unsigned int num_components){
	circuit output;
	output.components = malloc(sizeof(component)*num_components);
	output.num_components = num_components;
	return output;
}

circuit_view *create_circuit_view(unsigned int num_wires, unsigned int num_input_pins, unsigned int num_output_pins, int view_left, int view_right, int view_up, int view_down){
	circuit_view *output;
	output = malloc(sizeof(circuit_view));
	output->wires = malloc(sizeof(wire)*num_wires);
	output->wire_buff_length = num_wires;
	output->num_wires = 0;
	output->input_pins = malloc(sizeof(input_pin)*num_input_pins);
	output->input_pin_buff_length = num_input_pins;
	output->num_input_pins = 0;
	output->output_pins = malloc(sizeof(output_pin)*num_output_pins);
	output->output_pin_buff_length = num_output_pins;
	output->num_output_pins = 0;
	output->view_left = view_left;
	output->view_right = view_right;
	output->view_up = view_up;
	output->view_down = view_down;
	output->circ = create_circuit(0);
	output->components_buff_length = 0;	
	return output;
}

void add_wire(circuit_view *c, wire w){
	wire *new_wires;
	
	if(c->num_wires == c->wire_buff_length){
		c->wire_buff_length <<= 1;
		new_wires = malloc(sizeof(wire)*c->wire_buff_length);
		memcpy(new_wires, c->wires, sizeof(wire)*c->num_wires);
		free(c->wires);
		c->wires = new_wires;
	}

	c->wires[c->num_wires] = w;
	c->num_wires++;
}

void add_component(circuit_view *c, component comp){
	component *new_components;

	if(c->circ.num_components == c->components_buff_length){
		c->components_buff_length <<= 1;
		new_components = malloc(sizeof(component)*c->components_buff_length);
		memcpy(new_components, c->circ.components, sizeof(component)*c->circ.num_components);
		free(c->circ.components);
		c->circ.components = new_components;
	}

	c->circ.components[c->circ.num_components] = comp;
	c->circ.num_components += 1;
}

void draw_wire(wire w, int offset_x, int offset_y){
	int x0;
	int y0;
	int x1;
	int y1;

	x0 = w.pos.x;
	y0 = w.pos.y;

	if(w.direction == HORIZONTAL){
		x1 = x0 + w.length;
		y1 = y0;
	} else {
		x1 = x0;
		y1 = y0 + w.length;
	}

	if(x1 < x0){
		int temp_x;
		temp_x = x1;
		x1 = x0;
		x0 = temp_x;
	}
	if(y1 < y0){
		int temp_y;
		temp_y = y1;
		y1 = y0;
		y0 = temp_y;
	}
	
	x0 = x0 - offset_x;
	y0 = y0 - offset_y;
	x1 = x1 - offset_x;
	y1 = y1 - offset_y;

	if(ON_VIEW(x0, y0) || ON_VIEW(x1, y1)){
		if(x0 < 0){
			x0 = 0;
		}
		if(x1 > 159){
			x1 = 159;
		}
		if(y0 < 16){
			y0 = 16;
		}
		if(y1 > 99){
			y1 = 99;
		}
		DrawLine(x0, y0, x1, y1, A_THICK1);
	}
}

void draw_wires(circuit_view *c){
	unsigned int i;

	for(i = 0; i < c->num_wires; i++){
		draw_wire(c->wires[i], c->view_left, c->view_up);
	}
}

void draw_component(component c, int x_offset, int y_offset, unsigned char direction){

}

void shift_view_y(int offset){
	global_view->view_up += offset;
	global_view->view_down += offset;
}

void shift_view_x(int offset){
	global_view->view_left += offset;
	global_view->view_right += offset;
}

void set_cursor(){
	cursor_pos.x = cursor_x + global_view->view_left + 3;
	cursor_pos.y = cursor_y + global_view->view_up + 3;
}

void redraw_all(){
	clrscr();

	Sprite32(50, 50, 20, and_sprite_north, LCD_MEM, SPRT_OR);

	MenuOn(menu_run);
	draw_wires(global_view);
	redraw_cursor();
}

DEFINE_INT_HANDLER (time_update){
	unsigned int key;
	short menu_result;
	unsigned char cursor_speed;
	cursor_speed = 4;

	while(!OSdequeue(&key, kbq)){
		key = key&0xF7FF;
		menu_result = MenuKey(menu_run, key);
		if(menu_result == M_NOTMENUKEY){
			if(key == KEY_UP && !disable_vertical){
				command = KEY_UP;
				if(cursor_y - cursor_speed >= 15){
					redraw_cursor();
					cursor_y -= cursor_speed;
					redraw_cursor();
					set_cursor();
				} else {
					shift_view_y(-cursor_speed);
					redraw_all();
					set_cursor();
				}
			} else if(key == KEY_DOWN && !disable_vertical){
				command = KEY_DOWN;
				if(cursor_y + cursor_speed <= 91){
					redraw_cursor();
					cursor_y += cursor_speed;
					redraw_cursor();
					set_cursor();
				} else {
					shift_view_y(cursor_speed);
					redraw_all();
					set_cursor();
				}
			} else if(key == KEY_LEFT && !disable_horizontal){
				command = KEY_LEFT;
				if(cursor_x - cursor_speed >= 0){
					redraw_cursor();
					cursor_x -= cursor_speed;
					redraw_cursor();
					set_cursor();
				} else {
					shift_view_x(-cursor_speed);
					redraw_all();
					set_cursor();
				}
			} else if(key == KEY_RIGHT && !disable_horizontal){
				command = KEY_RIGHT;
				if(cursor_x + cursor_speed <= 151){
					redraw_cursor();
					cursor_x += cursor_speed;
					redraw_cursor();
					set_cursor();
				} else {
					shift_view_x(cursor_speed);
					redraw_all();
					set_cursor();
				}
			} else {
				command = key;
			}
		} else {
			menu_command = menu_result;
		}
	}
}

void _main(){
	old_int_5 = GetIntVec(AUTO_INT_5);
	
	menu = MenuNew(0, 0, 0);
	DynMenuAdd(menu, 0, "File", MENU_FILE, DMF_TEXT);
	DynMenuAdd(menu, MENU_FILE, "New", MENU_FILE_NEW, DMF_TEXT);
	DynMenuAdd(menu, MENU_FILE, "Open", MENU_FILE_OPEN, DMF_TEXT);
	DynMenuAdd(menu, MENU_FILE, "Save", MENU_FILE_SAVE, DMF_TEXT);
	DynMenuAdd(menu, MENU_FILE, "Save as", MENU_FILE_SAVE_AS, DMF_TEXT);
	DynMenuAdd(menu, 0, "Add", MENU_ADD, DMF_TEXT);
	DynMenuAdd(menu, MENU_ADD, "Wire", MENU_ADD_WIRE, DMF_TEXT);
	DynMenuAdd(menu, MENU_ADD, "AND", MENU_ADD_AND, DMF_TEXT);
	DynMenuAdd(menu, MENU_ADD, "OR", MENU_ADD_OR, DMF_TEXT);
	DynMenuAdd(menu, MENU_ADD, "NOT", MENU_ADD_NOT, DMF_TEXT);
	DynMenuAdd(menu, MENU_ADD, "XOR", MENU_ADD_XOR, DMF_TEXT);
	DynMenuAdd(menu, MENU_ADD, "Load", MENU_ADD_LOAD, DMF_TEXT);
	DynMenuAdd(menu, 0, "Quit", MENU_QUIT, DMF_TEXT);
	DynMenuAdd(menu, MENU_QUIT, "Quit", MENU_QUIT_QUIT, DMF_TEXT);
	
	global_view = create_circuit_view(1, 0, 0, 0, 159, 0, 99);
	add_wire(global_view, (wire) {.pos = (coordinate) {.x = 50, .y = 50}, .direction = HORIZONTAL, .length = 5});
	add_wire(global_view, (wire) {.pos = (coordinate) {.x = 50, .y = 50}, .direction = VERTICAL, .length = 15});

	kbq = kbd_queue();
	old_key_delay = OSInitKeyInitDelay(50);
	old_key_speed = OSInitBetweenKeyDelay(10);

	menu_run = MenuBegin(HLock(menu), 0, 0, 0);
	menu_command = 0;
	command = 0;
	state = NONE;
	direction = NORTH;
	SetIntVec(AUTO_INT_5, time_update);

	redraw_all();

	while(menu_command != MENU_QUIT_QUIT){
		if(state == NONE && menu_command == MENU_ADD_WIRE){
			wire new_wire;
			state = ADDING_WIRE;
			cursor_buffer.x = cursor_pos.x;
			cursor_buffer.y = cursor_pos.y;
			new_wire.pos = cursor_pos;
			new_wire.direction = HORIZONTAL;
			new_wire.length = 0;
			add_wire(global_view, new_wire);
			menu_command = NONE;
		}

		if(state == ADDING_WIRE){
			if(!disable_horizontal && !disable_vertical){
				if(command == KEY_LEFT || command == KEY_RIGHT){
					disable_vertical = 1;
					global_view->wires[global_view->num_wires - 1].direction = HORIZONTAL;
				} else if(command == KEY_UP || command == KEY_DOWN){
					disable_horizontal = 1;
					global_view->wires[global_view->num_wires - 1].direction = VERTICAL;
				}
			}
			
			if(command == KEY_UP || command == KEY_DOWN){
				global_view->wires[global_view->num_wires - 1].length = cursor_pos.y - cursor_buffer.y;
				redraw_all();
			} else if(command == KEY_LEFT || command == KEY_RIGHT){
				global_view->wires[global_view->num_wires - 1].length = cursor_pos.x - cursor_buffer.x;
				redraw_all();
			} else if(command == KEY_ESC){
				state = NONE;
				disable_horizontal = 0;
				disable_vertical = 0;
			}
		}
		command = NONE;
	}
	
	MenuEnd(menu_run);
	HeapFree(menu);
	SetIntVec(AUTO_INT_5, old_int_5);
	OSInitKeyInitDelay(old_key_delay);
	OSInitBetweenKeyDelay(old_key_speed);
}
