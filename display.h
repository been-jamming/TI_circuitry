#include "circuit.h"

//Coordinates represent an x-y position (relative to the entire circuit, not the current view)
//which is used as positions of wires and elements of other structures.
//
//Coordinates can be negative since they are relative to the center of the circuit.

typedef struct coordinate coordinate;

struct coordinate{
	int x;
	int y;
};

//Input pins point to the input of a circuit.
//
//Input pins can be connected to only one output, but multiple inputs can be connected to the
//same output.

typedef struct input_pin input_pin;

struct input_pin{
	coordinate pos;
	value *input_value;
};

//Output pins point to the output of a circuit.
//
//Output pins are pointed to by any wire connected to it (indirectly or directly). This gives
//the application access to which value a wire takes on without having to propogate the signal
//directly between the wires.

typedef struct output_pin output_pin;

struct output_pin{
	coordinate pos;
	value *output_value;
};

//Wires propogate output to input pins and other wires.
//
//Wires can only be horizontal or vertical to speed up rendering and make circuits neater.

typedef struct wire wire;

struct wire{
	coordinate pos;
	enum direction_type {HORIZONTAL, VERTICAL} direction;
	int length;
	output_pin *output;
	wire *connection1_left;
	wire *connection1_right;
	wire *connection1_up;
	wire *connection1_down;
	wire *connection2_left;
	wire *connection2_right;
	wire *connection2_up;
	wire *connection2_down;
};

//Component views contain all of the display information for an individual component in a circuit view.

typedef struct component_view component_view;

struct component_view{
	component *comp;
	union {
		struct box_orientation {
			coordinate p0;
			coordinate p1;
		} box;
		unsigned char orientation;
	};
	coordinate *input_pins;
	coordinate *output_pins;
};

//Circuit views contain all of the display information of a circuit simulation.

typedef struct circuit_view circuit_view;

struct circuit_view{
	wire *wires;
	unsigned int num_wires;
	unsigned int wire_buff_length;
	input_pin *input_pins;
	unsigned int num_input_pins;
	unsigned int input_pin_buff_length;
	output_pin *output_pins;
	unsigned int num_output_pins;
	unsigned int output_pin_buff_length;
	circuit circ;
	unsigned int components_buff_length;
	component_view *component_views;
	int view_left;
	int view_right;
	int view_up;
	int view_down;
};

