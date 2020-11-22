#include "exmap.h"
#include "vector.h"

#include "../core/core.h"
#include "../dmdism/opcodes.h"

#include <cmath>
#include <chrono>
#include <memory>

int str_id_vector_pointer;

std::shared_ptr<vector>& get_vector(Value val)
{
	uint32_t v = val.get_by_id(str_id_vector_pointer).value;
	if (v == 0) Runtime("Vector has null extools pointer!");
	return *((std::shared_ptr<vector>*)v);
}

trvh vector_register(unsigned int args_len, Value* args, Value src)
{
	std::shared_ptr<vector> *ptr = new std::shared_ptr<vector>;
	//*ptr = std::make_shared<vector>(src.get_by_id(str_id_x).valuef);
	SetVariable(src.type, src.value, str_id_vector_pointer, Value(NUMBER, (int)ptr));
	return Value::Null();
}
/*
trvh get_x(unsigned int args_len, Value* args, Value src)
{
	return Value(get_vector(src)->get_x());
}

trvh get_y(unsigned int args_len, Value* args, Value src)
{
	return Value(get_vector(src)->get_y());
}

trvh set_x(unsigned int args_len, Value* args, Value src)
{
	get_vector(src)->set_x(args_len > 0 ? args[0].valuef : 0);
	return Value::Null();
}

trvh set_y(unsigned int args_len, Value* args, Value src)
{
	get_vector(src)->set_y(args_len > 0 ? args[0].valuef : 0);
	return Value::Null();
}
*/


const char* enable_exmap(){
	//Set up hooks
	Core::get_proc("/datum/vectorNew/proc/vector_register").hook(vector_register);
	//Core::get_proc("/datum/vectorNew/proc/vector_unregister").hook(vector_unregister);
	//Accessors & mutators for vector instances.
	/*
	Core::get_proc("/datum/vectorNew/proc/get_x").hook(get_x);
	Core::get_proc("/datum/vectorNew/proc/get_y").hook(get_y);
	Core::get_proc("/datum/vectorNew/proc/set_x").hook(set_x);
	Core::get_proc("/datum/vectorNew/proc/set_y").hook(set_y);
	*/

	str_id_vector_pointer = Core::GetStringId("_extools_pointer_vector", true);
    return "ok";
}