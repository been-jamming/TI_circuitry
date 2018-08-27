#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "circuit.h"

primitive *create_primitive(value *input1, value *input2, primitive_type type){
	primitive *output;
	output = malloc(sizeof(primitive));
	output->type = type;
	output->input1 = input1;
	output->input2 = input2;
	return output;
}

void tick_primitive(primitive *p){
	if(p->type == AND){
		if(*(p->input1) == TRUE_VALUE && *(p->input2) == TRUE_VALUE){
			p->next_output = TRUE_VALUE;
		} else if(*(p->input1) == UNDEFINED_VALUE || *(p->input2) == UNDEFINED_VALUE){
			p->next_output = UNDEFINED_VALUE;
		} else {
			p->next_output = FALSE_VALUE;
		}
	} else if(p->type == OR){
		if(*(p->input1) == UNDEFINED_VALUE || *(p->input2) == UNDEFINED_VALUE){
			p->next_output = UNDEFINED_VALUE;
		} else if(*(p->input1) == TRUE_VALUE || *(p->input2) == TRUE_VALUE){
			p->next_output = TRUE_VALUE;
		} else {
			p->next_output = FALSE_VALUE;
		}
	} else if(p->type == NOT){
		if(*(p->input1) == UNDEFINED_VALUE){
			p->next_output = UNDEFINED_VALUE;
		} else if(*(p->input1) == TRUE_VALUE){
			p->next_output = FALSE_VALUE;
		} else {
			p->next_output = TRUE_VALUE;
		}
	} else if(p->type == XOR){
		if(*(p->input1) == UNDEFINED_VALUE || *(p->input2) == UNDEFINED_VALUE){
			p->next_output = UNDEFINED_VALUE;
		} else if(*(p->input1) != *(p->input2)){
			p->next_output = TRUE_VALUE;
		} else {
			p->next_output = FALSE_VALUE;
		}
	} else if(p->type == NAND){
		if(*(p->input1) == TRUE_VALUE && *(p->input2) == TRUE_VALUE){
			p->next_output = FALSE_VALUE;
		} else if(*(p->input1) == UNDEFINED_VALUE || *(p->input2) == UNDEFINED_VALUE){
			p->next_output = UNDEFINED_VALUE;
		} else {
			p->next_output = TRUE_VALUE;
		}
	} else if(p->type == NOR){
		if(*(p->input1) == UNDEFINED_VALUE || *(p->input2) == UNDEFINED_VALUE){
			p->next_output = UNDEFINED_VALUE;
		} else if(*(p->input1) == TRUE_VALUE || *(p->input2) == TRUE_VALUE){
			p->next_output = FALSE_VALUE;
		} else {
			p->next_output = TRUE_VALUE;
		}
	} else if(p->type == BUFF){
		p->next_output = *(p->input1);
	} else if(p->type == XNOR){
		if(*(p->input1) == UNDEFINED_VALUE || *(p->input2) == UNDEFINED_VALUE){
			p->next_output = UNDEFINED_VALUE;
		} else if(*(p->input1) != *(p->input2)){
			p->next_output = FALSE_VALUE;
		} else {
			p->next_output = TRUE_VALUE;
		}
	}
}

void update_primitive(primitive *p){
	p->output = p->next_output;
}

table *create_table(unsigned int num_inputs, unsigned int num_outputs){
	unsigned int i;
	unsigned int j;
	table *output;
	
	output = malloc(sizeof(table));
	output->num_inputs = num_inputs;
	output->num_outputs = num_outputs;
	output->inputs = calloc(num_inputs, sizeof(value *));
	output->table_outputs = malloc(sizeof(value *)*(1<<num_inputs));
	for(i = 0; i < (1<<num_inputs); i++){
		output->table_outputs[i] = malloc(sizeof(value)*num_outputs);
		for(j = 0; j < num_outputs; j++){
			output->table_outputs[i][j] = UNDEFINED_VALUE;
		}
	}
	output->next_outputs = malloc(sizeof(value)*num_outputs);
	output->outputs = malloc(sizeof(value)*num_outputs);
	for(i = 0; i < num_outputs; i++){
		output->next_outputs[i] = UNDEFINED_VALUE;
		output->outputs[i] = UNDEFINED_VALUE;
	}
}

void tick_table(table *t){
	unsigned int i;
	unsigned int table_index;
	bool inputs_undefined;

	table_index = 0;
	inputs_undefined = 0;

	for(i = 0; i < t->num_inputs; i++){
		table_index <<= 1;
		if(*(t->inputs[i]) == UNDEFINED_VALUE){
			inputs_undefined = true;
			break;
		} else if(*(t->inputs[i]) == TRUE_VALUE){
			table_index |= 1;
		}
	}

	if(!inputs_undefined){
		memcpy(t->next_outputs, t->table_outputs[table_index], sizeof(value)*t->num_outputs);
	} else {
		for(i = 0; i < t->num_outputs; i++){
			t->next_outputs[i] = UNDEFINED_VALUE;
		}
	}
}

void update_table(table *t){
	memcpy(t->outputs, t->next_outputs, sizeof(value)*t->num_outputs);
}

component create_component(component_type type){
	component output;
	output = (component) {.type = type};
	return output;
}

void tick_component(component *c){
	if(c->type == PRIMITIVE){
		tick_primitive(c->prim);
	} else if(c->type == TABLE){
		tick_table(c->table);
	} else if(c->type == SUBCIRCUIT){
		tick_circuit(c->circ);
	}
}

void update_component(component *c){
	if(c->type == PRIMITIVE){
		update_primitive(c->prim);
	} else if(c->type == TABLE){
		update_table(c->table);
	} else if(c->type == SUBCIRCUIT){
		update_circuit(c->circ);
	}
}

circuit *create_circuit(unsigned int num_components){
	circuit *output;
	output = malloc(sizeof(circuit));
	output->num_components = num_components;
	output->components = malloc(sizeof(component)*num_components);
	return output;
}

void tick_circuit(circuit *c){
	unsigned int i;

	for(i = 0; i < c->num_components; i++){
		tick_component(c->components + i);
	}
}

void update_circuit(circuit *c){
	unsigned int i;

	for(i = 0; i < c->num_components; i++){
		update_component(c->components + i);
	}
}

