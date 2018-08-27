#define true 1
#define false 0

typedef unsigned char bool;

typedef enum{TRUE_VALUE, FALSE_VALUE, UNDEFINED_VALUE} value;

typedef enum{AND, OR, NOT, XOR, NAND, BUFF, NOR, XNOR} primitive_type;

typedef struct primitive primitive;

struct primitive{
	primitive_type type;
	value *input1;	
	value *input2;
	value next_output;
	value output;
};

typedef struct table table;

struct table{
	unsigned int num_inputs;
	unsigned int num_outputs;
	value **inputs;
	value **table_outputs;
	value *next_outputs;
	value *outputs;
};

typedef enum{PRIMITIVE, TABLE, SUBCIRCUIT} component_type;

typedef struct component component;

typedef struct circuit circuit;

struct circuit{
	unsigned int num_components;
	component *components;
};

struct component{
	component_type type;
	union{
		primitive *prim;
		table *table;
		circuit *circ;
	};
};

void tick_circuit(circuit *c);

void update_circuit(circuit *c);
